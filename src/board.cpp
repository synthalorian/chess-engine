#include "board.h"
#include <cstring>

// Attack tables
U64 knight_attacks[64];
U64 king_attacks[64];
U64 rook_attacks[64][64];
U64 bishop_attacks[64][64];
U64 rook_rays[64][4];
U64 bishop_rays[64][4];

static const int KNIGHT_DIRS[8][2] = {
    {-2,-1}, {-2,1}, {-1,-2}, {-1,2},
    {1,-2}, {1,2}, {2,-1}, {2,1}
};

static const int KING_DIRS[8][2] = {
    {-1,-1}, {-1,0}, {-1,1}, {0,-1},
    {0,1}, {1,-1}, {1,0}, {1,1}
};

static const int ROOK_DIRS[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
static const int BISHOP_DIRS[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};

void init_attack_tables() {
    auto on_board = [](int f, int r) -> bool {
        return f >= 0 && f < 8 && r >= 0 && r < 8;
    };
    // Knight
    for (int s = 0; s < 64; s++) {
        int f = file_of(s), r = rank_of(s);
        U64 bb = 0;
        for (int d = 0; d < 8; d++) {
            int nf = f + KNIGHT_DIRS[d][0];
            int nr = r + KNIGHT_DIRS[d][1];
            if (on_board(nf, nr)) bb |= sq_bb(sq(nr, nf));
        }
        knight_attacks[s] = bb;
    }

    // King
    for (int s = 0; s < 64; s++) {
        int f = file_of(s), r = rank_of(s);
        U64 bb = 0;
        for (int d = 0; d < 8; d++) {
            int nf = f + KING_DIRS[d][0];
            int nr = r + KING_DIRS[d][1];
            if (on_board(nf, nr)) bb |= sq_bb(sq(nr, nf));
        }
        king_attacks[s] = bb;
    }

    // Rook rays and attacks
    for (int s = 0; s < 64; s++) {
        int f = file_of(s), r = rank_of(s);
        for (int d = 0; d < 4; d++) {
            U64 ray = 0;
            int nf = f + ROOK_DIRS[d][0];
            int nr = r + ROOK_DIRS[d][1];
            while (on_board(nf, nr)) {
                ray |= sq_bb(sq(nr, nf));
                nf += ROOK_DIRS[d][0];
                nr += ROOK_DIRS[d][1];
            }
            rook_rays[s][d] = ray;
        }
        // Precompute rook attacks between any two squares
        for (int t = 0; t < 64; t++) {
            if (s == t) { rook_attacks[s][t] = 0; continue; }
            int tf = file_of(t), tr = rank_of(t);
            U64 bb = 0;
            if (f == tf) {
                int step = (tr > r) ? 1 : -1;
                for (int nr = r + step; nr != tr; nr += step)
                    bb |= sq_bb(sq(nr, f));
            } else if (r == tr) {
                int step = (tf > f) ? 1 : -1;
                for (int nf = f + step; nf != tf; nf += step)
                    bb |= sq_bb(sq(r, nf));
            }
            rook_attacks[s][t] = bb;
        }
    }

    // Bishop rays and attacks
    for (int s = 0; s < 64; s++) {
        int f = file_of(s), r = rank_of(s);
        for (int d = 0; d < 4; d++) {
            U64 ray = 0;
            int nf = f + BISHOP_DIRS[d][0];
            int nr = r + BISHOP_DIRS[d][1];
            while (on_board(nf, nr)) {
                ray |= sq_bb(sq(nr, nf));
                nf += BISHOP_DIRS[d][0];
                nr += BISHOP_DIRS[d][1];
            }
            bishop_rays[s][d] = ray;
        }
        for (int t = 0; t < 64; t++) {
            if (s == t) { bishop_attacks[s][t] = 0; continue; }
            int tf = file_of(t), tr = rank_of(t);
            U64 bb = 0;
            if (abs(tf - f) == abs(tr - r) && abs(tf - f) > 0) {
                int fstep = (tf > f) ? 1 : -1;
                int rstep = (tr > r) ? 1 : -1;
                int nf = f + fstep, nr = r + rstep;
                while (nf != tf && nr != tr) {
                    bb |= sq_bb(sq(nr, nf));
                    nf += fstep; nr += rstep;
                }
            }
            bishop_attacks[s][t] = bb;
        }
    }
}

// Board implementation
Board::Board() { clear(); }

void Board::clear() {
    pieces.fill(0);
    occupied[0] = occupied[1] = 0;
    all_occupied = 0;
    side_to_move = WHITE;
    castling_rights = 0;
    ep_square = -1;
    halfmove_clock = 0;
    fullmove_number = 1;
}

void Board::set_piece(PieceType pt, Color c, Square sq) {
    U64 bb = sq_bb(sq);
    int idx = pt + (c == BLACK ? 6 : 0);
    pieces[idx] |= bb;
    occupied[c] |= bb;
    all_occupied |= bb;
}

void Board::remove_piece(Square sq) {
    U64 bb = ~sq_bb(sq);
    for (int i = 0; i < 12; i++) pieces[i] &= bb;
    occupied[0] &= bb;
    occupied[1] &= bb;
    all_occupied &= bb;
}

PieceType Board::piece_type_at(Square sq) const {
    U64 bb = sq_bb(sq);
    for (int pt = 0; pt < 6; pt++) {
        if (pieces[pt] & bb) return (PieceType)pt;
        if (pieces[pt + 6] & bb) return (PieceType)pt;
    }
    return NO_PIECE;
}

Color Board::color_at(Square sq) const {
    U64 bb = sq_bb(sq);
    if (occupied[WHITE] & bb) return WHITE;
    if (occupied[BLACK] & bb) return BLACK;
    return NO_COLOR;
}

void Board::parse_fen(const std::string& fen) {
    clear();
    size_t i = 0;
    int rank = 7, file = 0;
    
    while (i < fen.size() && fen[i] != ' ') {
        char c = fen[i++];
        if (c == '/') {
            rank--; file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            PieceType pt = pt_from_char(c);
            Color col = (c >= 'a' && c <= 'z') ? BLACK : WHITE;
            if (pt != NO_PIECE) {
                set_piece(pt, col, sq(rank, file));
            }
            file++;
        }
    }
    
    if (i < fen.size() && fen[i] == ' ') i++;
    if (i < fen.size()) side_to_move = (fen[i] == 'w') ? WHITE : BLACK;
    i += 2;
    
    castling_rights = 0;
    while (i < fen.size() && fen[i] != ' ') {
        switch (fen[i++]) {
            case 'K': castling_rights |= 1; break;
            case 'Q': castling_rights |= 2; break;
            case 'k': castling_rights |= 4; break;
            case 'q': castling_rights |= 8; break;
        }
    }
    i++;
    
    if (i < fen.size() && fen[i] != '-') {
        int f = fen[i++] - 'a';
        int r = fen[i++] - '1';
        ep_square = sq(r, f);
    }
}

bool Board::in_check(Color c) const {
    // Find king
    Square king_sq = -1;
    U64 king_bb = pieces[KING + (c == BLACK ? 6 : 0)];
    if (king_bb) king_sq = lsb(king_bb);
    if (king_sq < 0) return false;
    
    Color enemy = ~c;
    
    // Knight attacks
    if (knight_attacks[king_sq] & pieces[KNIGHT + (enemy == BLACK ? 6 : 0)]) return true;
    
    // Pawn attacks
    U64 pawns = pieces[PAWN + (enemy == BLACK ? 6 : 0)];
    if (c == WHITE) {
        U64 attackers = (shift_se(pawns) | shift_sw(pawns)) & sq_bb(king_sq);
        if (attackers) return true;
    } else {
        U64 attackers = (shift_ne(pawns) | shift_nw(pawns)) & sq_bb(king_sq);
        if (attackers) return true;
    }
    
    // King attacks
    if (king_attacks[king_sq] & pieces[KING + (enemy == BLACK ? 6 : 0)]) return true;
    
    // Rook/Queen attacks - check same rank and file
    U64 rook_queen = pieces[ROOK + (enemy == BLACK ? 6 : 0)] | pieces[QUEEN + (enemy == BLACK ? 6 : 0)];
    int kf = file_of(king_sq), kr = rank_of(king_sq);
    for (int d = 0; d < 4; d++) {
        int nf = kf + ROOK_DIRS[d][0];
        int nr = kr + ROOK_DIRS[d][1];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            int t = sq(nr, nf);
            U64 t_bb = sq_bb(t);
            if (t_bb & all_occupied) {
                if (t_bb & rook_queen) return true;
                break; // blocked
            }
            nf += ROOK_DIRS[d][0];
            nr += ROOK_DIRS[d][1];
        }
    }
    
    // Bishop/Queen attacks - check diagonals
    U64 bishop_queen = pieces[BISHOP + (enemy == BLACK ? 6 : 0)] | pieces[QUEEN + (enemy == BLACK ? 6 : 0)];
    for (int d = 0; d < 4; d++) {
        int nf = kf + BISHOP_DIRS[d][0];
        int nr = kr + BISHOP_DIRS[d][1];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            int t = sq(nr, nf);
            U64 t_bb = sq_bb(t);
            if (t_bb & all_occupied) {
                if (t_bb & bishop_queen) return true;
                break; // blocked
            }
            nf += BISHOP_DIRS[d][0];
            nr += BISHOP_DIRS[d][1];
        }
    }
    
    return false;
}

U64 Board::attackers_to(Square sq, Color by) const {
    U64 occ = all_occupied;
    U64 attackers = 0;
    
    // Pawns
    U64 pawns = pieces[PAWN + (by == BLACK ? 6 : 0)];
    if (by == WHITE) {
        attackers |= ((shift_se(pawns) | shift_sw(pawns)) & sq_bb(sq));
    } else {
        attackers |= ((shift_ne(pawns) | shift_nw(pawns)) & sq_bb(sq));
    }
    
    // Knights
    attackers |= (knight_attacks[sq] & pieces[KNIGHT + (by == BLACK ? 6 : 0)]);
    
    // King
    attackers |= (king_attacks[sq] & pieces[KING + (by == BLACK ? 6 : 0)]);
    
    // Rooks/Queens
    U64 rook_queen = pieces[ROOK + (by == BLACK ? 6 : 0)] | pieces[QUEEN + (by == BLACK ? 6 : 0)];
    for (int d = 0; d < 4; d++) {
        U64 ray = rook_rays[sq][d];
        U64 bq_on_ray = ray & rook_queen;
        while (bq_on_ray) {
            Square s = lsb(bq_on_ray);
            U64 between = rook_attacks[sq][s];
            if ((between & occ) == 0) attackers |= sq_bb(s);
            poplsb(bq_on_ray);
        }
    }
    
    // Bishops/Queens
    U64 bishop_queen = pieces[BISHOP + (by == BLACK ? 6 : 0)] | pieces[QUEEN + (by == BLACK ? 6 : 0)];
    for (int d = 0; d < 4; d++) {
        U64 ray = bishop_rays[sq][d];
        U64 bq_on_ray = ray & bishop_queen;
        while (bq_on_ray) {
            Square s = lsb(bq_on_ray);
            U64 between = bishop_attacks[sq][s];
            if ((between & occ) == 0) attackers |= sq_bb(s);
            poplsb(bq_on_ray);
        }
    }
    
    return attackers;
}

bool Board::can_castle_kingside(Color c) const {
    if (c == WHITE) return (castling_rights & 1) != 0;
    return (castling_rights & 4) != 0;
}

bool Board::can_castle_queenside(Color c) const {
    if (c == WHITE) return (castling_rights & 2) != 0;
    return (castling_rights & 8) != 0;
}

void Board::make_move(Move m) {
    Square from = m.from();
    Square to = m.to();
    int flags = m.flags();
    Color us = side_to_move;
    Color them = ~us;
    
    PieceType pt = piece_type_at(from);
    PieceType captured = piece_type_at(to);
    
    // Remove from source
    U64 from_bb = sq_bb(from);
    U64 to_bb = sq_bb(to);
    int pt_idx = pt + (us == BLACK ? 6 : 0);
    pieces[pt_idx] &= ~from_bb;
    occupied[us] &= ~from_bb;
    
    // Remove captured piece
    if (captured != NO_PIECE) {
        int cap_idx = captured + (them == BLACK ? 6 : 0);
        pieces[cap_idx] &= ~to_bb;
        occupied[them] &= ~to_bb;
    }
    
    // Handle special moves
    if (flags == FLAG_ENPASSANT) {
        Square ep_cap = (us == WHITE) ? to - 8 : to + 8;
        U64 ep_bb = sq_bb(ep_cap);
        pieces[PAWN + (them == BLACK ? 6 : 0)] &= ~ep_bb;
        occupied[them] &= ~ep_bb;
    }
    
    // Place piece on destination
    if (flags & FLAG_PROMOTION) {
        PieceType promo = promo_piece(flags);
        pieces[pt_idx] &= ~to_bb; // remove pawn
        pieces[promo + (us == BLACK ? 6 : 0)] |= to_bb;
    } else {
        pieces[pt_idx] |= to_bb;
    }
    occupied[us] |= to_bb;
    
    // Castling: move rook
    if (flags == FLAG_CASTLE_K) {
        if (us == WHITE) {
            pieces[ROOK] &= ~sq_bb(H1); pieces[ROOK] |= sq_bb(F1);
            occupied[WHITE] &= ~sq_bb(H1); occupied[WHITE] |= sq_bb(F1);
        } else {
            pieces[ROOK + 6] &= ~sq_bb(H8); pieces[ROOK + 6] |= sq_bb(F8);
            occupied[BLACK] &= ~sq_bb(H8); occupied[BLACK] |= sq_bb(F8);
        }
    } else if (flags == FLAG_CASTLE_Q) {
        if (us == WHITE) {
            pieces[ROOK] &= ~sq_bb(A1); pieces[ROOK] |= sq_bb(D1);
            occupied[WHITE] &= ~sq_bb(A1); occupied[WHITE] |= sq_bb(D1);
        } else {
            pieces[ROOK + 6] &= ~sq_bb(A8); pieces[ROOK + 6] |= sq_bb(D8);
            occupied[BLACK] &= ~sq_bb(A8); occupied[BLACK] |= sq_bb(D8);
        }
    }
    
    // Update castling rights
    if (pt == KING) {
        if (us == WHITE) castling_rights &= ~3;
        else castling_rights &= ~12;
    }
    if (pt == ROOK) {
        if (from == A1) castling_rights &= ~2;
        else if (from == H1) castling_rights &= ~1;
        else if (from == A8) castling_rights &= ~8;
        else if (from == H8) castling_rights &= ~4;
    }
    if (captured == ROOK) {
        if (to == A1) castling_rights &= ~2;
        else if (to == H1) castling_rights &= ~1;
        else if (to == A8) castling_rights &= ~8;
        else if (to == H8) castling_rights &= ~4;
    }
    
    // Update ep square
    if (pt == PAWN && abs(to - from) == 16) {
        ep_square = (from + to) / 2;
    } else {
        ep_square = -1;
    }
    
    all_occupied = occupied[WHITE] | occupied[BLACK];
    side_to_move = them;
    halfmove_clock++;
    if (us == BLACK) fullmove_number++;
}

void Board::unmake_move(Move m, PieceType captured, Square old_ep, int old_castling, int old_halfmove) {
    // Simpler to just not support unmake for v1, or store more state
    // For v1, we'll just not use unmake in search (no iterative deepening)
    // This is a placeholder for completeness
    (void)m; (void)captured; (void)old_ep; (void)old_castling; (void)old_halfmove;
}
