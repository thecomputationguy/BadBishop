#include "defs.h"
#include <assert.h>
#include <stdio.h>


S_HASHTABLE HashTable[1];

#define HASH_INDEX(key) ((key) & (HASH_ENTRIES - 1))

static int MoveExists(S_BOARD *pos, const int move) {
    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        if (!MakeMove(pos, list->moves[MoveNum].move)) {
            continue;
        }
        TakeMove(pos);
        if (list->moves[MoveNum].move == move) {
            return TRUE;
        }
    }
    return FALSE;
}

int GetPvLine(const int depth, S_BOARD *pos) {
    assert(depth < MAXDEPTH);

    int move = ProbePvMove(pos);
    int count = 0;

    while (move != NOMOVE && count < depth) {
        assert(count < MAXDEPTH);

        if (MoveExists(pos, move)) {
            MakeMove(pos, move);
            pos->PvArray[count++] = move;
        } else {
            break;
        }
        move = ProbePvMove(pos);
    }

    while (pos->ply > 0) {
        TakeMove(pos);
    }

    return count;
}

void ClearHashTable(S_HASHTABLE *table) {
    for (int i = 0; i < HASH_ENTRIES; ++i) {
        table->pTable[i].posKey = 0ULL;
        table->pTable[i].move = NOMOVE;
        table->pTable[i].depth = 0;
        table->pTable[i].score = 0;
        table->pTable[i].flags = HFNONE;
    }
    table->newWrite = 0;
    table->overWrite = 0;
    table->hit = 0;
    table->cut = 0;
}

void InitHashTable(S_HASHTABLE *table) {
    table->numEntries = HASH_ENTRIES;
    ClearHashTable(table);
}

int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {
    U64 index = HASH_INDEX(pos->posKey);

    if (HashTable->pTable[index].posKey == pos->posKey) {
        *move = HashTable->pTable[index].move;
        if (HashTable->pTable[index].depth >= depth) {
            HashTable->hit++;

            *score = HashTable->pTable[index].score;
            if (*score > ISMATE) {
                *score -= pos->ply;
            } else if (*score < -ISMATE) {
                *score += pos->ply;
            }

            switch (HashTable->pTable[index].flags) {
            case HFALPHA:
                if (*score <= alpha) {
                    *score = alpha;
                    return TRUE;
                }
                break;
            case HFBETA:
                if (*score >= beta) {
                    *score = beta;
                    return TRUE;
                }
                break;
            case HFEXACT:
                return TRUE;
            default:
                assert(FALSE);
                break;
            }
        }
    }

    return FALSE;
}

void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {
    U64 index = HASH_INDEX(pos->posKey);

    if (HashTable->pTable[index].posKey == 0ULL) {
        HashTable->newWrite++;
    } else {
        HashTable->overWrite++;
    }

    if (score > ISMATE) {
        score += pos->ply;
    } else if (score < -ISMATE) {
        score -= pos->ply;
    }

    HashTable->pTable[index].move = move;
    HashTable->pTable[index].posKey = pos->posKey;
    HashTable->pTable[index].flags = flags;
    HashTable->pTable[index].score = score;
    HashTable->pTable[index].depth = depth;
}

int ProbePvMove(const S_BOARD *pos) {
    U64 index = HASH_INDEX(pos->posKey);

    if (HashTable->pTable[index].posKey == pos->posKey) {
        return HashTable->pTable[index].move;
    }

    return NOMOVE;
}