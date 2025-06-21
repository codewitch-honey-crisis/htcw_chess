#include "chess.h"

#include <memory.h>
#include <stdio.h>
#ifndef NULL
#define NULL 0
#endif
static void move_until_obstacle(chess_value_t (*index_fn)(chess_value_t team, chess_value_t index), chess_value_t team, chess_value_t index, const chess_value_t* game_board, chess_value_t* out_moves, chess_value_t* out_size) {
    chess_value_t i = index;
    while (1) {
        i = index_fn(team, i);
        if (i != -1) {
            if (game_board[i] != -1) {
                if (CHESS_TEAM(game_board[i]) == team) {
                    return;
                }
                out_moves[(*out_size)++] = i;
                return;
            } else {
                out_moves[(*out_size)++] = i;
            }
        } else {
            break;
        }
    }
}
static chess_value_t index_advance(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    if (team == 0) {
        index += 8;
    } else if (team == 1) {
        index -= 8;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_advance_left(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    const chess_value_t x = index % 8;
    if (team == 0) {
        if (x == 7 || index + 8 > 63) {
            return -1;
        }
        index += 9;
    } else if (team == 1) {
        if (x == 0 || index - 9 < 0) {
            return -1;
        }
        index -= 9;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_advance_right(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    const chess_value_t x = index % 8;
    if (team == 0) {
        if (x == 0 || index + 7 > 63) {
            return -1;
        }
        index += 7;
    } else if (team == 1) {
        if (x == 7 || index - 7 < 0) {
            return -1;
        }
        index -= 7;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_retreat_left(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    const chess_value_t x = index % 8;
    if (team == 0) {
        if (x == 7 || index - 7 < 0) {
            return -1;
        }
        index -= 7;
    } else if (team == 1) {
        if (x == 0 || index + 7 > 63) {
            return -1;
        }
        index += 7;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_retreat_right(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    const chess_value_t x = index % 8;
    if (team == 0) {
        if (x == 0 || index - 9 < 0) {
            return -1;
        }
        index -= 9;
    } else if (team == 1) {
        if (x == 7 || index + 9 > 63) {
            return -1;
        }
        index += 9;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_retreat(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    if (team == 1) {
        index += 8;
    } else if (team == 0) {
        index -= 8;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_left(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    const chess_value_t x = index % 8;
    if (team == 0) {
        if (x == 7) {
            return -1;
        }
        index += 1;
    } else if (team == 1) {
        if (x == 0) {
            return -1;
        }
        index -= 1;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static chess_value_t index_right(chess_value_t team, chess_value_t index) {
    if (index == -1) return -1;
    const chess_value_t x = index % 8;
    if (team == 1) {
        if (x == 7) {
            return -1;
        }
        index += 1;
    } else if (team == 0) {
        if (x == 0) {
            return -1;
        }
        index -= 1;
    }
    if (index < 0 || index > 63) return -1;
    return index;
}
static void add_en_passant_target(chess_game_t* game, chess_value_t index) {
    for (int i = 0; i < 16; ++i) {
        if (game->en_passant_targets[i] == -1) {
            game->en_passant_targets[i] = index;
            return;
        }
    }
}
static void clear_en_passant_target(chess_game_t* game, chess_value_t index) {
    for (int i = 0; i < 16; ++i) {
        if (game->en_passant_targets[i] == index) {
            game->en_passant_targets[i] = -1;
            return;
        }
    }
}
static chess_value_t is_en_passant_target(const chess_game_t* game, chess_value_t index) {
    for (int i = 0; i < 16; ++i) {
        if (game->en_passant_targets[i] == index) {
            return 1;
        }
    }
    return 0;
}
static chess_value_t en_passant_target_from_move(const chess_game_t* game, chess_value_t index_from, chess_value_t index_to, const chess_value_t* game_board) {
    if (game == NULL || index_from < 0 || index_from > 63 || index_to < 0 || index_to > 63 || game_board[index_from] == -1) {
        return -1;
    }
    const chess_value_t id = game_board[index_from];
    const chess_value_t team = CHESS_TEAM(id);
    chess_value_t tmp = index_advance_left(team, index_from);
    if (tmp != index_to) {
        tmp = index_advance_right(team, index_from);
        if (tmp != index_to) {
            return -1;
        }
    }
    tmp = index_retreat(team, tmp);
    if (tmp == -1) {
        return -1;
    }  // shouldn't happen
    const chess_value_t id_cmp = game_board[tmp];
    if (id_cmp == -1) return -1;
    const chess_type_t type_cmp = CHESS_TYPE(id_cmp);
    const chess_value_t team_cmp = CHESS_TEAM(id_cmp);
    if (type_cmp != CHESS_PAWN || team_cmp == team) {
        return -1;
    }
    return is_en_passant_target(game, tmp) ? tmp : -1;
}
static chess_size_t compute_moves(const chess_game_t* game, chess_index_t index, chess_index_t* out_moves, const chess_id_t* game_board) {
    const chess_value_t id = game_board[index];
    if (id == -1) {
        return 0;
    }
    const chess_value_t type = CHESS_TYPE(id);
    const chess_value_t team = CHESS_TEAM(id);
    chess_value_t result = 0;
    switch (type) {
        case CHESS_PAWN:
            if ((team == 0 && index < 16) || (team == 1 && index > (64 - 16))) {  // the pawn is on its first move
                chess_value_t tmp = index_advance(team, index);
                if (tmp != -1) {
                    if (game_board[tmp] == -1) {
                        out_moves[result++] = tmp;
                    }
                    chess_value_t attack = index_advance_left(team, index);
                    if (attack != -1 && game_board[attack] != -1 && CHESS_TEAM(game_board[attack]) != team) {
                        out_moves[result++] = attack;
                    }
                    attack = index_advance_right(team, index);
                    if (attack != -1 && game_board[attack] != -1 && CHESS_TEAM(game_board[attack]) != team) {
                        out_moves[result++] = attack;
                    }
                    if (game_board[tmp] == -1) {
                        const chess_value_t tmp2 = index_advance(team, tmp);
                        if (tmp2 != -1 && game_board[tmp2] == -1) {
                            out_moves[result++] = tmp2;
                        }
                    }
                    attack = index_advance_left(team, tmp);
                    if (attack != -1 && game_board[attack] != -1 && CHESS_TEAM(game_board[attack]) != team) {
                        out_moves[result++] = attack;
                    }
                    attack = index_advance_right(team, tmp);
                    if (attack != -1 && game_board[attack] != -1 && CHESS_TEAM(game_board[attack]) != team) {
                        out_moves[result++] = attack;
                    }
                }
            } else {
                chess_value_t tmp = index_advance(team, index);
                if (tmp != -1) {
                    if (game_board[tmp] == -1) {
                        out_moves[result++] = tmp;
                    }
                    chess_value_t attack = index_advance_left(team, index);
                    if (attack != -1 && game_board[attack] != -1 && CHESS_TEAM(game_board[attack]) != team) {
                        out_moves[result++] = attack;
                    }
                    attack = index_advance_right(team, index);
                    if (attack != -1 && game_board[attack] != -1 && CHESS_TEAM(game_board[attack]) != team) {
                        out_moves[result++] = attack;
                    }
                }
                // check for en passant capture moves
                int tmp2 = index_left(team, index);
                if (tmp2 != -1 && game_board[tmp2] != -1 && CHESS_TEAM(game_board[tmp2]) != team) {
                    if (is_en_passant_target(game, tmp2)) {
                        tmp2 = index_advance_left(team, index);
                        if (tmp2 != -1 && game_board[tmp2] == -1) {
                            out_moves[result++] = tmp2;
                        }
                    }
                }
                tmp2 = index_right(team, index);
                if (tmp2 != -1 && game_board[tmp2] != -1 && CHESS_TEAM(game_board[tmp2]) != team) {
                    if (is_en_passant_target(game, tmp2)) {
                        tmp2 = index_advance_right(team, index);
                        if (tmp2 != -1 && game_board[tmp2] == -1) {
                            out_moves[result++] = tmp2;
                        }
                    }
                }
            }

            break;
        case CHESS_KNIGHT: {
            chess_value_t i1 = index_advance(team, index);
            chess_value_t i2 = index_advance(team, i1);
            const chess_value_t l2 = index_left(team, i2);
            const chess_value_t r2 = index_right(team, i2);
            chess_value_t tmp = index_left(team, i1);
            const chess_value_t l1 = index_left(team, tmp);
            tmp = index_right(team, i1);
            const chess_value_t r1 = index_right(team, tmp);

            i1 = index_retreat(team, index);
            i2 = index_retreat(team, i1);
            const chess_value_t l4 = index_left(team, i2);
            const chess_value_t r4 = index_right(team, i2);
            tmp = index_left(team, i1);
            const chess_value_t l3 = index_left(team, tmp);
            tmp = index_right(team, i1);
            const chess_value_t r3 = index_right(team, tmp);

            tmp = l1;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
            tmp = l2;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
            tmp = r1;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
            tmp = r2;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }

            tmp = l3;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
            tmp = l4;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
            tmp = r3;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
            tmp = r4;
            if (tmp != -1 && (game_board[tmp] == -1 || CHESS_TEAM(game_board[tmp]) != team)) {
                out_moves[result++] = tmp;
            }
        } break;
        case CHESS_BISHOP: {
            move_until_obstacle(index_advance_left, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_advance_right, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_retreat_left, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_retreat_right, team, index, game_board, out_moves, &result);
        } break;
        case CHESS_ROOK: {
            move_until_obstacle(index_advance, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_left, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_right, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_retreat, team, index, game_board, out_moves, &result);
        } break;
        case CHESS_QUEEN: {
            move_until_obstacle(index_advance, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_left, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_right, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_retreat, team, index, game_board, out_moves, &result);

            move_until_obstacle(index_advance_left, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_advance_right, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_retreat_left, team, index, game_board, out_moves, &result);
            move_until_obstacle(index_retreat_right, team, index, game_board, out_moves, &result);
        } break;
        case CHESS_KING: {
            chess_value_t i = index;
            if (i != -1) {
                i = index_advance(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_advance_left(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_advance_right(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_left(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_right(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_retreat(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_retreat_left(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
            i = index;
            if (i != -1) {
                i = index_retreat_right(team, i);
                if (i != -1 && (game_board[i] == -1 || CHESS_TEAM(game_board[i]) != team)) {
                    out_moves[result++] = i;
                }
            }
        } break;
    }
    return result;
}
chess_bool_t chess_contains_move(const chess_value_t* moves, chess_size_t moves_size, chess_index_t index) {
    for (chess_value_t i = 0; i < moves_size; ++i) {
        if (moves[i] == index) {
            return CHESS_TRUE;
        }
    }
    return CHESS_FALSE;
}
static chess_value_t is_checked_king(const chess_game_t* game, chess_value_t king_index, const chess_value_t* game_board) {
    chess_value_t tmp_moves[64];
    chess_value_t result = 0;
    if (king_index == -1) {
        return 0;
    }
    const chess_value_t id = game_board[king_index];
    if (id == -1) {
        return 0;
    }
    const chess_value_t team = CHESS_TEAM(id);
    chess_value_t c = 0;
    for (chess_value_t i = 0; i < 64; ++i) {
        const chess_value_t id_cmp = game_board[i];
        if (id_cmp != -1) {
            const chess_value_t team_cmp = CHESS_TEAM(id_cmp);
            if (team_cmp != team) {
                // found an opposing piece on the board
                const chess_value_t tmp_moves_size = compute_moves(game, i, tmp_moves, game_board);
                if (chess_contains_move(tmp_moves, tmp_moves_size, king_index)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
static chess_value_t compute_check_moves(const chess_game_t* game, chess_value_t index, chess_value_t checked_king_index, const chess_value_t* game_board, chess_value_t* out_moves) {
    chess_value_t tmp_moves[64];
    chess_value_t tmp_board[64];
    chess_value_t result = 0;
    const chess_value_t id = game_board[index];
    const chess_value_t checked_king_id = game_board[checked_king_index];
    if (CHESS_KING != CHESS_TYPE(checked_king_id)) {
        return 0;
    }
    if (CHESS_TEAM(checked_king_id) != CHESS_TEAM(id)) {
        return 0;
    }
    chess_value_t tmp_moves_sz = compute_moves(game, index, tmp_moves, game_board);
    for (chess_value_t i = 0; i < tmp_moves_sz; ++i) {
        memcpy(tmp_board, game_board, sizeof(tmp_board));
        tmp_board[index] = -1;
        tmp_board[tmp_moves[i]] = id;
        if (CHESS_KING == CHESS_TYPE(id)) {
            if (!is_checked_king(game, tmp_moves[i], tmp_board)) {
                out_moves[result++] = tmp_moves[i];
            }
        } else {
            if (!is_checked_king(game, checked_king_index, tmp_board)) {
                out_moves[result++] = tmp_moves[i];
            }
        }
    }
    return result;
}
static void eliminate_checked_moves(const chess_game_t* game, chess_value_t index, const chess_value_t* kings, const chess_value_t* game_board, chess_value_t* in_out_moves, chess_value_t* in_out_moves_size) {
    if (index == -1) {
        return;
    }
    chess_value_t tmp_board[64];
    const chess_value_t id = game_board[index];
    const chess_value_t team = CHESS_TEAM(id);
    const chess_value_t type = CHESS_TYPE(id);
    if (kings[team] != index) {  // other piece
        const chess_value_t king_index = kings[team];
        for (chess_value_t i = 0; i < *(in_out_moves_size); ++i) {
            memcpy(tmp_board, game_board, sizeof(tmp_board));
            tmp_board[index] = -1;
            tmp_board[in_out_moves[i]] = id;
            if (is_checked_king(game, king_index, tmp_board)) {
                for (chess_value_t j = i; j < (*in_out_moves_size) - 1; ++j) {
                    in_out_moves[j] = in_out_moves[j + 1];
                }
                --(*in_out_moves_size);
                --i;
            }
        }
    } else {  // king piece
        const chess_value_t king_index = index;
        const chess_value_t king_id = game_board[king_index];
        for (chess_value_t i = 0; i < (*in_out_moves_size); ++i) {
            memcpy(tmp_board, game_board, sizeof(tmp_board));
            tmp_board[king_index] = -1;
            const chess_value_t move = in_out_moves[i];
            tmp_board[move] = king_id;
            if (is_checked_king(game, move, tmp_board)) {
                for (chess_value_t j = i; j < (*in_out_moves_size) - 1; ++j) {
                    in_out_moves[j] = in_out_moves[j + 1];
                }
                --(*in_out_moves_size);
                --i;
            }
        }
    }
}
void chess_init(chess_game_t* out_game) {
    out_game->turn = 0;
    out_game->no_castle[0] = 0;
    out_game->no_castle[1] = 0;
    for (int i = 0; i < 16; ++i) {
        out_game->en_passant_targets[i] = -1;
    }
    for (int i = 0; i < 64; ++i) {
        out_game->board[i] = -1;
    }
    for (int i = 0; i < 8; ++i) {
        out_game->board[8 + i] = CHESS_ID(0, CHESS_PAWN);
        out_game->board[48 + i] = CHESS_ID(1, CHESS_PAWN);
    }
    out_game->board[0] = CHESS_ID(0, CHESS_ROOK);
    out_game->board[7] = CHESS_ID(0, CHESS_ROOK);
    out_game->board[1] = CHESS_ID(0, CHESS_KNIGHT);
    out_game->board[6] = CHESS_ID(0, CHESS_KNIGHT);
    out_game->board[2] = CHESS_ID(0, CHESS_BISHOP);
    out_game->board[5] = CHESS_ID(0, CHESS_BISHOP);
    out_game->board[3] = CHESS_ID(0, CHESS_KING);
    out_game->kings[0] = 3;
    out_game->board[4] = CHESS_ID(0, CHESS_QUEEN);

    out_game->board[56 + 0] = CHESS_ID(1, CHESS_ROOK);
    out_game->board[56 + 7] = CHESS_ID(1, CHESS_ROOK);
    out_game->board[56 + 1] = CHESS_ID(1, CHESS_KNIGHT);
    out_game->board[56 + 6] = CHESS_ID(1, CHESS_KNIGHT);
    out_game->board[56 + 2] = CHESS_ID(1, CHESS_BISHOP);
    out_game->board[56 + 5] = CHESS_ID(1, CHESS_BISHOP);
    out_game->board[56 + 3] = CHESS_ID(1, CHESS_QUEEN);
    out_game->board[56 + 4] = CHESS_ID(1, CHESS_KING);
    out_game->kings[1] = 56 + 4;
}
static chess_value_t compute_castling(const chess_game_t* game, chess_value_t index, chess_value_t queen_side) {
    // do not call if if not your turn.
    const chess_value_t id = game->board[index];
    const chess_type_t type = CHESS_TYPE(id);
    const chess_value_t team = CHESS_TEAM(id);
    chess_value_t index_other = -1;
    if (type != CHESS_KING && type != CHESS_ROOK) {
        return -1;
    }

    if (game->no_castle[team]) {
        return -1;
    }
    if (team == 0) {
        if (type == CHESS_KING) {
            if (queen_side) {
                index_other = 7;  // the queen-side rook
                // no pieces between king and rook?
                for (int i = index + 1; i < index_other; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            } else {
                index_other = 0;
                // no pieces between king and rook?
                for (int i = index_other + 1; i < index; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            }
        } else {
            queen_side = index == 7;
            index_other = 3;
            if (queen_side) {
                for (int i = index_other + 1; i < index; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            } else {
                for (int i = index + 1; i < index_other; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            }
        }
    } else {
        if (type == CHESS_KING) {
            if (queen_side) {
                index_other = 56 + 0;  // the queen-side rook
                for (int i = index_other + 1; i < index; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            } else {
                index_other = 56 + 7;
                for (int i = index + 1; i < index_other; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            }
        } else {
            queen_side = index == 56 + 7;
            index_other = 56 + 4;
            if (queen_side) {
                for (int i = index + 1; i < index_other; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            } else {
                for (int i = index_other + 1; i < index; ++i) {
                    if (game->board[i] != -1) {
                        return -1;
                    }
                }
            }
        }
    }
    // now we have to see if a piece is attacking any square between this one and the other index, inclusive
    chess_value_t tmp_moves[64];
    chess_value_t tmp_moves_size;
    if (index_other > index) {
        for (int i = index; i <= index_other; ++i) {
            for (int j = 0; j < 64; ++j) {
                const chess_value_t cmp_id = game->board[j];
                if (-1 != cmp_id && CHESS_TEAM(cmp_id) != team) {
                    // opposing piece
                    tmp_moves_size = compute_moves(game, j, tmp_moves, game->board);
                    if (chess_contains_move(tmp_moves, tmp_moves_size, i)) {
                        return -1;
                    }
                }
            }
        }
    } else {
        for (int i = index_other; i <= index; ++i) {
            for (int j = 0; j < 64; ++j) {
                const chess_value_t cmp_id = game->board[j];
                if (-1 != cmp_id && CHESS_TEAM(cmp_id) != team) {
                    // opposing piece
                    tmp_moves_size = compute_moves(game, j, tmp_moves, game->board);
                    if (chess_contains_move(tmp_moves, tmp_moves_size, i)) {
                        return -1;
                    }
                }
            }
        }
    }

    return index_other;
}
chess_value_t chess_move(chess_game_t* game, chess_value_t index_from, chess_value_t index_to) {
    if (game == NULL || index_from < 0 || index_from > 63 || index_to < 0 || index_to > 63 || index_from == index_to) {
        return -2;
    }

    const chess_value_t id = game->board[index_from];
    const chess_type_t type = CHESS_TYPE(id);
    const chess_value_t team = CHESS_TEAM(id);
    if (game->turn != team) {
        return -2;
    }
    chess_value_t tmp_moves[64];
    chess_value_t tmp_moves_size = 0;
    chess_value_t index_other = -1;
    const chess_value_t king_index = game->kings[team];
    if (is_checked_king(game, king_index, game->board)) {
        tmp_moves_size = compute_check_moves(game, index_from, king_index, game->board, tmp_moves);
    } else {
        tmp_moves_size = compute_moves(game, index_from, tmp_moves, game->board);
        // castle if possible
        index_other = compute_castling(game, index_from, 0);
        if (index_other != index_to) {
            index_other = compute_castling(game, index_from, 1);
        }
        if (index_other == index_to) {
            game->no_castle[team] = 1;
            const chess_value_t other_id = game->board[index_other];
            game->board[index_to] = game->board[index_from];
            game->board[index_from] = other_id;
            if (type == CHESS_KING) {
                game->kings[team] = index_to;
            }
            // still our turn
            return -1;
        }
    }
    if (chess_contains_move(tmp_moves, tmp_moves_size, index_to)) {
        char added = 0;
        chess_value_t result = -1;
        if (type == CHESS_PAWN) {
            clear_en_passant_target(game, index_from);
            // check for en passant target eligibility.
            // we can tell if it's the first move advanced by two
            // simply by checking the index for a difference of 16
            // the only way it gets that is moving two and the only
            // time that can happen is first move
            if (team == 0) {
                if (index_from == index_to - 16) {
                    add_en_passant_target(game, index_to);
                    added = 1;
                }
            } else if (team == 1) {
                if (index_from == index_to + 16) {
                    add_en_passant_target(game, index_to);
                    added = 1;
                }
            }
            const chess_value_t attack_index = en_passant_target_from_move(game, index_from, index_to, game->board);
            if (attack_index != -1) {
                game->board[attack_index] = -1;
                result = attack_index;
            }
        }
        if (game->board[index_to] != -1 && CHESS_TEAM(game->board[index_to]) != team) {
            result = index_to;
        }
        game->board[index_to] = game->board[index_from];
        if (!added && game->board[index_to] != -1) {
            clear_en_passant_target(game, index_to);
        }
        if (++game->turn > 1) {
            game->turn = 0;
        }
        if (CHESS_TYPE(id) == CHESS_KING) {
            game->kings[CHESS_TEAM(id)] = index_to;
        }
        game->board[index_from] = -1;
        return result;
    }
    return -2;
}
chess_size_t chess_compute_moves(const chess_game_t* game, chess_index_t index, chess_index_t* out_moves) {
    if (game == NULL || index < 0 || index > 63) {
        return 0;
    }
    const chess_value_t id = game->board[index];
    if (game->turn != CHESS_TEAM(id)) {
        return 0;
    }
    chess_value_t result = 0;
    const chess_value_t king_index = game->kings[CHESS_TEAM(id)];
    if (is_checked_king(game, king_index, game->board)) {
        result = compute_check_moves(game, index, king_index, game->board, out_moves);
    } else {
        result = compute_moves(game, index, out_moves, game->board);
        eliminate_checked_moves(game, index, game->kings, game->board, out_moves, &result);
        chess_value_t index_other = compute_castling(game, index, 0);
        if (index_other != -1) {
            out_moves[result++] = index_other;
        }
        index_other = compute_castling(game, index, 1);
        if (index_other != -1) {
            out_moves[result++] = index_other;
        }
    }
    return result;
}
chess_team_t chess_turn(const chess_game_t* game) {
    return game->turn;
}
chess_value_t chess_index_to_id(const chess_game_t* game, chess_value_t index) {
    if (game == NULL || index < 0 || index > 63) {
        return -1;
    }
    return game->board[index];
}
chess_status_t chess_status(const chess_game_t* game, chess_team_t team) {
    const chess_value_t index = game->kings[(int)team];
    chess_value_t moves[64];
    if (is_checked_king(game, index, game->board)) {
        if (0 == chess_compute_moves(game, index, moves)) {
            return CHESS_CHECKMATE;
        }
        return CHESS_CHECK;
    } else {
        if (0 == chess_compute_moves(game, index, moves)) {
            return CHESS_STALEMATE;
        }
    }
    return CHESS_NORMAL;
}
chess_result_t chess_promote_pawn(chess_game_t* game, chess_index_t index, chess_type_t new_type) {
    if (game == NULL || index < 0 || index > 63 || new_type == CHESS_PAWN) {
        return CHESS_INVALID;
    }
    const chess_value_t id = game->board[index];
    const chess_value_t team = CHESS_TEAM(id);
    if (CHESS_TYPE(id) != CHESS_PAWN || team != game->turn) {
        return 0;
    }
    if (team == 0) {
        if (index < (64 - 8)) {
            return 0;
        }
    } else {
        if (index > 7) {
            return 0;
        }
    }
    clear_en_passant_target(game, index);
    game->board[index] = CHESS_ID(team, new_type);
    return CHESS_SUCCESS;
}
chess_result_t chess_index_name(chess_index_t index, char* out_buffer) {
    if (index < 0 || index > 63 || out_buffer == NULL) {
        return CHESS_INVALID;
    }
    const chess_value_t x = index % 8;
    const chess_value_t y = (7 - index / 8) + 1;
    sprintf(out_buffer, "%c%d", x + 'a', y);
    return CHESS_SUCCESS;
}
