# BadBishop Engine Architecture Documentation

BadBishop is a classical, self-contained UCI chess engine written in standard C99. It is designed to achieve 2000–2200+ Elo on both general-purpose desktop CPUs (x86-64/ARM64) and bare-metal microcontrollers such as the Raspberry Pi Pico 2 WH (RP2350, 520 KB SRAM).

## Architectural Overview

BadBishop uses a **hybrid board representation** that pairs a 120-square mailbox array for fast ray-cast move generation with 64-bit bitboards for single-cycle pawn structure evaluation, backed by 64-bit Zobrist hashing for state indexing.

### Documentation Modules

1. **[01 — Hybrid Board Representation](./01-board-representation.md)**
   120-square mailbox geometry, sentinel padding, 64-bit pawn bitboards, piece lists, and coordinate conversion tables.
2. **[02 — Zobrist Hashing & Position Keys](./02-zobrist-hashing.md)**
   64-bit XOR hashing, PRNG key table initialization, collision math, and incremental update mechanics.
3. **[03 — Board State, FEN Parsing & Integrity (`CheckBoard`)](./03-state-and-integrity.md)**
   `S_BOARD` memory layout, exact RAM budget (PC vs. RP2350), FEN state hydration, and automated invariant assertions.
4. **[04 — Attack Detection & Move Generation](./04-attacks-and-movegen.md)**
   Reverse ray-casting (`SqAttacked`), 25-bit move integer bit-packing, stack-allocated move lists, and table-driven pseudo-legal move generation.

---

## Global Complexity & Memory Summary

| Subsystem | Source Files | Static RAM (Tables) | Per-Board RAM | Runtime Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Coordinate Lookup** | `init.c` | `1,696 B` | `0 B` | $O(1)$ array lookup |
| **Mailbox & Bitboards** | `board.c`, `bitboards.c` | `256 B` | `1,164 B` (active core) | $O(1)$ bit/square access |
| **Zobrist Hashing** | `hashkeys.c` | `13,608 B` | `8 B` (`posKey`) | $O(N)$ init, $O(1)$ incremental |
| **Undo History Stack** | `defs.h` | `0 B` | `32,768 B` (2048 plies) | $O(1)$ push/pop |
| **Attack Detection** | `attack.c` | `336 B` | `0 B` | $O(1)$ leapers, $O(R)$ sliders ($R \le 7$) |
| **Move Generation** | `movegen.c`, `io.c` | `512 B` | `2,052 B` (`S_MOVELIST`) | $O(P + S \cdot R)$ per node |

---

## Build & Verification Quick Reference

```bash
# Configure and compile (Release mode)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Execute unit and integrity test suite
ctest --test-dir build --output-on-failure -V