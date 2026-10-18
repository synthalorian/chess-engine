![chess-engine](.github/preview.png)

<p align="center">
  <img src=".github/assets/synthalorian-idle-128.gif" width="128" height="128" alt="The synthalorian: a pixel-art figure in a black helmet with a pink T-visor, idling">
</p>
<p align="center"><em>A from-scratch UCI chess engine: bitboards, alpha-beta and a neon board.</em></p>

# Chess Engine

> *"Every position has a truth. The engine just searches deeper than you can dream."*

A chess engine built from scratch. No libraries for move generation, no copied evaluation tables. Just bitboards, alpha-beta, and the will to find the best move.

## What It Is

- UCI-compatible engine for Arena, Cute Chess, and command-line analysis
- Bitboard-based board representation for maximum speed
- Legal move generation with full rules support (pins, checks, castling, en passant)
- Iterative deepening alpha-beta search with quiescence
- Target: ~2000 ELO in self-play within v3

## UCI Protocol

```
> uci
< id name NeonKnight
< id author Unknown
< uciok
> position startpos moves e2e4
> go depth 10
< info depth 10 score cp 45 nodes 1420034 time 1240 pv e2e4 e7e5 g1f3
< bestmove e2e4
```

Compatible with:
- [Arena](http://www.playwitharena.com/)
- [Cute Chess](https://cutechess.com/)
- [Lichess Bot API](https://lichess.org/api#tag/Bot) (v4)

## Features

| Feature | Version | Status |
|---------|---------|--------|
| Bitboard representation | v1 | Planned |
| Legal move generation | v1 | Planned |
| Make / unmake move | v1 | Planned |
| Alpha-beta search | v2 | Planned |
| Quiescence search | v2 | Planned |
| Iterative deepening | v2 | Planned |
| Transposition table | v3 | Planned |
| Null move pruning | v3 | Planned |
| Late move reduction | v3 | Planned |
| Opening book | v3 | Planned |
| UCI protocol | v4 | Planned |
| Lichess bot | v4 | Planned |

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./neonknight uci
```

Requires C++20. Tested on GCC 13+ and Clang 16+.

## Quick Start

```bash
# UCI mode
./neonknight uci

# Single analysis
./neonknight --fen "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R" --depth 12

# Self-play benchmark
./neonknight --bench
```

## ELO Target

| Version | Target ELO | Notes |
|---------|-----------|-------|
| v1 | 1200 | Legal moves only, random play |
| v2 | 1600 | Alpha-beta + basic eval |
| v3 | 2000 | Full search enhancements |
| v4 | 2200+ | Opening book + tablebase prep |

---

*Sixty-four squares. Two armies. One optimal line. Search on.*

---

## ☕ Support the Developer

If this project saved you time, solved a problem, or just made your day a little more neon, you can fuel the next one:

[![Buy Me A Coffee](https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png)](https://buymeacoffee.com/synthalorian)

---

<p align="center">
  <img src=".github/assets/synthalorian-idle-128.gif" width="32" height="32" alt="The synthalorian sprite"><br>
  <sub>Built by <a href="https://github.com/synthalorian">the synthalorian</a></sub>
  <br><sub>AI-assisted art: the banner image at the top of this README was made with AI image tools. The synthalorian sprite is code-drawn pixel art.</sub>
</p>
