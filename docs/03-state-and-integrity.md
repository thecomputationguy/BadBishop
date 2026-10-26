# 03 — Board State, FEN Parsing & Integrity (`CheckBoard`)

**Source Files:** `src/defs.h`, `src/board.c`, `tests/test_perft.c`

Because BadBishop maintains redundant data structures for speed (a 120-square mailbox, three 64-bit pawn bitboards, piece lists, incremental material counters, and a Zobrist key), keeping them synchronized is the #1 correctness requirement of the engine.

---

## 1. Complete `S_BOARD` Memory Layout

During search, BadBishop **never copies `S_BOARD`**. A single `S_BOARD` instance lives in memory, modified in-place by `MakeMove()` and restored by `TakeMove()` using the `history[]` undo stack.

| Field | C Type | Size (Bytes) | Role |
| :--- | :--- | :--- | :--- |
| `pieces[120]` | `int[120]` | `480 B` | 120-square mailbox array (`EMPTY`, `wP..bK`, `OFFBOARD`) |
| `pawns[3]` | `U64[3]` | `24 B` | 64-bit bitboards for `WHITE`, `BLACK`, and `BOTH` pawns |
| `KingSq[2]` | `int[2]` | `8 B` | Fast $O(1)$ lookup for White and Black King 120-squares |
| `side`, `enPas`, `fiftyMove` | `int` $\times 3$ | `12 B` | Active side, En Passant square (`NO_SQ=99`), 50-move counter |
| `ply`, `hisPly` | `int` $\times 2$ | `8 B` | Current search depth ply and total game half-moves |
| `castlePerm` | `int` | `4 B` | 4-bit bitmask (`WKCA=1, WQCA=2, BKCA=4, BQCA=8`) |
| `posKey` | `U64` | `8 B` | 64-bit Zobrist hash key of current position |
| `pceNum[13]` | `int[13]` | `52 B` | Count of each piece type currently on the board |
| `bigPce[3]`, `majPce[3]`, `minPce[3]` | `int[3]` $\times 3$ | `36 B` | Non-pawn, Rook/Queen, and Bishop/Knight counts per side |
| `material[3]` | `int[3]` | `12 B` | Incremental centipawn material score (`WHITE`, `BLACK`, `BOTH`) |
| `pList[13][10]` | `int[13][10]` | `520 B` | Piece list storing 120-square coordinates per piece type |
| **Active Board Core Subtotal** | — | **`1,164 B` (~1.14 KB)** | **Fits entirely inside CPU L1 Data Cache (32 KB)** |
| `history[2048]` (`S_UNDO`) | `S_UNDO[2048]` | `32,768 B` (32 KB) | Stores irreversible state (`move, castlePerm, enPas, fiftyMove, posKey`) |
| **Total `S_BOARD` Footprint** | — | **`33,932 B` (~33.1 KB)** | **6.3% of Raspberry Pi Pico 2 WH's 520 KB SRAM** |

---

## 2. FEN Parsing (`ParseFen` in `board.c`)

`ParseFen(const char *fen, S_BOARD *pos)` translates a standard 6-field Forsyth–Edwards Notation string into a hydrated `S_BOARD` state in $O(1)$ time (bounded by string length $\le 90$ chars + 64 board squares).

### Example Trace
Given the FEN:
```text
r1bqk2r/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQK2R w KQkq d6 0 8

```

1. **`ResetBoard(pos)`:** Sets all 120 squares to `OFFBOARD`, the inner 64 squares to `EMPTY`, zeros all counters/bitboards, and clears `pList` to `NO_SQ`.
2. **Field 1 (Piece Placement):** Walks from `RANK_8` down to `RANK_1`, `FILE_A` to `FILE_H`. Digits `'1'..'8'` advance `file` by `count`; piece characters populate `pos->pieces[sq120]`.
3. **Field 2 (Active Color):** `'w'` sets `pos->side = WHITE`.
4. **Field 3 (Castling):** `'KQkq'` sets `pos->castlePerm = 15` (`0b1111`).
5. **Field 4 (En Passant):** `'d6'` converts `file = 'd' - 'a' = 3` and `rank = '6' - '1' = 5` via `FR2SQ(3, 5)` into `sq120 = 74` (`D6`).
6. **Hydration Pass:** Loops `sq64 = 0..63`, populating `bigPce`, `majPce`, `minPce`, `material`, `pList`, `pceNum`, `KingSq`, and `pawns[]`.
7. **Hash Generation:** Calls `pos->posKey = GeneratePosKey(pos)`.

---

## 3. Self-Auditing State Verification (`CheckBoard`)

`CheckBoard(const S_BOARD *pos)` is an automated invariant checker designed to run inside `assert(CheckBoard(pos))` during debug builds and unit testing. When compiled with `-DNDEBUG` (Release mode), `assert()` compiles out to **zero instructions**.

### What `CheckBoard()` Verifies (10 Invariant Checks)

1. **Piece Count Parity:** Scans `pos->pieces[]` across all 64 squares and asserts `t_pceNum[p] == pos->pceNum[p]` for all 12 piece types.
2. **Pawn Bitboard Population:** Verifies `CountBits(pos->pawns[c]) == pos->pceNum[wP/bP]`.
3. **Pawn Bitboard Square Alignment:** Pops every bit from `t_pawns[WHITE]` and `t_pawns[BLACK]` and asserts `pos->pieces[SQ120(sq64)]` actually holds a `wP` or `bP`.
4. **Material & Category Totals:** Recalculates `t_material[]`, `t_bigPce[]`, `t_majPce[]`, and `t_minPce[]` from scratch and asserts exact equality with incremental values.
5. **Side Validity:** Asserts `pos->side == WHITE || pos->side == BLACK`.
6. **Zobrist Hash Sync:** Re-runs `GeneratePosKey(pos)` from scratch and asserts it equals the incremental `pos->posKey`.
7. **En Passant Rank Legality:** If `pos->enPas != NO_SQ`, asserts it lies on `RANK_6` (when White to move) or `RANK_3` (when Black to move).
8. **King Square Sync:** Asserts `pos->pieces[pos->KingSq[WHITE]] == wK` and `pos->pieces[pos->KingSq[BLACK]] == bK`.
9. **Piece List Sync:** Loops through `pos->pList[p][0 .. pceNum[p]-1]` and asserts every listed coordinate contains piece `p` on `pos->pieces[]`.

### Runtime Cost

* **Debug Mode (`-g -O0`):** $O(64)$ per call (~350 CPU cycles).
* **Release Mode (`-O3 -DNDEBUG`):** $0$ cycles (stripped by preprocessor).