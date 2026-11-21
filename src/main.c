#include "defs.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    AllInit();

    S_BOARD pos[1];
    S_SEARCHINFO info[1];
    memset(pos, 0, sizeof(S_BOARD));
    memset(info, 0, sizeof(S_SEARCHINFO));

    ParseFen(START_FEN, pos);
    UciLoop(pos, info);

    return 0;
}