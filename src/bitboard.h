#pragma once
#include <cstdint>

using U64 = uint64_t;
using Square = int;

enum {
    A1=0, B1, C1, D1, E1, F1, G1, H1,
    A2=8, B2, C2, D2, E2, F2, G2, H2,
    A3=16, B3, C3, D3, E3, F3, G3, H3,
    A4=24, B4, C4, D4, E4, F4, G4, H4,
    A5=32, B5, C5, D5, E5, F5, G5, H5,
    A6=40, B6, C6, D6, E6, F6, G6, H6,
    A7=48, B7, C7, D7, E7, F7, G7, H7,
    A8=56, B8, C8, D8, E8, F8, G8, H8
};

inline int popcount(U64 bb) {
    return __builtin_popcountll(bb);
}

inline Square lsb(U64 bb) {
    return __builtin_ctzll(bb);
}

inline U64 poplsb(U64 &bb) {
    U64 lsb_bb = bb & -bb;
    bb ^= lsb_bb;
    return lsb_bb;
}

inline U64 shift_n(U64 bb)  { return bb << 8; }
inline U64 shift_s(U64 bb)  { return bb >> 8; }
inline U64 shift_e(U64 bb)  { return (bb << 1) & ~0x0101010101010101ULL; }
inline U64 shift_w(U64 bb)  { return (bb >> 1) & ~0x8080808080808080ULL; }
inline U64 shift_ne(U64 bb) { return (bb << 9) & ~0x0101010101010101ULL; }
inline U64 shift_nw(U64 bb) { return (bb << 7) & ~0x8080808080808080ULL; }
inline U64 shift_se(U64 bb) { return (bb >> 7) & ~0x0101010101010101ULL; }
inline U64 shift_sw(U64 bb) { return (bb >> 9) & ~0x8080808080808080ULL; }

inline U64 sq_bb(Square sq) { return 1ULL << sq; }
inline int rank_of(Square sq) { return sq >> 3; }
inline int file_of(Square sq) { return sq & 7; }
inline Square sq(int r, int f) { return (r << 3) | f; }

constexpr U64 RANK_1 = 0xFFULL;
constexpr U64 RANK_2 = 0xFF00ULL;
constexpr U64 RANK_3 = 0xFF0000ULL;
constexpr U64 RANK_4 = 0xFF000000ULL;
constexpr U64 RANK_5 = 0xFF00000000ULL;
constexpr U64 RANK_6 = 0xFF0000000000ULL;
constexpr U64 RANK_7 = 0xFF000000000000ULL;
constexpr U64 RANK_8 = 0xFF00000000000000ULL;

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_B = 0x0202020202020202ULL;
constexpr U64 FILE_C = 0x0404040404040404ULL;
constexpr U64 FILE_D = 0x0808080808080808ULL;
constexpr U64 FILE_E = 0x1010101010101010ULL;
constexpr U64 FILE_F = 0x2020202020202020ULL;
constexpr U64 FILE_G = 0x4040404040404040ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;
