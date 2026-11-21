# UCI Protocol & GUI Integration (`src/uci.c`)

BadBishop implements the **Universal Chess Interface (UCI)** over unbuffered `stdin`/`stdout` (`setvbuf(..., _IONBF, 0)`), allowing direct connection to chess GUIs such as Arena, Cute Chess, and Banksia.

## Supported UCI Commands

| Command | Handler | Behavior |
| :--- | :--- | :--- |
| `uci` | `UciLoop()` | Identifies `BadBishop`, advertises `Hash` and `Personality` options, and prints `uciok`. |
| `isready` | `UciLoop()` | Synchronizes engine state with the GUI and responds `readyok`. |
| `ucinewgame` | `UciLoop()` | Clears the Transposition Table (`ClearHashTable`) and resets to `START_FEN`. |
| `setoption` | `ParseSetOption()` | Dynamically resizes the Transposition Table (`Hash`) or updates `pos->style` (`Personality`). |
| `position` | `ParsePosition()` | Loads `startpos` or a custom `fen` string, restores active `currentStyle`, and replays algebraic `moves` via `ParseMove()`. |
| `go` | `ParseGo()` | Parses `wtime`, `btime`, `winc`, `binc`, `movestogo`, `movetime`, and `depth`, allocates search time, and calls `SearchPosition()`. |
| `quit` | `UciLoop()` | Sets `info->quit = TRUE` and exits cleanly. |

## Exposed UCI Options

* **`Hash` (`spin`, default `16`, min `1`, max `96`):** Calls `InitHashTable(HashTable, mb)` to reconfigure the active power-of-two Transposition Table window in megabytes.
* **`Personality` (`combo`, default `Balanced`):** Switches static evaluation weights between `Balanced`, `Positional`, and `Sharp` across games.

## Algebraic Move Parsing (`ParseMove` in `src/io.c`)

`ParseMove(const char *ptrChar, S_BOARD *pos)` translates 4- or 5-character UCI coordinate strings (e.g., `e2e4`, `e1g1`, `a7a8q`) into internal 32-bit packed `S_MOVE` integers by generating all pseudo-legal moves for the current position and matching `FROMSQ`, `TOSQ`, and `PROMOTED` piece flags.