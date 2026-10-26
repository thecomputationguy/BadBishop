#include "defs.h"

int Sq120ToSq64[BRD_SQ_NUM]; // Maps 120-square board to 64-square board
int Sq64ToSq120[64];         // Maps 64-square board to 120
int FilesBrd[BRD_SQ_NUM]; // Maps each square on the 120-square board to its corresponding file (A-H)
int RanksBrd[BRD_SQ_NUM]; // Maps each square on the 120-square board to its corresponding file (A-H) and rank (1-8)

const int PieceBig[13] = { FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE }; // Indicates whether a piece is a "big" piece (non-pawn)
const int PieceMaj[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE }; // Indicates whether a piece is a "major" piece (Rook or Queen)
const int PieceMin[13] = { FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE }; // Indicates whether a piece is a "minor" piece (Knight or Bishop)
const int PieceVal[13] = { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000 }; // Material value of each piece type
const int PieceCol[13] = { BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK }; // Color of each piece type (WHITE, BLACK, or BOTH)

// Indices: EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK
const int PieceKnight[13]      = { FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE };
const int PieceKing[13]        = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE };
const int PieceRookQueen[13]   = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE };
const int PieceBishopQueen[13] = { FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE };

static void InitSq120To64(void){
    int index = 0;
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1; // Starting square in 120-square board
    int sq64 = 0; // Starting square in 64-square board

    // Reset all 120 squares to an invalid index (65)
    for(index = 0; index < BRD_SQ_NUM; index++) {
        Sq120ToSq64[index] = 65; // Invalid value for squares not on the board
    }

    // Reset all 64 squares to an invalid square (120)
    for(index = 0; index < 64; index++) {
        Sq64ToSq120[index] = 120; // Invalid value for squares not on the board
    }

    // Populate the mapping arrays for valid squares on the board
    for(rank = RANK_1; rank <= RANK_8; rank++) {
        for(file = FILE_A; file <= FILE_H; file++){
            sq = 21 + file + (rank * 10); // Calculate the square index in 120-square board
            Sq64ToSq120[sq64] = sq; // Map 64-square index to 120-square index
            Sq120ToSq64[sq] = sq64; // Map 120-square index to 64-square index
            sq64++; // Move to the next square in 64-square board
        }
    }
}

static void InitFilesRanksBrd() {
    int index = 0;
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1; // Starting square in 120-square board
    int sq64 = 0; // Starting square in 64-square board

    // Reset all 120 squares to an invalid index (65)
    for(index = 0; index < BRD_SQ_NUM; index++) {
        FilesBrd[index] = OFFBOARD; // Invalid value for squares not on the board
        RanksBrd[index] = OFFBOARD; // Invalid value for squares not on the board
    }

    // Populate the mapping arrays for valid squares on the board
    for(rank = RANK_1; rank <= RANK_8; rank++) {
        for(file = FILE_A; file <= FILE_H; file++){
            sq = 21 + file + (rank * 10); // Calculate the square index in 120-square board
            FilesBrd[sq] = file; // Map 120-square index to file
            RanksBrd[sq] = rank; // Map 120-square index to rank
        }
    }
}

void AllInit(void) {
    InitSq120To64(); // Initialize the square mapping arrays
    InitFilesRanksBrd(); // Initialize the file and rank mapping arrays
    InitHashKeys(); // Initialize the Zobrist hash keys
}