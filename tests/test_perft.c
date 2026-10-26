#include "defs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_sq_conversions(void) {
    printf("[1/6] Testing 120 <-> 64 Square Conversions...\n");
    // Corners
    assert(SQ64(A1) == 0);
    assert(SQ64(H1) == 7);
    assert(SQ64(A8) == 56);
    assert(SQ64(H8) == 63);

    // Round-trip identity for all 64 squares
    for (int sq64 = 0; sq64 < 64; ++sq64) {
        int sq120 = SQ120(sq64);
        assert(sq120 >= A1 && sq120 <= H8);
        assert(SQ64(sq120) == sq64);
    }

    // Offboard sentinels
    assert(SQ64(0) == 65);
    assert(SQ64(119) == 65);
}

static void test_bitboards(void) {
    printf("[2/6] Testing Bitboard Operations (SET, CLR, CNT, POP)...\n");
    U64 bb = 0ULL;
    assert(CountBits(bb) == 0);

    SETBIT(bb, 0);  // A1
    SETBIT(bb, 28); // E4
    SETBIT(bb, 63); // H8
    assert(CountBits(bb) == 3);

    // PopBit must extract LSB first (0, then 28, then 63)
    int sq = PopBit(&bb);
    assert(sq == 0);
    assert(CountBits(bb) == 2);

    CLRBIT(bb, 28);
    assert(CountBits(bb) == 1);

    sq = PopBit(&bb);
    assert(sq == 63);
    assert(bb == 0ULL);
}

static void test_fen_and_checkboard(void) {
    printf("[3/6] Testing FEN Parser & CheckBoard Integrity...\n");
    S_BOARD board[1];

    // 1. Standard Start FEN
    assert(ParseFen(START_FEN, board) == 0);
    assert(CheckBoard(board) == TRUE);
    assert(board->side == WHITE);
    assert(board->castlePerm == (WKCA | WQCA | BKCA | BQCA));
    assert(board->enPas == NO_SQ);
    assert(board->pceNum[wP] == 8);
    assert(board->pceNum[bP] == 8);
    assert(board->KingSq[WHITE] == E1);
    assert(board->KingSq[BLACK] == E8);

    // 2. Kiwipete (complex tactical position)
    const char *kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    assert(ParseFen(kiwipete, board) == 0);
    assert(CheckBoard(board) == TRUE);

    // 3. En Passant & Partial Castling FEN
    const char *ep_fen = "r1bqk2r/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQK2R w KQkq d6 0 8";
    assert(ParseFen(ep_fen, board) == 0);
    assert(CheckBoard(board) == TRUE);
    assert(board->enPas == D6);
}

static void test_zobrist_determinism(void) {
    printf("[4/6] Testing Zobrist Hash Determinism...\n");
    S_BOARD b1[1], b2[1];

    ParseFen(START_FEN, b1);
    ParseFen(START_FEN, b2);
    assert(b1->posKey == b2->posKey);
    assert(b1->posKey != 0ULL);

    // Changing side to move must alter the hash key
    const char *black_to_move = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
    ParseFen(black_to_move, b2);
    assert(b1->posKey != b2->posKey);
}

static void test_sq_attacked(void) {
    printf("[5/6] Testing SqAttacked Ray & Leaper Detection...\n");
    S_BOARD board[1];

    // White Queen on d4, White Knight on f3, Black Rook on e8
    const char *fen = "4r1k1/ppp2ppp/2n5/8/3Q4/5N2/PPP2PPP/4R1K1 w - - 0 1";
    ParseFen(fen, board);

    // White Queen on D4 attacks D8 (orthogonal), A7 (diagonal), H4 (rank)
    assert(SqAttacked(D8, WHITE, board) == TRUE);
    assert(SqAttacked(A7, WHITE, board) == TRUE);
    assert(SqAttacked(H4, WHITE, board) == TRUE);

    // White Queen on D4 is blocked by friendly pawn on D2 -> D1 not attacked by Q
    // (Though D1 IS attacked by White Rook on E1!)
    assert(SqAttacked(D1, WHITE, board) == TRUE);

    // White Knight on F3 attacks E5 and G5
    assert(SqAttacked(E5, WHITE, board) == TRUE);
    assert(SqAttacked(G5, WHITE, board) == TRUE);

    // Black Knight on C6 attacks D4 and B4
    assert(SqAttacked(D4, BLACK, board) == TRUE);
    assert(SqAttacked(B4, BLACK, board) == TRUE);

    // Black Rook on E8 attacks E4, but is blocked by White Rook on E1 from hitting F1
    assert(SqAttacked(E4, BLACK, board) == TRUE);
    assert(SqAttacked(A4, BLACK, board) == FALSE);
}

static void test_move_encoding_and_io(void) {
    printf("[6/6] Testing 32-Bit Move Bit-Packing & Algebraic Strings...\n");

    // Quiet Pawn Start: e2 -> e4
    int m1 = MOVE(E2, E4, EMPTY, EMPTY, MFLAGPS);
    assert(FROMSQ(m1) == E2);
    assert(TOSQ(m1) == E4);
    assert(CAPTURED(m1) == EMPTY);
    assert(PROMOTED(m1) == EMPTY);
    assert((m1 & MFLAGPS) != 0);
    assert((m1 & MFLAGCAP) == 0);
    assert(strcmp(PrMove(m1), "e2e4") == 0);

    // Capture + Knight Promotion: b7 -> a8=N capturing Black Rook
    int m2 = MOVE(B7, A8, bR, wN, 0);
    assert(FROMSQ(m2) == B7);
    assert(TOSQ(m2) == A8);
    assert(CAPTURED(m2) == bR);
    assert(PROMOTED(m2) == wN);
    assert((m2 & MFLAGCAP) != 0);
    assert((m2 & MFLAGPROM) != 0);
    assert(strcmp(PrMove(m2), "b7a8n") == 0);
}

int main(void) {
    AllInit();

    test_sq_conversions();
    test_bitboards();
    test_fen_and_checkboard();
    test_zobrist_determinism();
    test_sq_attacked();
    test_move_encoding_and_io();

    printf("\n========================================\n");
    printf(" ALL 6 UNIT TEST SUITES PASSED CLEANLY! \n");
    printf("========================================\n");
    return 0;
}