# chess

A simple chess engine written in C.

- implements the rules of chess
- can preview available moves for a given piece, following the rules
- can report the status of the board for any team at any time

An example pocket chess game which uses it can be found [here](https://github.com/codewitch-honey-crisis/core2_chess)

```
[env:node32s]
platform = espressif32
board = node32s
framework = arduino
lib_deps = 
	codewitch-honey-crisis/htcw_chess
```
