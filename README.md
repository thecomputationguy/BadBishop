# ♝ BadBishop

[![Documentation](https://img.shields.io/badge/docs-Sphinx%20%7C%20Live-blue?style=for-the-badge&logo=readthedocs)](https://thecomputationguy.github.io/BadBishop/)
[![Language](https://img.shields.io/badge/C99-Zero--Malloc-00599C?style=for-the-badge&logo=c)](#)
[![Target](https://img.shields.io/badge/Target-PC%20%26%20RP2350%20(Pico%202)-C51A4A?style=for-the-badge&logo=raspberrypi)](#)

A classical, search-based chess engine written in C.

👉 **[Read the Full Architecture & Complexity Documentation (Sphinx Portal)](https://thecomputationguy.github.io/BadBishop/)**

---

## Overview

BadBishop is a hobby chess engine project aimed at playing competitive chess around the ~2000+ Elo mark. Engineered to run on both desktop CPUs and bare-metal microcontrollers such as the **Raspberry Pi Pico 2 WH (RP2350, 520 KB SRAM)** with zero dynamic memory allocation (`malloc`), the engine utilizes:

* **Hybrid Board Representation:** A 120-square mailbox board representation paired with 64-bit pawn bitboards and 64-bit Zobrist hashing.
* **Selective Search Architecture:** Negamax Alpha-Beta pruning, Quiescence Search with Delta Pruning, Iterative Deepening, a compile-time scaled Transposition Table, and 4-tier Move Ordering (PV/TT, MVV-LVA, Killer Moves, and History Heuristic).
* **Three Engine Personalities:** Configurable playing styles (**Balanced**, **Positional**, and **Sharp**) driven by fixed-point evaluation weights and dynamic draw `Contempt`.

BadBishop is designed as a standalone engine communicating via the standard Universal Chess Interface (UCI) protocol. To play against it or watch it play, connect the compiled binary to a chess GUI such as [Arena](http://www.playwitharena.de/), [Cute Chess](https://cutechess.com/), or [Banksia GUI](https://banksiagui.com/).

---

## Build & Verification Quick Start

```bash
# 1. Configure and compile (Release mode)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 2. Run the 11-stage Unit & Canonical Perft Verification Suite
ctest --test-dir build -C Release --output-on-failure -V

# 3. Build Sphinx HTML Documentation locally (Optional)
pip install -r docs/requirements.txt
sphinx-build -b html docs build/docs
```

---

## Documentation Modules

The [`docs/`](./docs/index.md) directory contains detailed breakdowns of every subsystem, including memory footprints and runtime complexities:

1. **[01 — Hybrid Board Representation](./docs/01-board-representation.md)**
2. **[02 — Zobrist Hashing & Position Keys](./docs/02-zobrist-hashing.md)**
3. **[03 — Board State, FEN Parsing & Integrity (`CheckBoard`)](./docs/03-state-and-integrity.md)**
4. **[04 — Attack Detection & Move Generation](./docs/04-attacks-and-movegen.md)**
5. **[05 — State Transitions (`MakeMove` / `TakeMove`) & Perft Verification](./docs/05-makemove-and-perft.md)**
6. **[06 — Static Evaluation & Engine Personalities](./docs/06-evaluation-and-personalities.md)**
7. **[07 — Transposition Table & Alpha-Beta Search](./docs/07-transposition-and-search.md)**

---

## Acknowledgments & Credits

This project was inspired by and structurally guided by the classic chess programming tutorial series by **Bluefever Software**.

Special thanks and full credit to the author for the open-source reference implementation of the Vice chess engine:
- **Vice Repository:** [https://github.com/bluefeversoft/vice](https://github.com/bluefeversoft/vice)
- **Author:** Bluefever Software