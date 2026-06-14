#pragma once
#include "bitboard.h"
#include "move.h"
#include <array>
#include <string>

struct Board {
    // 6 piece types x 2 colors = 12 bitboards
    std::array<U64, 12> pieces; // 0-5: white, 6-11: black
    U64 occupied[2]; // by color
    U64 all_occupied;
    Color side_to_move;
    int castling_rights; // 1=K, 2=Q, 4=k, 8=q
    Square ep_square;    // -1 if none
    int halfmove_clock;
    int fullmove_number;

    Board();
    void clear();
    void set_piece(PieceType pt, Color c, Square sq);
    void remove_piece(Square sq);
    void parse_fen(const std::string& fen);

    U64 pieces_of(Color c, PieceType pt) const { return pieces[pt + (c == BLACK ? 6 : 0)]; }
    U64 color_occupancy(Color c) const { return occupied[c]; }
    U64 piece_at(Square sq) const { return all_occupied & sq_bb(sq); }
    PieceType piece_type_at(Square sq) const;
    Color color_at(Square sq) const;

    void make_move(Move m);
    void unmake_move(Move m, PieceType captured, Square old_ep, int old_castling, int old_halfmove);

    bool in_check(Color c) const;
    U64 attackers_to(Square sq, Color by) const;

    bool can_castle_kingside(Color c) const;
    bool can_castle_queenside(Color c) const;
};

extern U64 knight_attacks[64];
extern U64 king_attacks[64];
extern U64 rook_attacks[64][64];  // [from][to] - precomputed ray masks (not full magic, just rays)
extern U64 bishop_attacks[64][64]; // same
extern U64 rook_rays[64][4];       // N, S, E, W ray masks from each square
extern U64 bishop_rays[64][4];     // NE, NW, SE, SW ray masks

void init_attack_tables();
