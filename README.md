# htcw_chess

## A Chess Engine in C

This library implements the rules of chess. You can use it to implement a chess game in C or C++ on whatever platform you like. It should work with any modern C or C++ compiler, and has been tested with Microsoft's and GNU GCC.

It is compact, and perhaps not the most efficient implementation, CPU wise, as some of the checks it has to do - for example, for castling - are a bit intensive, but it shouldn't be noticable at all on a modern PC, nor on any 32-bit embedded MCU.

What it does do:

- It implements the rules of chess for you, including castling and en passant captures.
- It provides enough information that you can preview acceptable moves from any position.
- It keeps score according to the rules of chess.
- It can tell you whether the game is in check, checkmate, or a stalemate.

What it doesn't do:

- It does not do anything display or input related.
- It does not time moves.
- It does not play chess. It cannot automate a team.

### Using this mess

The library can be built using the available repos for PlatformIO (codewitch-honey-crisis/htcw_chess), or Arduino (htcw_chess), or it can be built using CMake.

You simply include "chess.h" where you want to use it.

Next you need to initialize the game, which sets the initial board, resets the scores, etc.

```c
chess_game_t game;
// cannot fail unless you pass in a null reference
// in which case the behavior is undefined.
chess_init(&game);
```

From there at some point you'll probably want to determine which team is up next:
```c
chess_team_t team = chess_turn(&game);
// will be CHESS_WHITE (0) or CHESS_BLACK (1)
```

Assuming a team is currently up you can move a team's piece like so:
```c
// capture contains the board index of a captured
// opponent or -1 if none, -2 if the move didn't happen
// because it was invalid.
chess_index_t capture = chess_move(&game, from, to);
```

The board is laid out in increasing indexes from top to bottom, left to right, where `0` is the top-left (team 0 side), and `63` is bottom right (team 1 side)
The canonical chess columns `a`-`h` in canonical row `1` are represented by indices 0-7 and so on.
You can get the canonical string representing the board square in chess terminology using `chess_index_name()`:
```c
char idx[3];
// idx will be filled with the string chess square representation such as "h3"
chess_index_name(board_index, idx);
```

Each category of chess piece has an id which is a combination of the chess piece type `chess_type_t` and the chess team `chess_team_t`.
You can retrieve the chess id for any index of the board using `chess_index_to_id()`:
```c
chess_id_t id = chess_index_to_id(&game, board_index);
```
Once you have an id, you can use the `CHESS_TEAM()` and `CHESS_TYPE()` macros to get the team and type from the id.
You can also compose an id using `CHESS_ID()` passing the team and the type.
```c
chess_team_t team = CHESS_TEAM(id);
chess_type_t type = CHESS_TYPE(id);
```
If you want a list of available moves from a particular board index you can pass a buffer `chess_index_t[64]` to `chess_compute_moves()` which it will fill with the available moves:
```c
chess_index_t next_moves[64];
size_t move_count = chess_compute_moves(&game, board_index, next_moves);
// now next_moves contains move_count items
```
If the available move is not among these, attempting to move will fail with `-2`

There's a helper function you can use to determine if the `next_moves` from above contains a particular index:
```c
bool found = chess_contains_move(next_moves, move_count, index);
```
When a pawn reaches the end of the board, you'll want to promote it. You'll probably want to promote it to queen, but you can choose other pieces, as per the rules of chess:
```c
// will fail if the board index isn't on a promotable pawn, not currently the 
// pawn's turn, or the new_type isn't a valid piece type for promotion 
chess_promote_pawn(&game, board_index, new_type);
```
At any point you can check the current status of the game using `chess_status()`:
```c
// returns a value indicating normal play, check, checkmate or stalemate
chess_status_t status = chess_status(&game,team);
```
You can check the score for a team at any time using `chess_score()`:
```c
chess_score_t score = chess_score(&game, team);
```

