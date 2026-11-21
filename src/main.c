#include "defs.h"
#include <stdio.h>
#include <string.h>


static void RunMateIn2Test(S_BOARD *pos, const char *name, const char *fen,
                           const char *expectedMove) {
    ClearHashTable(HashTable);
    ParseFen(fen, pos);

    S_SEARCHINFO info[1];
    memset(info, 0, sizeof(S_SEARCHINFO));
    info->depth = 5;
    info->timeset = FALSE;
    info->starttime = GetTimeMs();

    printf("==================================================\n");
    printf("Puzzle: %s\n", name);
    PrintBoard(pos);

    SearchPosition(pos, info);

    const char *found = PrMove(pos->PvArray[0]);
    printf("Expected: %s | Engine Found: %s -> %s\n\n", expectedMove, found,
           (strcmp(found, expectedMove) == 0) ? "PASS" : "FAIL");
}

int main(void) {
    AllInit();

    S_BOARD board[1];

    RunMateIn2Test(board, "1. Quiet Back-Rank Queen Sacrifice",
                   "4r1k1/5ppp/8/8/8/8/3Q1PPP/3R2K1 w - - 0 1", "d2d8");

    RunMateIn2Test(board, "2. Classic Smothered Mate", "5r1k/6pp/7N/8/8/1Q6/6PP/7K w - - 0 1",
                   "b3g8");

    RunMateIn2Test(board, "3. Arabian Rook & Knight Mate", "7k/5p2/5N1p/8/8/8/6PP/4R2K w - - 0 1",
                   "e1e8");

    RunMateIn2Test(board, "4. Black Double-Check Mate", "6k1/5pp1/2q4p/8/8/5p1P/5PPK/6r1 b - - 0 1",
                   "g1g2");

    return 0;
}