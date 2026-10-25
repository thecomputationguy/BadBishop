#include <stdio.h>
#include "defs.h"

const char PceChar[] = ".PNBRQKpnbrqk";
const char SideChar[] = "wb-";

void PrintBoard(const S_BOARD *pos) {
    /*
    Prints the current board state in a human-readable format.
    Displays the board with ranks and files, along with the side to move, en passant square, castling rights, and the position key.
    */
    int sq, file, rank, piece;

    printf("\nGame Board:\n\n");

    for (rank = RANK_8; rank >= RANK_1; rank--) {
        printf("%d  ", rank + 1);
        for (file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank);
            piece = pos->pieces[sq];
            printf("%3c", PceChar[piece]);
        }
        printf("\n");
    }

    printf("\n   ");
    for (file = FILE_A; file <= FILE_H; file++) {
        printf("%3c", 'a' + file);
    }
    printf("\n\n");

    printf("side: %c\n", SideChar[pos->side]);
    printf("enPas: %d\n", pos->enPas);
    printf("castle: %c%c%c%c\n",
           pos->castlePerm & WKCA ? 'K' : '-',
           pos->castlePerm & WQCA ? 'Q' : '-',
           pos->castlePerm & BKCA ? 'k' : '-',
           pos->castlePerm & BQCA ? 'q' : '-');
    printf("posKey: %llX\n\n", pos->posKey);

}

void ResetBoard(S_BOARD *pos) {
    /*
    Resets the board to an empty state.
    */
    int index = 0;

    // 1. Set all 120 squares to OFFBOARD
    for (index = 0; index < BRD_SQ_NUM; index++) {
        pos->pieces[index] = OFFBOARD;
    }

    // 2. Set the 64 squares on the board to EMPTY
    for (index = 0; index < 64; index++) {
        int sq120 = Sq64ToSq120[index];
        pos->pieces[sq120] = EMPTY;
    }

    // 3. Reset piece counters
    for (index = 0; index < 3; ++index) {
        pos->bigPce[index] = 0;
        pos->majPce[index] = 0;
        pos->minPce[index] = 0;
        pos->material[index] = 0;
        pos->pawns[index] = 0ULL; // Reset pawn bitboards
    }

    for (index = 0; index < 13; ++index) {
        pos->pceNum[index] = 0;
    }

    // 4. Reset king coordinates
    pos->KingSq[WHITE] = NO_SQ;
    pos->KingSq[BLACK] = NO_SQ;

    // 5. Reset board state flags
    pos->side = BOTH; // No side to move
    pos->enPas = NO_SQ; // No en passant square
    pos->fiftyMove = 0; // Reset fifty-move counter
    pos->ply = 0; // Reset search depth
    pos->hisPly = 0; // Reset total half-moves played
    pos->castlePerm = 0; // No castling rights
    pos->posKey = 0ULL; // Reset position key

    // 6. Reset piece list
    for (index = 0; index < 13; ++index) {
        int j;
        for (j = 0; j < 10; ++j) {
            pos->pList[index][j] = NO_SQ;
        }
    }
}

#include <stdio.h>
#include "defs.h"

int CheckBoard(const S_BOARD *pos) {
    /*
    Validates the board state for consistency and correctness.
    */
    int t_pceNum[13] = { 0 };
    int t_bigPce[3] = { 0 };
    int t_majPce[3] = { 0 };
    int t_minPce[3] = { 0 };
    int t_material[3] = { 0 };

    int sq64, t_piece, t_pce_num, sq120, colour, pcount;

    U64 t_pawns[3] = { 0ULL, 0ULL, 0ULL };

    t_pawns[WHITE] = pos->pawns[WHITE];
    t_pawns[BLACK] = pos->pawns[BLACK];
    t_pawns[BOTH]  = pos->pawns[BOTH];

    // 1. Audit piece counts, bitboards, and materials from square array
    for (sq64 = 0; sq64 < 64; ++sq64) {
        sq120 = SQ120(sq64);
        t_piece = pos->pieces[sq120];
        t_pceNum[t_piece]++;
        colour = PieceCol[t_piece];

        if (PieceBig[t_piece] == TRUE) t_bigPce[colour]++;
        if (PieceMin[t_piece] == TRUE) t_minPce[colour]++;
        if (PieceMaj[t_piece] == TRUE) t_majPce[colour]++;

        t_material[colour] += PieceVal[t_piece];
        t_material[BOTH]   += PieceVal[t_piece];
    }

    // 2. Validate piece counts
    for (t_piece = wP; t_piece <= bK; ++t_piece) {
        if (t_pceNum[t_piece] != pos->pceNum[t_piece]) {
            printf("Error: pceNum mismatch on piece %d\n", t_piece);
            return FALSE;
        }
    }

    // 3. Validate pawn bitboard counts
    pcount = CountBits(t_pawns[WHITE]);
    if (pcount != pos->pceNum[wP]) {
        printf("Error: White pawn bitboard count mismatch\n");
        return FALSE;
    }
    pcount = CountBits(t_pawns[BLACK]);
    if (pcount != pos->pceNum[bP]) {
        printf("Error: Black pawn bitboard count mismatch\n");
        return FALSE;
    }
    pcount = CountBits(t_pawns[BOTH]);
    if (pcount != (pos->pceNum[wP] + pos->pceNum[bP])) {
        printf("Error: Combined pawn bitboard count mismatch\n");
        return FALSE;
    }

    // 4. Validate pawn bitboard square positions
    while (t_pawns[WHITE]) {
        sq64 = PopBit(&t_pawns[WHITE]);
        if (pos->pieces[SQ120(sq64)] != wP) {
            printf("Error: White pawn bit mismatch on sq %d\n", sq64);
            return FALSE;
        }
    }
    while (t_pawns[BLACK]) {
        sq64 = PopBit(&t_pawns[BLACK]);
        if (pos->pieces[SQ120(sq64)] != bP) {
            printf("Error: Black pawn bit mismatch on sq %d\n", sq64);
            return FALSE;
        }
    }
    while (t_pawns[BOTH]) {
        sq64 = PopBit(&t_pawns[BOTH]);
        if (pos->pieces[SQ120(sq64)] != wP && pos->pieces[SQ120(sq64)] != bP) {
            printf("Error: Combined pawn bit mismatch on sq %d\n", sq64);
            return FALSE;
        }
    }

    // 5. Validate material and piece type counters
    if (t_material[WHITE] != pos->material[WHITE] ||
        t_material[BLACK] != pos->material[BLACK] ||
        t_material[BOTH]  != pos->material[BOTH]) {
        printf("Error: Material balance mismatch\n");
        return FALSE;
    }

    if (t_minPce[WHITE] != pos->minPce[WHITE] ||
        t_minPce[BLACK] != pos->minPce[BLACK] ||
        t_majPce[WHITE] != pos->majPce[WHITE] ||
        t_majPce[BLACK] != pos->majPce[BLACK] ||
        t_bigPce[WHITE] != pos->bigPce[WHITE] ||
        t_bigPce[BLACK] != pos->bigPce[BLACK]) {
        printf("Error: Piece category count mismatch\n");
        return FALSE;
    }

    // 6. Validate side to move
    if (pos->side != WHITE && pos->side != BLACK) {
        printf("Error: Invalid active side: %d\n", pos->side);
        return FALSE;
    }

    // 7. Validate position hash key
    if (GeneratePosKey(pos) != pos->posKey) {
        printf("Error: Hash key mismatch\n");
        return FALSE;
    }

    // 8. Validate en-passant square
    if (pos->enPas != NO_SQ) {
        if ((pos->side == WHITE && (RanksBrd[pos->enPas] != RANK_6)) ||
            (pos->side == BLACK && (RanksBrd[pos->enPas] != RANK_3))) {
            printf("Error: Invalid en-passant square rank: %d\n", pos->enPas);
            return FALSE;
        }
    }

    // 9. Validate kings on board
    if (pos->pieces[pos->KingSq[WHITE]] != wK) {
        printf("Error: White king square corrupted\n");
        return FALSE;
    }
    if (pos->pieces[pos->KingSq[BLACK]] != bK) {
        printf("Error: Black king square corrupted\n");
        return FALSE;
    }

    // 10. Validate piece list consistency
    for (t_piece = wP; t_piece <= bK; ++t_piece) {
        for (t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
            sq120 = pos->pList[t_piece][t_pce_num];
            if (pos->pieces[sq120] != t_piece) {
                printf("Error: Piece list mismatch for piece %d at sq120 %d\n", t_piece, sq120);
                return FALSE;
            }
        }
    }

    return TRUE;
}

int ParseFen(const char *fen, S_BOARD *pos) {
    if (fen == NULL || pos == NULL) return -1;

    int rank = RANK_8;
    int file = FILE_A;
    int piece = 0;
    int count = 0;
    int i = 0;
    int sq64 = 0;
    int sq120 = 0;

    ResetBoard(pos);

    // 1. Piece placement
    while ((rank >= RANK_1) && *fen) {
        count = 1;
        switch (*fen) {
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error on piece character: %c\n", *fen);
                return -1;
        }

        for (i = 0; i < count; ++i) {
            sq64 = rank * 8 + file;
            sq120 = SQ120(sq64);
            if (piece != EMPTY) {
                pos->pieces[sq120] = piece;
            }
            file++;
        }
        fen++;
    }

    // 2. Active color ('w' or 'b')
    if (*fen != 'w' && *fen != 'b') {
        printf("FEN error on active color: %c\n", *fen);
        return -1;
    }
    pos->side = (*fen == 'w') ? WHITE : BLACK;
    fen += 2; // Skip color and trailing space

    // 3. Castling rights ('K', 'Q', 'k', 'q' or '-')
    for (i = 0; i < 4; ++i) {
        if (*fen == ' ') break;
        switch (*fen) {
            case 'K': pos->castlePerm |= WKCA; break;
            case 'Q': pos->castlePerm |= WQCA; break;
            case 'k': pos->castlePerm |= BKCA; break;
            case 'q': pos->castlePerm |= BQCA; break;
            case '-': break;
            default: break;
        }
        fen++;
    }
    fen++; // Skip space

    // 4. En passant target square
    if (*fen != '-') {
        file = fen[0] - 'a';
        rank = fen[1] - '1';
        pos->enPas = FR2SQ(file, rank);
        fen += 2;
    } else {
        pos->enPas = NO_SQ;
        fen++;
    }
    fen++; // Skip space

    // 5. Populate piece bitboards, counts, and king squares
    for (i = 0; i < 64; ++i) {
        sq120 = SQ120(i);
        piece = pos->pieces[sq120];
        if (piece != EMPTY) {
            int color = PieceCol[piece];
            if (PieceBig[piece] == TRUE) pos->bigPce[color]++;
            if (PieceMin[piece] == TRUE) pos->minPce[color]++;
            if (PieceMaj[piece] == TRUE) pos->majPce[color]++;

            pos->material[color] += PieceVal[piece];
            pos->material[BOTH]  += PieceVal[piece]; // Total on-board material

            // Update piece list and piece count
            pos->pList[piece][pos->pceNum[piece]] = sq120;
            pos->pceNum[piece]++;

            if (piece == wK) pos->KingSq[WHITE] = sq120;
            if (piece == bK) pos->KingSq[BLACK] = sq120;

            if (piece == wP) {
                SETBIT(pos->pawns[WHITE], i);
                SETBIT(pos->pawns[BOTH], i);
            } else if (piece == bP) {
                SETBIT(pos->pawns[BLACK], i);
                SETBIT(pos->pawns[BOTH], i);
            }
        }
    }

    // 6. Generate the initial position key
    pos->posKey = GeneratePosKey(pos);

    return 0;
}