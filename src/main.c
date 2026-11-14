#include "defs.h"
#include <stdio.h>

static void EvaluateAndPrint(S_BOARD *pos, const char *label, const char *fen) {
    ParseFen(fen, pos);
    printf("==================================================\n");
    printf("Position: %s\n", label);
    PrintBoard(pos);

    pos->style = STYLE_BALANCED;
    int bal = EvalPosition(pos);

    pos->style = STYLE_POSITIONAL;
    int positional = EvalPosition(pos);

    pos->style = STYLE_SHARP;
    int sharp = EvalPosition(pos);

    const char *stm = (pos->side == WHITE) ? "White" : "Black";
    printf("Side to Move : %s\n", stm);
    printf("Balanced     : %+.2f (%+d cp)\n", bal / 100.0, bal);
    printf("Positional   : %+.2f (%+d cp)\n", positional / 100.0, positional);
    printf("Sharp        : %+.2f (%+d cp)\n\n", sharp / 100.0, sharp);
}

int main(void) {
    AllInit();

    S_BOARD board[1];

    // 1. Symmetric Starting Position (Expected: 0 cp across all styles)
    EvaluateAndPrint(board, "1. Starting Position (Symmetric)", START_FEN);

    // 2. White Advantage: Extra Bishop, Passed d5 Pawn, Open Black King File
    EvaluateAndPrint(board, "2. White Tactical & Material Advantage",
                     "r1bq1rk1/ppp2p1p/2n5/3P4/2B1N3/8/PPP2PPP/R1BQ1RK1 w - - 0 1");

    // 3. Passed Pawn Race (White has a dangerous 6th-rank passed pawn on e6)
    EvaluateAndPrint(board, "3. Passed Pawn on e6 (Positional vs Sharp weighting)",
                     "4k3/pp3ppp/4P3/8/8/8/PP3PPP/4K3 w - - 0 1");

    // 4. King Endgame Centralization (White King centralized on d4 vs Black King passive on a8)
    EvaluateAndPrint(board, "4. Endgame King Activity (KingE PST active)",
                     "k7/8/8/8/3K4/8/8/8 w - - 0 1");

    // 5. Black to Move with Structural Weakness (Black has isolated pawns, White has Bishop Pair)
    EvaluateAndPrint(board,
                     "5. Black to Move vs Bishop Pair & Isolated Pawns (Expect negative score)",
                     "r1bqk2r/p1p1p1p1/8/8/8/2B2B2/PPP2PPP/R2QK2R b KQkq - 0 1");

    // 6. Locked Closed Center (Equal material, 0 open files, structural advantage for White)
    EvaluateAndPrint(board, "6. Closed Positional Structure (Locked Center & Pawn Islands)",
                     "r2q1rk1/pp1n1ppp/2pbpn2/3p4/2PPp3/2N1PNP1/PP1B1PBP/R2Q1RK1 w - - 0 1");
    return 0;
}