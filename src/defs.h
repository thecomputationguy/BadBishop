/**
 * @file defs.h
 * @brief Core definitions, data structures, macros, globals, and function prototypes for BadBishop.
 */

#ifndef DEFS_H
#define DEFS_H

typedef unsigned long long U64; /**< Custom type for the pieces (64-bit unsigned integer bitboard). */

#define NAME "Bad Bishop 1.0" /**< Engine name and version string. */
#define BRD_SQ_NUM 120        /**< Total number of squares in the 10x12 mailbox board representation. */

/**
 * @brief Converts a square from 120-square board to 64-square board.
 * @param sq120 Square index on the 120-square board.
 */
#define SQ64(sq120) (Sq120ToSq64[(sq120)])

/**
 * @brief Converts a square from 64-square board to 120-square board.
 * @param sq64 Square index on the 64-square board.
 */
#define SQ120(sq64) (Sq64ToSq120[(sq64)])

/**
 * @brief Sets a bit in the bitboard.
 * @param bb Bitboard variable to modify.
 * @param sq 64-square index of the bit to set.
 */
#define SETBIT(bb, sq) ((bb) |= (1ULL << (sq)))

/**
 * @brief Clears a bit in the bitboard.
 * @param bb Bitboard variable to modify.
 * @param sq 64-square index of the bit to clear.
 */
#define CLRBIT(bb, sq) ((bb) &= ~(1ULL << (sq)))

/**
 * @brief Converts file and rank to a square index in the 120-square board.
 * @param f File index (FILE_A to FILE_H).
 * @param r Rank index (RANK_1 to RANK_8).
 */
#define FR2SQ(f, r) ((21 + (f)) + ((r) * 10))

/** @brief Starting position in Forsyth-Edwards Notation (FEN). */
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/**
 * @name Game Move Bitpacking Macros
 * @{
 */
#define FROMSQ(m) ((m) & 0x7F)           /**< Extracts the origin square from a packed move. */
#define TOSQ(m) (((m) >> 7) & 0x7F)      /**< Extracts the destination square from a packed move. */
#define CAPTURED(m) (((m) >> 14) & 0xF)  /**< Extracts the captured piece type from a packed move. */
#define PROMOTED(m) (((m) >> 20) & 0xF)  /**< Extracts the promoted piece type from a packed move. */

#define MFLAGEP 0x40000   /**< Move flag indicating an en passant capture. */
#define MFLAGPS 0x80000   /**< Move flag indicating a two-square pawn start. */
#define MFLAGCA 0x1000000 /**< Move flag indicating a castling move. */

#define MFLAGCAP 0x7C000   /**< Bitmask for any capture move (including en passant). */
#define MFLAGPROM 0xF00000 /**< Bitmask for any pawn promotion move. */

/**
 * @brief Helper macro to pack a move integer.
 * @param f Origin square (0-119).
 * @param t Destination square (0-119).
 * @param ca Captured piece type.
 * @param pro Promoted piece type.
 * @param fl Special move flags (e.g., MFLAGEP, MFLAGPS, MFLAGCA).
 */
#define MOVE(f, t, ca, pro, fl) ((f) | ((t) << 7) | ((ca) << 14) | ((pro) << 20) | (fl))
/** @} */

#define MAXPOSITIONMOVES 256 /**< Maximum number of legal/pseudo-legal moves in a single position. */
#define MAXGAMEMOVES 2048    /**< Maximum number of half-moves in a game history. */

/**
 * @brief Represents a single chess move and its move-ordering score.
 */
typedef struct {
    int move;  /**< Bitpacked move integer. */
    int score; /**< Move ordering / search heuristic score. */
} S_MOVE;

/**
 * @brief List of moves generated for a position.
 */
typedef struct {
    S_MOVE moves[MAXPOSITIONMOVES]; /**< Array of moves. */
    int count;                      /**< Number of moves currently in the list. */
} S_MOVELIST;

/**
 * @name io.c Prototypes
 * @{
 */
/**
 * @brief Converts a square index to algebraic notation (e.g., 21 -> "a1").
 * @param sq Square index on the 120-square board.
 * @return Pointer to a static null-terminated algebraic square string.
 */
char *PrSq(const int sq);

/**
 * @brief Converts a move integer to algebraic notation (e.g., 0x123456 -> "e2e4").
 * @param move Bitpacked move integer.
 * @return Pointer to a static null-terminated algebraic move string.
 */
char *PrMove(const int move);

char *PrSq(const int sq);
char *PrMove(const int move);

/**
 * @brief Prints a list of moves in algebraic notation.
 * @param list Pointer to the move list to print.
 */
void PrintMoveList(const S_MOVELIST *list);
/** @} */

/**
 * @brief Prints a 64-bit bitboard in a human-readable 8x8 format.
 * @param bb The 64-bit bitboard to display.
 */
void PrintBitBoard(U64 bb);

/**
 * @brief Counts the number of set bits in the bitboard.
 * @param bb The 64-bit bitboard to count.
 * @return Number of bits set to 1.
 */
int CountBits(U64 bb);

/**
 * @brief Pops the least significant bit from the bitboard and returns its index.
 * @param bb Pointer to the 64-bit bitboard to modify.
 * @return 0-based index (0-63) of the popped least significant bit.
 */
int PopBit(U64 *bb);

extern int FilesBrd[BRD_SQ_NUM]; /**< Maps each square on the 120-square board to its corresponding file (A-H). */
extern int RanksBrd[BRD_SQ_NUM]; /**< Maps each square on the 120-square board to its corresponding rank (1-8). */

/**
 * @brief Board pieces for black (b) and white (w).
 *
 * P -> Pawn, N -> Knight, B -> Bishop, R -> Rook, Q -> Queen, K -> King.
 */
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };

/** @brief Names of files along the board (A-H). */
enum {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NONE
};

/** @brief Names of ranks along the board (1-8). */
enum {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NONE
};

/** @brief Castling permissions bitmask flags. */
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

/** @brief Player name (whose turn is it to move). */
enum { WHITE, BLACK, BOTH };

// clang-format off
/**
 * @brief 120-square mailbox board structure (A-H: Files and 1-8: Ranks) and square indices.
 *
 * @verbatim
 * Row 11:   110  111  112  113  114  115  116  117  118  119   <-- Top off-board border
 * Row 10:   100  101  102  103  104  105  106  107  108  109   <-- Top off-board border
 * Row 9:     90 [ A8   B8   C8   D8   E8   F8   G8   H8 ]  99   <-- Real board (rank 8: 91–98)
 * Row 8:     80 [ A7   B7   C7   D7   E7   F7   G7   H7 ]  89   <-- Real board (rank 7: 81–88)
 * Row 7:     70 [ A6   B6   C6   D6   E6   F6   G6   H6 ]  79   <-- Real board (rank 6: 71–78)
 * Row 6:     60 [ A5   B5   C5   D5   E5   F5   G5   H5 ]  69   <-- Real board (rank 5: 61–68)
 * Row 5:     50 [ A4   B4   C4   D4   E4   F4   G4   H4 ]  59   <-- Real board (rank 4: 51–58)
 * Row 4:     40 [ A3   B3   C3   D3   E3   F3   G3   H3 ]  49   <-- Real board (rank 3: 41–48)
 * Row 3:     30 [ A2   B2   C2   D2   E2   F2   G2   H2 ]  39   <-- Real board (rank 2: 31–38)
 * Row 2:     20 [ A1   B1   C1   D1   E1   F1   G1   H1 ]  29   <-- Real board (rank 1: 21–28)
 * Row 1:     10   11   12   13   14   15   16   17   18   19   <-- Bottom off-board border
 * Row 0:      0    1    2    3    4    5    6    7    8    9   <-- Bottom off-board border
 *            ^                                            ^
 *       Left border                                  Right border
 * @endverbatim
 */
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

/** @brief Boolean constants. */
enum { FALSE, TRUE };

/**
 * @brief Stores state needed to undo a single move.
 */
typedef struct {
    int move;       /**< Move that was played. */
    int castlePerm; /**< Castling permissions prior to the move. */
    int enPas;      /**< En passant square prior to the move. */
    int fiftyMove;  /**< Fifty-move counter prior to the move. */
    U64 posKey;     /**< Zobrist position key prior to the move. */
} S_UNDO;

/**
 * @brief Core board structure.
 */
typedef struct {
    int pieces[BRD_SQ_NUM]; /**< Board array indexed 0 to 119. */
    U64 pawns[3];           /**< Pawn bitboards: [0]=white, [1]=black, [2]=both. */

    int KingSq[2]; /**< Fast lookup for White and Black king squares. */

    int side;      /**< Whose turn it is (WHITE or BLACK). */
    int enPas;     /**< Square behind a pawn that just moved 2 squares (or NO_SQ). */
    int fiftyMove; /**< Half-move counter for the 50-move draw rule. */

    int ply;    /**< Depth of search in the current tree branch. */
    int hisPly; /**< Total half-moves played in the whole game. */

    int castlePerm; /**< Castling permissions encoded as a 4-bit integer. */
    int style;      /**< STYLE_BALANCED (0), STYLE_POSITIONAL (1), or STYLE_SHARP (2). */

    U64 posKey; /**< 64-bit Zobrist hash of the current position. */

    int pceNum[13];  /**< Count of each piece type currently alive. */
    int bigPce[3];   /**< Count of non-pawn pieces. */
    int majPce[3];   /**< Count of major pieces (Rooks, Queens). */
    int minPce[3];   /**< Count of minor pieces (Knights, Bishops). */
    int material[3]; /**< Material value for each side (WHITE, BLACK, BOTH). */

    int pList[13][10]; /**< Piece list for each piece type (0-12) and their positions on the board (up to 10 pieces of each type). */

    S_UNDO history[MAXGAMEMOVES]; /**< History of moves made in the game, storing undo information for each move. */
} S_BOARD;

extern int Sq120ToSq64[BRD_SQ_NUM]; /**< Maps 120-square board to 64-square board. */
extern int Sq64ToSq120[64];         /**< Maps 64-square board to 120-square board. */

extern U64 PieceKeys[13][120];  /**< Zobrist keys for each piece type on each square. */
extern U64 SideKey;             /**< Zobrist key for the side to move. */
extern U64 CastleKeys[16];      /**< Zobrist keys for each possible castling rights combination. */
extern const int PieceBig[13];  /**< Indicates whether a piece is a "big" piece (non-pawn). */
extern const int PieceMaj[13];  /**< Indicates whether a piece is a "major" piece (Rook or Queen). */
extern const int PieceMin[13];  /**< Indicates whether a piece is a "minor" piece (Knight or Bishop). */
extern const int PieceVal[13];  /**< Material value of each piece type. */
extern const int PieceCol[13];  /**< Color of each piece type (WHITE, BLACK, or BOTH). */
extern const int PiecePawn[13]; /**< Indicates whether a piece is a pawn (1) or not (0). */

/**
 * @name Piece Type Classification Arrays
 * @{
 */
extern const int PieceKnight[13];      /**< Indicates whether a piece is a knight (TRUE/FALSE). */
extern const int PieceKing[13];        /**< Indicates whether a piece is a king (TRUE/FALSE). */
extern const int PieceRookQueen[13];   /**< Indicates whether a piece is a rook or queen (TRUE/FALSE). */
extern const int PieceBishopQueen[13]; /**< Indicates whether a piece is a bishop or queen (TRUE/FALSE). */
extern const char SideChar[];          /**< Character representations for sides ('w', 'b', '-'). */
/** @} */

/**
 * @name attack.c Prototypes
 * @{
 */
/**
 * @brief Checks if a square is attacked by the specified side.
 * @param sq Square index on the 120-square board.
 * @param side Attacking side (WHITE or BLACK).
 * @param pos Pointer to the board state.
 * @return TRUE if the square is attacked, FALSE otherwise.
 */
int SqAttacked(const int sq, const int side, const S_BOARD *pos);
/** @} */

/**
 * @brief Initializes the Zobrist hash keys for pieces, side, and castling rights.
 */
void InitHashKeys(void);

/**
 * @brief Generates a unique 64-bit hash key for the current board position.
 * @param pos Pointer to the board state.
 * @return 64-bit Zobrist position key.
 */
U64 GeneratePosKey(const S_BOARD *pos);

/**
 * @brief Resets the board to an empty state.
 * @param pos Pointer to the board state to reset.
 */
void ResetBoard(S_BOARD *pos);

/**
 * @brief Prints the current board state in a human-readable format.
 * @param pos Pointer to the board state to print.
 */
void PrintBoard(const S_BOARD *pos);

/**
 * @brief Parses a FEN string and sets up the board accordingly.
 * @param fen Null-terminated FEN string.
 * @param pos Pointer to the board state to populate.
 * @return 0 on success, -1 on error.
 */
int ParseFen(const char *fen, S_BOARD *pos);

/**
 * @brief Validates the board state for consistency and correctness.
 * @param pos Pointer to the board state to validate.
 * @return TRUE if the board state is valid, FALSE otherwise.
 */
int CheckBoard(const S_BOARD *pos);

/**
 * @name movegen.c Prototypes
 * @{
 */
/**
 * @brief Generates all possible moves for the current position and stores them in a move list.
 * @param pos Pointer to the board state.
 * @param list Pointer to the move list to populate.
 */
void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);

/**
 * @brief Generates all capture moves (including en passant) for the current position.
 * @param pos Pointer to the board state.
 * @param list Pointer to the move list to populate.
 */
void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
/** @} */

/**
 * @name makemove.c Prototypes
 * @{
 */
/**
 * @brief Makes a move on the board and updates the game state accordingly.
 * @param pos Pointer to the board state.
 * @param move Bitpacked move integer to execute.
 * @return TRUE if the move is legal, FALSE if it leaves the moving side's king in check.
 */
int MakeMove(S_BOARD *pos, int move);

/**
 * @brief Undoes the last move made on the board and restores the previous game state.
 * @param pos Pointer to the board state.
 */
void TakeMove(S_BOARD *pos);
/** @} */

/**
 * @name perft.c Prototypes
 * @{
 */
/**
 * @brief Counts the number of leaf nodes at a given depth from the current position.
 * @param depth Search depth in plies.
 * @param pos Pointer to the board state.
 * @return Total number of leaf nodes visited.
 */
long Perft(int depth, S_BOARD *pos);

/**
 * @brief Runs a perft test to the given depth and prints per-move node counts.
 * @param depth Search depth in plies.
 * @param pos Pointer to the board state.
 */
void PerftTest(int depth, S_BOARD *pos);
/** @} */

/**
 * @brief Engine personality modes.
 */
typedef enum {
    STYLE_BALANCED = 0,   /**< Balanced objective weights and zero contempt. */
    STYLE_POSITIONAL = 1, /**< Positional weights favoring pawn structure and king safety. */
    STYLE_SHARP = 2       /**< Sharp weights favoring mobility, king attack, and negative contempt. */
} ENGINE_STYLE;

/**
 * @brief Evaluation weights and contempt configuration for an engine personality.
 */
typedef struct {
    int materialWeight;   /**< 100 = 1.0x */
    int pstWeight;        /**< Piece-square table weight */
    int pawnStructWeight; /**< Isolated / passed pawns */
    int mobilityWeight;   /**< Open files / piece activity */
    int ownKingSafety;    /**< Defending our own king */
    int oppKingAttack;    /**< Attacking the enemy king */
    int contempt;         /**< Draw score in centipawns (e.g., -50 for Sharp) */
} S_PERSONALITY;

extern const S_PERSONALITY Personalities[3]; /**< Personality configurations indexed by ENGINE_STYLE. */

/**
 * @name Evaluation Bitboard Masks
 * @{
 */
extern U64 FileBBMask[8];       /**< Bitboard masks for each file (A-H). */
extern U64 RankBBMask[8];       /**< Bitboard masks for each rank (1-8). */
extern U64 BlackPassedMask[64]; /**< Passed pawn masks for Black pawns on each 64-square index. */
extern U64 WhitePassedMask[64]; /**< Passed pawn masks for White pawns on each 64-square index. */
extern U64 IsolatedMask[64];    /**< Isolated pawn masks for each 64-square index. */
/** @} */

/**
 * @name evaluate.c Prototypes
 * @{
 */
/**
 * @brief Evaluates the current position and returns a score from the side-to-move's perspective.
 * @param pos Pointer to the board state.
 * @return Static evaluation score in centipawns.
 */
int EvalPosition(const S_BOARD *pos);

/**
 * @brief Returns the contempt value based on the current position's style.
 * @param pos Pointer to the board state.
 * @return Contempt score in centipawns.
 */
int GetContempt(const S_BOARD *pos);
/** @} */

/**
 * @brief Initializes all the necessary data structures for the chess engine.
 */
void AllInit(void);

#endif /* DEFS_H */
