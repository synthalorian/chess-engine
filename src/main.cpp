#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include "board.h"

// Forward declarations
std::vector<Move> generate_legal_moves(const Board& board);
Move search_best_move(Board& board, int depth);
unsigned long long perft(Board& board, int depth);
int evaluate(const Board& board);

class UCIEngine {
    Board board;
    std::atomic<bool> searching{false};
    std::atomic<bool> stop_search{false};
    std::thread search_thread;
    
public:
    UCIEngine() { init_attack_tables(); }
    
    void loop() {
        std::string line;
        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            
            if (cmd == "uci") {
                std::cout << "id name synthclaw-chess v1.0\n";
                std::cout << "id author synthclaw\n";
                std::cout << "uciok\n";
            } else if (cmd == "isready") {
                std::cout << "readyok\n";
            } else if (cmd == "quit") {
                stop();
                return;
            } else if (cmd == "position") {
                handle_position(iss);
            } else if (cmd == "go") {
                handle_go(iss);
            } else if (cmd == "stop") {
                stop_search = true;
                if (search_thread.joinable()) search_thread.join();
            } else if (cmd == "d") {
                print_board();
            } else if (cmd == "eval") {
                std::cout << "eval: " << evaluate(board) << "\n";
            } else if (cmd == "perft") {
                int depth;
                iss >> depth;
                auto start = std::chrono::steady_clock::now();
                unsigned long long nodes = perft(board, depth);
                auto end = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << "perft(" << depth << ") = " << nodes;
                if (ms > 0) std::cout << "  (" << (nodes / ms) << " nps)";
                std::cout << "\n";
            } else if (cmd == "moves") {
                auto moves = generate_legal_moves(board);
                std::cout << "legal moves (" << moves.size() << "):\n";
                for (const Move& m : moves) {
                    std::cout << "  " << m.uci() << "\n";
                }
            }
            std::cout.flush();
            
            // Wait for any detached search thread to finish before next command
            if (searching && search_thread.joinable()) {
                search_thread.join();
            }
        }
    }
    
private:
    void handle_position(std::istringstream& iss) {
        std::string token;
        iss >> token;
        
        if (token == "startpos") {
            board.parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } else if (token == "fen") {
            std::string fen;
            std::string part;
            for (int i = 0; i < 6; i++) {
                iss >> part;
                if (i > 0) fen += " ";
                fen += part;
            }
            board.parse_fen(fen);
        }
        
        // Check for "moves" section
        iss >> token;
        if (token == "moves") {
            while (iss >> token) {
                // Parse and apply UCI move string (e.g., "e2e4", "e1g1" for O-O)
                if (token.length() < 4) continue;
                int from_f = token[0] - 'a';
                int from_r = token[1] - '1';
                int to_f = token[2] - 'a';
                int to_r = token[3] - '1';
                int from_sq = from_r * 8 + from_f;
                int to_sq = to_r * 8 + to_f;
                
                int flags = 0;
                if (token.length() > 4) {
                    char promo = token[4];
                    flags = FLAG_PROMOTION;
                    if (promo == 'n') flags += 0;
                    else if (promo == 'b') flags += 1;
                    else if (promo == 'r') flags += 2;
                    else if (promo == 'q') flags += 3;
                }
                
                // Detect special moves
                PieceType pt = board.piece_type_at(from_sq);
                if (pt == KING && abs(to_f - from_f) == 2) {
                    flags = (to_f > from_f) ? FLAG_CASTLE_K : FLAG_CASTLE_Q;
                } else if (pt == PAWN && to_sq == board.ep_square) {
                    flags = FLAG_ENPASSANT;
                } else if (board.color_at(to_sq) != NO_COLOR) {
                    flags |= FLAG_CAPTURE;
                } else if (pt == PAWN && abs(to_r - from_r) == 2) {
                    flags = FLAG_DOUBLE_PAWN;
                }
                
                board.make_move(Move(from_sq, to_sq, flags));
            }
        }
    }
    
    void handle_go(std::istringstream& iss) {
        stop_search = false;
        
        std::string token;
        int depth = 0; // 0 means no depth limit (infinite)
        int wtime = 0, btime = 0, winc = 0, binc = 0, movetime = 0;
        bool depth_set = false;
        
        while (iss >> token) {
            if (token == "depth") { iss >> depth; depth_set = true; }
            else if (token == "wtime") iss >> wtime;
            else if (token == "btime") iss >> btime;
            else if (token == "winc") iss >> winc;
            else if (token == "binc") iss >> binc;
            else if (token == "movetime") iss >> movetime;
        }
        
        // Default depth if nothing specified
        if (!depth_set && movetime == 0) {
            depth = 4; // default depth
            depth_set = true;
        }
        
        // Simple time management
        if (movetime > 0) {
            // Use movetime directly
        } else if (board.side_to_move == WHITE && wtime > 0) {
            movetime = wtime / 30 + winc; // spend ~1/30 of time + increment
            if (movetime > wtime / 3) movetime = wtime / 3; // safety cap
        } else if (board.side_to_move == BLACK && btime > 0) {
            movetime = btime / 30 + binc;
            if (movetime > btime / 3) movetime = btime / 3;
        }
        
        if (movetime > 0 && movetime < 100) movetime = 100; // minimum 100ms
        
        searching = true;
        search_thread = std::thread([this, depth, movetime]() {
            auto start = std::chrono::steady_clock::now();
            Move best;
            
            if (depth > 0) {
                best = search_best_move(board, depth);
            } else {
                // Infinite search - just do a reasonable depth
                best = search_best_move(board, 6);
            }
            
            // Check if we should stop (time limit)
            if (movetime > 0) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                if (elapsed > movetime && depth > 1) {
                    // Redo with lower depth if we ran out of time
                    best = search_best_move(board, depth - 1);
                }
            }
            
            if (!stop_search) {
                std::cout << "bestmove " << best.uci() << "\n";
                std::cout.flush();
            }
            searching = false;
        });
        
        if (depth_set || movetime > 0) {
            search_thread.join(); // Wait for finite searches
        } else {
            search_thread.detach(); // Detach for infinite searches
        }
    }
    
    void print_board() {
        std::cout << "\n";
        for (int r = 7; r >= 0; r--) {
            std::cout << (r + 1) << " ";
            for (int f = 0; f < 8; f++) {
                int s = r * 8 + f;
                PieceType pt = board.piece_type_at(s);
                Color c = board.color_at(s);
                if (pt == NO_PIECE) {
                    std::cout << ((f + r) % 2 == 0 ? ". " : "  ");
                } else {
                    char ch = pt_to_char(pt, c);
                    std::cout << ch << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "  a b c d e f g h\n";
        std::cout << "Side: " << (board.side_to_move == WHITE ? "white" : "black") << "\n";
        std::cout << "Castling: " << board.can_castle_kingside(WHITE) << board.can_castle_queenside(WHITE)
                  << board.can_castle_kingside(BLACK) << board.can_castle_queenside(BLACK) << "\n";
        std::cout << "EP: " << board.ep_square << "\n";
        std::cout << "Eval: " << evaluate(board) << "\n\n";
    }
    
    void stop() {
        stop_search = true;
        if (search_thread.joinable()) search_thread.join();
    }
};

int main() {
    UCIEngine engine;
    engine.loop();
    return 0;
}
