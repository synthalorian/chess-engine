#pragma once
#include "bitboard.h"
#include <string>

enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE };
enum Color { WHITE, BLACK, NO_COLOR };

inline Color operator~(Color c) { return c == WHITE ? BLACK : WHITE; }

inline PieceType pt_from_char(char c) {
    switch (c) {
        case 'P': case 'p': return PAWN;
        case 'N': case 'n': return KNIGHT;
        case 'B': case 'b': return BISHOP;
        case 'R': case 'r': return ROOK;
        case 'Q': case 'q': return QUEEN;
        case 'K': case 'k': return KING;
        default: return NO_PIECE;
    }
}

inline char pt_to_char(PieceType pt, Color c) {
    static const char chars[] = "PNBRQKpnbrqk";
    return chars[pt + (c == BLACK ? 6 : 0)];
}

// Move encoding: 16-bit
// bits 0-5: from square
// bits 6-11: to square
// bits 12-15: flags

struct Move {
    uint16_t data;
    
    Move() : data(0) {}
    Move(Square from, Square to, int flags = 0) : data(from | (to << 6) | (flags << 12)) {}

    Square from() const { return data & 0x3F; }
    Square to() const   { return (data >> 6) & 0x3F; }
    int flags() const   { return data >> 12; }

    bool operator==(const Move& o) const { return data == o.data; }
    bool operator!=(const Move& o) const { return data != o.data; }

    std::string uci() const;
};

enum MoveFlag {
    FLAG_QUIET = 0,
    FLAG_DOUBLE_PAWN = 1,
    FLAG_CASTLE_K = 2,
    FLAG_CASTLE_Q = 3,
    FLAG_CAPTURE = 4,
    FLAG_ENPASSANT = 5,
    FLAG_PROMOTION = 8,
    FLAG_PROMO_CAPTURE = 12
};

inline int promo_flag(PieceType pt) {
    if (pt == KNIGHT) return 0;
    if (pt == BISHOP) return 1;
    if (pt == ROOK)   return 2;
    return 3; // QUEEN
}

inline PieceType promo_piece(int flag) {
    static const PieceType pieces[] = { KNIGHT, BISHOP, ROOK, QUEEN };
    return pieces[flag & 3];
}

inline std::string Move::uci() const {
    std::string s;
    static const char files[] = "abcdefgh";
    static const char ranks[] = "12345678";
    s += files[file_of(from())];
    s += ranks[rank_of(from())];
    s += files[file_of(to())];
    s += ranks[rank_of(to())];
    int f = flags();
    if (f & FLAG_PROMOTION) {
        PieceType pt = promo_piece(f);
        s += "nbrq"[pt - KNIGHT];
    }
    return s;
}
