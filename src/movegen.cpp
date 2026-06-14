#include "board.h"
#include <vector>

static void add_move(std::vector<Move>& moves, Square from, Square to, int flags = 0) {
    moves.emplace_back(from, to, flags);
}

static void generate_pawn_moves(const Board& board, std::vector<Move>& moves, Color us) {
    U64 pawns = board.pieces_of(us, PAWN);
    Color them = ~us;
    U64 enemy = board.occupied[them];
    U64 empty = ~board.all_occupied;
    int push_dir = (us == WHITE) ? 8 : -8;
    int start_rank = (us == WHITE) ? 1 : 6;
    int promo_rank = (us == WHITE) ? 7 : 0;
    
    while (pawns) {
        Square s = lsb(pawns);
        int f = file_of(s), r = rank_of(s);
        
        // Single push
        int to = s + push_dir;
        if (r + (us == WHITE ? 1 : -1) >= 0 && r + (us == WHITE ? 1 : -1) < 8 && (sq_bb(to) & empty)) {
            if (rank_of(to) == promo_rank) {
                for (int promo = KNIGHT; promo <= QUEEN; promo++)
                    add_move(moves, s, to, FLAG_PROMOTION + (promo - KNIGHT));
            } else {
                add_move(moves, s, to);
                // Double push from start rank
                if (r == start_rank) {
                    int to2 = s + 2 * push_dir;
                    if (sq_bb(to2) & empty) add_move(moves, s, to2, FLAG_DOUBLE_PAWN);
                }
            }
        }
        
        // Captures
        int captures[2] = { (us == WHITE) ? 7 : -9, (us == WHITE) ? 9 : -7 };
        for (int i = 0; i < 2; i++) {
            int to_c = s + captures[i];
            if (to_c < 0 || to_c > 63) continue;
            int tf = file_of(to_c);
            if (abs(tf - f) != 1) continue; // wrap-around check
            
            if (sq_bb(to_c) & enemy) {
                if (rank_of(to_c) == promo_rank) {
                    for (int promo = KNIGHT; promo <= QUEEN; promo++)
                        add_move(moves, s, to_c, FLAG_PROMOTION + (promo - KNIGHT) + FLAG_CAPTURE);
                } else {
                    add_move(moves, s, to_c, FLAG_CAPTURE);
                }
            }
            
            // En passant
            if (to_c == board.ep_square) {
                add_move(moves, s, to_c, FLAG_ENPASSANT);
            }
        }
        
        poplsb(pawns);
    }
}

static void generate_knight_moves(const Board& board, std::vector<Move>& moves, Color us) {
    U64 knights = board.pieces_of(us, KNIGHT);
    U64 own = board.occupied[us];
    
    while (knights) {
        Square s = lsb(knights);
        U64 attacks = knight_attacks[s] & ~own;
        while (attacks) {
            Square t = lsb(attacks);
            int flags = (board.color_at(t) != NO_COLOR) ? FLAG_CAPTURE : 0;
            add_move(moves, s, t, flags);
            poplsb(attacks);
        }
        poplsb(knights);
    }
}

static void generate_king_moves(const Board& board, std::vector<Move>& moves, Color us) {
    Square s = lsb(board.pieces_of(us, KING));
    if (s < 0) return;
    
    U64 own = board.occupied[us];
    U64 attacks = king_attacks[s] & ~own;
    while (attacks) {
        Square t = lsb(attacks);
        int flags = (board.color_at(t) != NO_COLOR) ? FLAG_CAPTURE : 0;
        add_move(moves, s, t, flags);
        poplsb(attacks);
    }
    
    // Castling
    if (us == WHITE) {
        if (board.can_castle_kingside(WHITE)) {
            if ((board.all_occupied & (sq_bb(F1) | sq_bb(G1))) == 0)
                add_move(moves, E1, G1, FLAG_CASTLE_K);
        }
        if (board.can_castle_queenside(WHITE)) {
            if ((board.all_occupied & (sq_bb(B1) | sq_bb(C1) | sq_bb(D1))) == 0)
                add_move(moves, E1, C1, FLAG_CASTLE_Q);
        }
    } else {
        if (board.can_castle_kingside(BLACK)) {
            if ((board.all_occupied & (sq_bb(F8) | sq_bb(G8))) == 0)
                add_move(moves, E8, G8, FLAG_CASTLE_K);
        }
        if (board.can_castle_queenside(BLACK)) {
            if ((board.all_occupied & (sq_bb(B8) | sq_bb(C8) | sq_bb(D8))) == 0)
                add_move(moves, E8, C8, FLAG_CASTLE_Q);
        }
    }
}

static void generate_sliding_moves(const Board& board, std::vector<Move>& moves, Color us, PieceType pt) {
    U64 pieces_bb = board.pieces_of(us, pt);
    U64 own = board.occupied[us];
    static const int ROOK_DIRS[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
    static const int BISHOP_DIRS[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};
    const int (*dirs)[2] = (pt == ROOK) ? ROOK_DIRS : BISHOP_DIRS;
    
    while (pieces_bb) {
        Square s = lsb(pieces_bb);
        int f = file_of(s), r = rank_of(s);
        
        for (int d = 0; d < 4; d++) {
            int nf = f + dirs[d][0];
            int nr = r + dirs[d][1];
            while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                int t = sq(nr, nf);
                U64 t_bb = sq_bb(t);
                
                if (t_bb & own) break; // blocked by own piece
                
                int flags = (t_bb & board.all_occupied) ? FLAG_CAPTURE : 0;
                add_move(moves, s, t, flags);
                
                if (t_bb & board.all_occupied) break; // blocked by enemy piece
                
                nf += dirs[d][0];
                nr += dirs[d][1];
            }
        }
        
        poplsb(pieces_bb);
    }
}

std::vector<Move> generate_legal_moves(const Board& board) {
    std::vector<Move> pseudo;
    Color us = board.side_to_move;
    
    generate_pawn_moves(board, pseudo, us);
    generate_knight_moves(board, pseudo, us);
    generate_king_moves(board, pseudo, us);
    generate_sliding_moves(board, pseudo, us, BISHOP);
    generate_sliding_moves(board, pseudo, us, ROOK);
    generate_sliding_moves(board, pseudo, us, QUEEN);
    
    // Filter legal moves: check if king is safe after each pseudo-legal move
    std::vector<Move> legal;
    for (const Move& m : pseudo) {
        Board copy = board;
        copy.make_move(m);
        if (!copy.in_check(board.side_to_move)) {
            legal.push_back(m);
        }
    }
    
    return legal;
}
