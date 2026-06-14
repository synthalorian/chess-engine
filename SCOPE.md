# Scope of Work — Chess Engine

## v1: Foundation

**Goal**: The board knows the rules. Every legal move is generated.

- [ ] Bitboard board representation (6 piece types × 2 colors + occupancy)
- [ ] All piece movement (sliders with magic bitboards, leapers with lookup)
- [ ] Legal move generation:
  - [ ] Pin detection
  - [ ] Check evasion (block, capture king attacker, move king)
  - [ ] Castling rights and legality checks
  - [ ] En passant (including discovery check edge cases)
- [ ] Make / unmake move with incremental updates
- [ ] Perft testing (position enumeration for correctness verification)

**Perft Targets**:
- `perft(5)` from startpos: 4,865,609 nodes
- `perft(6)` from startpos: 119,060,324 nodes

## v2: Search & Basic Evaluation

**Goal**: It plays real chess. It beats casual humans.

- [ ] Alpha-beta minimax search
- [ ] Quiescence search (capture-only to resolve horizon effect)
- [ ] Iterative deepening with time management
- [ ] Basic evaluation function:
  - [ ] Material balance
  - [ ] Piece-square tables (simplified, no tapering yet)
  - [ ] Mobility (optional)
- [ ] Principal variation reporting

## v3: Search Enhancements

**Goal**: Strong club player. ~2000 ELO.

- [ ] Transposition table (Zobrist hashing, replacement scheme)
- [ ] Null move pruning (with zugzwang checks)
- [ ] Late move reduction (LMR)
- [ ] Futility pruning / razoring
- [ ] Check extensions
- [ ] Opening book (Polyglot .bin format support)
- [ ] Tapered evaluation (opening → endgame interpolation)

## v4: Integration

**Goal**: Play the world.

- [ ] Full UCI protocol implementation
- [ ] Multi-PV mode
- [ ] Time control (classic, incremental, sudden death)
- [ ] Pondering support
- [ ] Lichess bot account integration (via lichess-bot bridge or direct API)
- [ ] Self-play tuning (SPSA / CLOP for eval weights)

## Architecture

```
+-------------+     +-----------+     +------------+     +---------+
|   UCI Loop  | <-> |  Search   | <-> |   MoveGen  | <-> | Bitboards |
|   (I/O)     |     | (AlphaBeta)|    | (Legal)    |     | (Board)   |
+-------------+     +-----------+     +------------+     +---------+
                           |
                           v
                    +-------------+
                    |   Eval      |
                    | (Material + |
                    |   PST +     |
                    |   Mobility) |
                    +-------------+
                           |
                           v
                    +-------------+
                    |  TT / Book  |
                    +-------------+
```

## Milestones

| Day | Deliverable | Acceptance Criteria |
|-----|-------------|-------------------|
| 1   | Board + MoveGen | Perft(5) matches known values |
| 3   | Search | Beats random mover 100% of the time |
| 7   | Playable | Wins against 1500 ELO human on Lichess (with manual moves) |
| 14  | UCI + Lichess | Plays autonomously via UCI in Arena or Lichess bot |

## Non-Goals

- Neural network evaluation (NNUE) — v4+ stretch goal only
- Tablebase endgame support — v4+ stretch
- Distributed computing / cluster search
- Own GUI (use existing UCI GUIs)
