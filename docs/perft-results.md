# Perft results

Measured on 2026-10-31 (Linux build box, `g++ -O3` (repo CXXFLAGS), single thread, `nice -n 19`). Run with `make test` (fast) and `make test-slow`.

| Position | Depth | Expected | Actual | Result | Time (ms) |
|---|---|---|---|---|---|
| startpos | 1 | 20 | 20 | PASS | <1 |
| startpos | 2 | 400 | 400 | PASS | <1 |
| startpos | 3 | 8902 | 8902 | PASS | 0.4 |
| startpos | 4 | 197281 | 197281 | PASS | 9 |
| startpos | 5 | 4865609 | 4865609 | PASS | 204 |
| startpos | 6 (slow) | 119060324 | 119060324 | PASS | 4422 |
| Kiwipete | 1 | 48 | 48 | PASS | <1 |
| Kiwipete | 2 | 2039 | 2039 | PASS | <1 |
| Kiwipete | 3 | 97862 | 97862 | PASS | 3.4 |
| Kiwipete | 4 | 4085603 | 4085603 | PASS | 168 |
| Kiwipete | 5 (slow) | 193690690 | 193690690 | PASS | 7065 |

Kiwipete FEN: `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -`
