#include "board.h"
#include <vector>
#include <algorithm>
#include <climits>

// Simple material values
const int MATERIAL[6] = { 100, 320, 330, 500, 900, 20000 };

// Piece-square tables (simplified, from white's perspective)
// Pawns
const int PST_PAWN[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

// Knights
const int PST_KNIGHT[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// Bishops
const int PST_BISHOP[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// Rooks
const int PST_ROOK[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

// Queens
const int PST_QUEEN[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// Kings (middle game)
const int PST_KING[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

const int* PST[6] = { PST_PAWN, PST_KNIGHT, PST_BISHOP, PST_ROOK, PST_QUEEN, PST_KING };

static inline int sq_flip(int s) { return s ^ 56; } // flip rank for black

int evaluate(const Board& board) {
    int score = 0;
    
    for (int pt = 0; pt < 6; pt++) {
        // White pieces
        U64 w = board.pieces[pt];
        while (w) {
            Square s = lsb(w);
            score += MATERIAL[pt] + PST[pt][s];
            poplsb(w);
        }
        // Black pieces
        U64 b = board.pieces[pt + 6];
        while (b) {
            Square s = lsb(b);
            score -= MATERIAL[pt] + PST[pt][sq_flip(s)];
            poplsb(b);
        }
    }
    
    return (board.side_to_move == WHITE) ? score : -score;
}

// Forward declaration
std::vector<Move> generate_legal_moves(const Board& board);

int quiescence(Board& board, int alpha, int beta, int depth);
int alpha_beta(Board& board, int depth, int alpha, int beta, bool allow_null);

int quiescence(Board& board, int alpha, int beta, int depth) {
    int stand_pat = evaluate(board);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;
    if (depth <= 0) return stand_pat;
    
    // In v1, simplified quiescence: just return stand_pat
    // Full quiescence requires capture-only move generation
    return stand_pat;
}

int alpha_beta(Board& board, int depth, int alpha, int beta, bool /*allow_null*/) {
    if (depth <= 0) {
        return quiescence(board, alpha, beta, 4); // quiescence depth 4
    }
    
    auto moves = generate_legal_moves(board);
    if (moves.empty()) {
        // Checkmate or stalemate
        if (board.in_check(board.side_to_move)) {
            return -30000 + (10 - depth); // checkmate, prefer faster
        }
        return 0; // stalemate
    }
    
    // Simple move ordering: captures first (MVV-LVA-ish)
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return (a.flags() & FLAG_CAPTURE) > (b.flags() & FLAG_CAPTURE);
    });
    
    int best_score = -INT_MAX;
    for (const Move& m : moves) {
        Board copy = board;
        // Apply move manually for search (since make_move doesn't support unmake)
        // This is a limitation of v1 - we'll do a full copy per node
        copy.make_move(m);
        
        int score = -alpha_beta(copy, depth - 1, -beta, -alpha, true);
        
        if (score > best_score) best_score = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // beta cutoff
    }
    
    return best_score;
}

Move search_best_move(Board& board, int depth) {
    auto moves = generate_legal_moves(board);
    if (moves.empty()) return Move();
    
    Move best_move = moves[0];
    int best_score = -INT_MAX;
    int alpha = -INT_MAX;
    int beta = INT_MAX;
    
    for (const Move& m : moves) {
        Board copy = board;
        copy.make_move(m);
        
        int score = -alpha_beta(copy, depth - 1, -beta, -alpha, true);
        
        if (score > best_score) {
            best_score = score;
            best_move = m;
        }
        if (score > alpha) alpha = score;
    }
    
    return best_move;
}

// Perft for testing correctness
unsigned long long perft(Board& board, int depth) {
    if (depth <= 0) return 1;
    
    auto moves = generate_legal_moves(board);
    if (depth == 1) return moves.size();
    
    unsigned long long nodes = 0;
    for (const Move& m : moves) {
        Board copy = board;
        copy.make_move(m);
        nodes += perft(copy, depth - 1);
    }
    return nodes;
}
