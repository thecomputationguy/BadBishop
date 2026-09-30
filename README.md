# BadBishop

A classical, search-based chess engine written in C.

## Overview
BadBishop is a hobby chess engine project aimed at playing competitive chess around the ~2000 Elo mark. The engine utilizes a classical 120-square mailbox board representation, alpha-beta pruning search, quiescence search, transposition tables, and standard chess evaluation heuristics.

BadBishop is designed as a standalone engine communicating via the standard Universal Chess Interface (UCI) protocol. To play against it or watch it play, connect the compiled binary to a chess GUI such as [Arena](http://www.playwitharena.de/), [Cute Chess](https://cutechess.com/), or [Banksia GUI](https://banksiagui.com/).

## Acknowledgments & Credits
This project was inspired by and structurally guided by the classic chess programming tutorial series by **Bluefever Software**. 

Special thanks and full credit to the author for the open-source reference implementation of the Vice chess engine:
* **Vice Repository:** [https://github.com/bluefeversoft/vice](https://github.com/bluefeversoft/vice)
* **Author:** Bluefever Software