#ifndef DEFS_H
#define DEFS_H

typedef unsigned long long U64; // Custom type for the pieces

#define NAME "Bad Bishop 1.0"
#define BRD_SQ_NUM 120

#define SQ64(sq120) (Sq120ToSq64[(sq120)]) // Converts a square from 120-square board to 64-square board
#define SQ120(sq64) (Sq64ToSq120[(sq64)]) // Converts a square from 64-square board to 120-square board

#define SETBIT(bb, sq) ((bb) |= (1ULL << (sq))) // Sets a bit in the bitboard
#define CLRBIT(bb, sq) ((bb) &= ~(1ULL << (sq))) // Clears a bit in the bitboard

#define FR2SQ(f, r) ((21 + (f)) + ((r) * 10)) // Converts file and rank to a square index in the 120-square board
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" // Starting position in Forsyth-Edwards Notation (FEN)

/* --- GAME MOVE BITPACKING MACROS --- */
#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m) >> 7) & 0x7F)
#define CAPTURED(m) (((m) >> 14) & 0xF)
#define PROMOTED(m) (((m) >> 20) & 0xF)

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

/* Helper macro to pack a move integer */
#define MOVE(f, t, ca, pro, fl) ((f) | ((t) << 7) | ((ca) << 14) | ((pro) << 20) | (fl))

/* --- IO.C PROTOTYPES --- */
char *PrSq(const int sq);
char *PrMove(const int move);

void PrintBitBoard(U64 bb);
int CountBits(U64 bb); // Counts the number of set bits in the bitboard
int PopBit(U64 *bb); // Pops the least significant bit from the bitboard and returns its index

extern int FilesBrd[BRD_SQ_NUM]; // Maps each square on the 120-square board to its corresponding file (A-H)
extern int RanksBrd[BRD_SQ_NUM]; // Maps each square on the 120-square board to its corresponding rank (1-8)

// Board pieces for black (b) and white (w).
// P -> Pawn, N -> Knight, B -> Bishop, R -> Rook, Q -> Queen, K -> King
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK }; 
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE }; // Names of files along the board (A-H)
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE }; // Names of ranks along the board (1-8)
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 }; // Castling permissions
enum { WHITE, BLACK, BOTH}; // Player name (whose turn is it to move)

// clang-format off
// Board structure (A-H: Files and 1-8: Ranks)
/*
Row 11:   110  111  112  113  114  115  116  117  118  119   <-- Top off-board border
Row 10:   100  101  102  103  104  105  106  107  108  109   <-- Top off-board border
Row 9:     90 [ A8   B8   C8   D8   E8   F8   G8   H8 ]  99   <-- Real board (rank 8: 91–98)
Row 8:     80 [ A7   B7   C7   D7   E7   F7   G7   H7 ]  89   <-- Real board (rank 7: 81–88)
Row 7:     70 [ A6   B6   C6   D6   E6   F6   G6   H6 ]  79   <-- Real board (rank 6: 71–78)
Row 6:     60 [ A5   B5   C5   D5   E5   F5   G5   H5 ]  69   <-- Real board (rank 5: 61–68)
Row 5:     50 [ A4   B4   C4   D4   E4   F4   G4   H4 ]  59   <-- Real board (rank 4: 51–58)
Row 4:     40 [ A3   B3   C3   D3   E3   F3   G3   H3 ]  49   <-- Real board (rank 3: 41–48)
Row 3:     30 [ A2   B2   C2   D2   E2   F2   G2   H2 ]  39   <-- Real board (rank 2: 31–38)
Row 2:     20 [ A1   B1   C1   D1   E1   F1   G1   H1 ]  29   <-- Real board (rank 1: 21–28)
Row 1:     10   11   12   13   14   15   16   17   18   19   <-- Bottom off-board border
Row 0:      0    1    2    3    4    5    6    7    8    9   <-- Bottom off-board border
           ^                                            ^
      Left border                                  Right border
*/
// clang-format on

// clang-format off
enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2, 
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};
// clang-format on

enum { FALSE, TRUE};

// Core board structure
typedef struct {
    int pieces[BRD_SQ_NUM]; // Board array indexed 0 to 119
    U64 pawns[3];           // Pawn bitboards: [0]=white, [1]=black, [2]=both
         
    int KingSq[2];          // Fast lookup for White and Black king squares
         
    int side;               // Whose turn it is (WHITE or BLACK)
    int enPas;              // Square behind a pawn that just moved 2 squares (or NO_SQ)
    int fiftyMove;          // Half-move counter for the 50-move draw rule
         
    int ply;                // Depth of search in the current tree branch
    int hisPly;             // Total half-moves played in the whole game

    int castlePerm;         // Castling permissions encoded as a 4-bit integer:
         
    U64 posKey;             // 64-bit Zobrist hash of the current position
         
    int pceNum[13];         // Count of each piece type currently alive
    int bigPce[3];          // Count of non-pawn pieces
    int majPce[3];          // Count of major pieces (Rooks, Queens)
    int minPce[3];          // Count of minor pieces (Knights, Bishops)
    int material[3];        // Material value for each side (WHITE, BLACK, BOTH)

    int pList[13][10]; // Piece list for each piece type (0-12) and their positions on the board (up to 10 pieces of each type)
} S_BOARD;

extern int Sq120ToSq64[BRD_SQ_NUM]; // Maps 120-square board to 64-square board
extern int Sq64ToSq120[64];         // Maps 64-square board to 120-square board

extern U64 PieceKeys[13][120]; // Zobrist keys for each piece type on each square
extern U64 SideKey; // Zobrist key for the side to move
extern U64 CastleKeys[16]; // Zobrist keys for each possible castling rights combination
extern const int PieceBig[13]; // Indicates whether a piece is a "big" piece (non-pawn)
extern const int PieceMaj[13]; // Indicates whether a piece is a "major" piece (Rook or Queen)
extern const int PieceMin[13]; // Indicates whether a piece is a "minor" piece (Knight or Bishop)
extern const int PieceVal[13]; // Material value of each piece type
extern const int PieceCol[13]; // Color of each piece type (WHITE, BLACK, or BOTH)

// Piece type classification arrays
extern const int PieceKnight[13];
extern const int PieceKing[13];
extern const int PieceRookQueen[13];
extern const int PieceBishopQueen[13];
extern const char SideChar[];

// attack.c
int SqAttacked(const int sq, const int side, const S_BOARD *pos);

void InitHashKeys(void); // Initializes the Zobrist hash keys for pieces, side, and castling rights
U64 GeneratePosKey(const S_BOARD *pos); // Generates a unique 64-bit hash key for the current board position

void ResetBoard(S_BOARD *pos); // Resets the board to an empty state
void PrintBoard(const S_BOARD *pos); // Prints the current board state in a human-readable format
int ParseFen(const char *fen, S_BOARD *pos); // Parses a FEN string and sets up the board accordingly
int CheckBoard(const S_BOARD *pos); // Validates the board state for consistency and correctness

void AllInit(void); // Initializes all the necessary data structures for the chess engine


#endif /* DEFS_H */

