/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   defs.h
 * Author: rahul
 *
 * Created on January 29, 2019, 7:58 PM
 */

#ifndef DEFS_H
#define DEFS_H

typedef unsigned long long U64; // Custom type foe the pieces

#define NAME "Bad Bishop 1.0"
#define BRD_SQ_NUM 120

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK }; // Board pieces for black and white
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE }; // Names of files along the board
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE }; // Names of ranks along the board
enum { WHITE, BLACK, BOTH}; // Player name

// Board structure (A-H: Files and 1-8: Ranks)
enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2, 
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

enum { FALSE, TRUE};

typedef struct{
    int pieces[BRD_SQ_NUM]; //each piece is a 64 bit number representing 64 squares
    U64 pawns[3]; // index0 for white, index1 for black and index2 for both
    
    int KingSq[2]; // Square of the king
    
    int side; // Which side to move
    int enPas; // en-passant square
    int fiftyMove; // counter for the 50 move rule
    
    int ply; // number of half moves
    int hisPly; // number of plies made
    
    U64 posKey; // unique 64-bit key for the board position at a time
    
    int pceNum[13]; // how many pieces are on the board. Corresponding to the 'pieces'enum
    int big[3]; // how many non-pawn pieces
    int majPce[3]; // how many major pieces (Rooks, queens)
    int minPce[3]; // how many minor pieces (Bishops, Knights)
} S_BOARD;








#endif /* DEFS_H */

