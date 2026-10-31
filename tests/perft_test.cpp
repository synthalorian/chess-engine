#include "board.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

std::vector<Move> generate_legal_moves(const Board& board);

static uint64_t perft(const Board& board, int depth) {
    if (depth == 0) return 1;
    const auto moves = generate_legal_moves(board);
    if (depth == 1) return moves.size();
    uint64_t nodes = 0;
    for (Move move : moves) {
        Board child = board;
        child.make_move(move);
        nodes += perft(child, depth - 1);
    }
    return nodes;
}

static bool castling_tests() {
    bool passed = true;
    for (Color us : {WHITE, BLACK}) {
        // A rook attacks each king-path square in turn; attacking B is allowed.
        for (int file : {-1, 4, 5, 6, 3, 2, 1}) {
            Board board;
            const int home = us == WHITE ? 0 : 7;
            const int enemy_home = 7 - home;
            board.side_to_move = us;
            board.castling_rights = us == WHITE ? 3 : 12;
            board.set_piece(KING, us, sq(home, 4));
            board.set_piece(ROOK, us, sq(home, 0));
            board.set_piece(ROOK, us, sq(home, 7));
            board.set_piece(KING, ~us, sq(enemy_home, 7));
            if (file >= 0) board.set_piece(ROOK, ~us, sq(enemy_home, file));
            bool kingside = false, queenside = false;
            for (Move move : generate_legal_moves(board)) {
                kingside |= move.flags() == FLAG_CASTLE_K;
                queenside |= move.flags() == FLAG_CASTLE_Q;
            }
            const bool ok = kingside == (file != 4 && file != 5 && file != 6) &&
                            queenside == (file != 4 && file != 3 && file != 2);
            passed &= ok;
            if (!ok) std::cerr << "Castling regression: color " << us << " file " << file << '\n';
        }
        // Pawn attacks on D/F must also prevent castling, for either color.
        Board board;
        const int home = us == WHITE ? 0 : 7;
        board.side_to_move = us;
        board.castling_rights = us == WHITE ? 3 : 12;
        board.set_piece(KING, us, sq(home, 4));
        board.set_piece(ROOK, us, sq(home, 0));
        board.set_piece(ROOK, us, sq(home, 7));
        board.set_piece(KING, ~us, sq(7 - home, 7));
        board.set_piece(PAWN, ~us, sq(us == WHITE ? 1 : 6, 4));
        for (Move move : generate_legal_moves(board)) {
            if (move.flags() == FLAG_CASTLE_K || move.flags() == FLAG_CASTLE_Q) passed = false;
        }
    }
    std::cout << "Castling regressions " << (passed ? "PASS" : "FAIL") << std::endl;
    return passed;
}

int main(int argc, char** argv) {
    const bool slow = argc == 2 && std::string(argv[1]) == "--slow";
    if (argc > 1 && !slow) {
        std::cerr << "Usage: " << argv[0] << " [--slow]\n";
        return 2;
    }
    init_attack_tables();
    struct Position {
        const char* name;
        const char* fen;
        std::vector<uint64_t> expected;
    };
    const Position positions[] = {
        {"startpos", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -",
         {20, 400, 8902, 197281, 4865609, 119060324}},
        {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
         {48, 2039, 97862, 4085603, 193690690}}
    };
    bool passed = castling_tests();
    std::cout << "Position depth expected actual result time(ms)\n";
    for (const auto& position : positions) {
        Board board;
        board.parse_fen(position.fen);
        const int max_depth = static_cast<int>(position.expected.size());
        for (int depth = slow ? max_depth : 1; depth <= (slow ? max_depth : max_depth - 1); ++depth) {
            const auto start = std::chrono::steady_clock::now();
            const auto actual = perft(board, depth);
            const auto ms = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count();
            const auto expected = position.expected[depth - 1];
            const bool ok = actual == expected;
            passed &= ok;
            std::cout << position.name << ' ' << depth << ' ' << expected << ' '
                      << actual << ' ' << (ok ? "PASS" : "FAIL") << ' ' << ms << std::endl;
            if (!ok) {
                std::cerr << "Divide for " << position.name << " depth " << depth << ":\n";
                for (Move move : generate_legal_moves(board)) {
                    Board child = board;
                    child.make_move(move);
                    std::cerr << move.uci() << ' ' << perft(child, depth - 1) << '\n';
                }
                break;
            }
        }
    }
    return passed ? 0 : 1;
}
