# BadBishop Engine Architecture Documentation

BadBishop is a classical, self-contained UCI chess engine written in standard C99. It is designed to achieve 2000–2200+ Elo on both general-purpose desktop CPUs (x86-64/ARM64) and bare-metal microcontrollers such as the Raspberry Pi Pico 2 WH (RP2350, 520 KB SRAM).

## Architectural Overview

BadBishop uses a **hybrid board representation** that pairs a 120-square mailbox array for fast ray-cast move generation with 64-bit bitboards for single-cycle pawn structure evaluation, backed by 64-bit Zobrist hashing, a zero-malloc Transposition Table, and a 3-mode Personality evaluation engine.

### Documentation Modules

1. **[01 — Hybrid Board Representation](./01-board-representation.md)**
   120-square mailbox geometry, sentinel padding, 64-bit pawn bitboards, piece lists, and coordinate conversion tables.
2. **[02 — Zobrist Hashing & Position Keys](./02-zobrist-hashing.md)**
   64-bit XOR hashing, PRNG key table initialization, collision math, and incremental update mechanics.
3. **[03 — Board State, FEN Parsing & Integrity (`CheckBoard`)](./03-state-and-integrity.md)**
   `S_BOARD` memory layout, exact RAM budget (PC vs. RP2350), FEN state hydration, and automated invariant assertions.
4. **[04 — Attack Detection & Move Generation](./04-attacks-and-movegen.md)**
   Reverse ray-casting (`SqAttacked`), 25-bit move integer bit-packing, stack-allocated move lists, `GenerateAllMoves()`, and tactical capture generation (`GenerateAllCaps()`).
5. **[05 — State Transitions (`MakeMove` / `TakeMove`) & Perft Verification](./05-makemove-and-perft.md)**
   In-place state mutation, $O(1)$ piece list swap-deletion, branchless castling permission masks, legal check filtering, and canonical Perft tree verification.
6. **[06 — Static Evaluation & Engine Personalities](./06-evaluation-and-personalities.md)**
   64-bit pawn structure bitmasks, phase-tapered Piece-Square Tables (PSTs), open-file mobility, King safety, and the 3 Personality Modes (*Balanced, Positional, Sharp*).
7. **[07 — Transposition Table & Alpha-Beta Search](./07-transposition-and-search.md)**
   Compile-time scaled zero-malloc Transposition Table (`pvtable.c`), Iterative Deepening, Negamax Alpha-Beta, Quiescence Search with Delta Pruning, and Move Ordering (PV/TT, MVV-LVA, Killers, History).

---

## Global Complexity & Memory Summary

| Subsystem | Source Files | Static RAM (Tables) | Per-Board RAM | Runtime Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Coordinate Lookup** | `init.c` | `1,696 B` | `0 B` | $O(1)$ array lookup |
| **Mailbox & Bitboards** | `board.c`, `bitboards.c` | `256 B` | `1,168 B` (active core) | $O(1)$ bit/square access |
| **Zobrist Hashing** | `hashkeys.c` | `12,616 B` | `8 B` (`posKey`) | $O(N)$ init, $O(1)$ incremental |
| **Undo History Stack** | `defs.h` | `0 B` | `32,768 B` (2048 plies) | $O(1)$ push/pop |
| **Attack Detection** | `attack.c` | `336 B` | `0 B` | $O(1)$ leapers, $O(R)$ sliders ($R \le 7$) |
| **Move Generation** | `movegen.c`, `io.c` | `512 B` | `2,052 B` (`S_MOVELIST`) | $O(P + S \cdot R)$ per node |
| **Make / Take Move** | `makemove.c` | `480 B` (`CastlePerm`) | `0 B` (uses `history[]`) | $O(1)$ state transition |
| **Perft Verification** | `perft.c` | `8 B` (`leafNodes`) | `2,052 B` per ply (stack) | $O(B^D)$ exhaustive tree walk |
| **Static Evaluation** | `evaluate.c`, `init.c` | `3,504 B` (Masks + PSTs) | `4 B` (`pos->style`) | $O(P)$ active pieces on board |
| **Transposition Table** | `pvtable.c` | `192 KB` (Pico) / `6 MB` (PC) | `256 B` (`PvArray[64]`) | $O(1)$ bitwise-AND probe/store |
| **Alpha-Beta Search** | `search.c` | `728 B` (`MvvLvaScores`) | `6,752 B` (History + Killers) | $O(B^{D/2})$ with optimal ordering |

---

## Build & Verification Quick Reference

```bash
# Configure and compile (Release mode)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Execute all 11 unit, Perft, Evaluation, and Search test suites
ctest --test-dir build --output-on-failure -V

# Build Sphinx HTML Documentation
sphinx-build -b html docs build/docs
```

```{toctree}
:maxdepth: 2
:caption: Engine Architecture Modules
:hidden:

01-board-representation
02-zobrist-hashing
03-state-and-integrity
04-attacks-and-movegen
05-makemove-and-perft
06-evaluation-and-personalities
07-transposition-and-search
api/library_root
```