/**
 * @file uci.c
 * @brief Universal Chess Interface (UCI) protocol command loop and option parsing.
 */

#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUTBUFFER 4096 /**< Maximum line buffer size for reading UCI commands from stdin. */

static int currentStyle = STYLE_BALANCED; /**< Active engine personality persisted across new positions. */

/**
 * @brief Parses a UCI "go" command, configures time/depth limits, and starts the search.
 * @param line Null-terminated UCI "go" command string.
 * @param info Pointer to search parameters and timing state.
 * @param pos Pointer to the current board state.
 */
static void ParseGo(const char *line, S_SEARCHINFO *info, S_BOARD *pos) {
    int depth = -1, movestogo = 30, movetime = -1;
    int time = -1, inc = 0;
    const char *ptr = NULL;

    info->timeset = FALSE;

    if ((ptr = strstr(line, "binc")) && pos->side == BLACK) inc = atoi(ptr + 5);
    if ((ptr = strstr(line, "winc")) && pos->side == WHITE) inc = atoi(ptr + 5);
    if ((ptr = strstr(line, "wtime")) && pos->side == WHITE) time = atoi(ptr + 6);
    if ((ptr = strstr(line, "btime")) && pos->side == BLACK) time = atoi(ptr + 6);
    if ((ptr = strstr(line, "movestogo"))) movestogo = atoi(ptr + 10);
    if ((ptr = strstr(line, "movetime"))) movetime = atoi(ptr + 9);
    if ((ptr = strstr(line, "depth"))) depth = atoi(ptr + 6);

    if (movetime != -1) {
        time = movetime;
        movestogo = 1;
    }

    info->starttime = GetTimeMs();
    info->depth = (depth != -1) ? depth : MAXDEPTH;

    if (time != -1) {
        info->timeset = TRUE;
        time /= movestogo;
        if (time > 50) time -= 50; /* Safety buffer for I/O latency */
        info->stoptime = info->starttime + time + inc;
    }

    SearchPosition(pos, info);
}

/**
 * @brief Parses a UCI "position" command ("startpos" or "fen") and applies any trailing moves.
 * @param lineIn Null-terminated UCI "position" command string.
 * @param pos Pointer to the board state to update.
 */
static void ParsePosition(const char *lineIn, S_BOARD *pos) {
    lineIn += 9; /* Skip "position " */
    const char *ptrChar = lineIn;

    if (strncmp(lineIn, "startpos", 8) == 0) {
        ParseFen(START_FEN, pos);
    } else {
        ptrChar = strstr(lineIn, "fen");
        if (ptrChar == NULL) {
            ParseFen(START_FEN, pos);
        } else {
            ptrChar += 4;
            ParseFen(ptrChar, pos);
        }
    }

    /* Restore user-selected UCI personality after ParseFen resets board state */
    pos->style = currentStyle;

    ptrChar = strstr(lineIn, "moves");
    if (ptrChar != NULL) {
        ptrChar += 6;
        while (*ptrChar) {
            int move = ParseMove(ptrChar, pos);
            if (move == NOMOVE) break;
            MakeMove(pos, move);
            pos->ply = 0;

            while (*ptrChar && *ptrChar != ' ')
                ptrChar++;
            while (*ptrChar == ' ')
                ptrChar++;
        }
    }
}

/**
 * @brief Parses a UCI "setoption" command to configure Hash size or engine Personality.
 * @param line Null-terminated UCI "setoption" command string.
 * @param pos Pointer to the board state whose personality style may be updated.
 */
static void ParseSetOption(const char *line, S_BOARD *pos) {
    if (strstr(line, "name Hash value")) {
        const char *valPtr = strstr(line, "value");
        if (valPtr) {
            int mb = atoi(valPtr + 6);
            InitHashTable(HashTable, mb);
        }
    } else if (strstr(line, "name Personality value")) {
        if (strstr(line, "Positional")) {
            currentStyle = STYLE_POSITIONAL;
        } else if (strstr(line, "Sharp")) {
            currentStyle = STYLE_SHARP;
        } else {
            currentStyle = STYLE_BALANCED;
        }
        pos->style = currentStyle;
    }
}

/**
 * @brief Runs the main UCI input loop reading commands from stdin and dispatching handlers.
 * @param pos Pointer to the board state.
 * @param info Pointer to the search info structure.
 */
void UciLoop(S_BOARD *pos, S_SEARCHINFO *info) {
    /* Disable stdout buffering so Arena receives uciok/bestmove immediately */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    char line[INPUTBUFFER];
    InitHashTable(HashTable, DEFAULT_HASH_MB);

    while (fgets(line, INPUTBUFFER, stdin) != NULL) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        if (strncmp(line, "isready", 7) == 0) {
            printf("readyok\n");
        } else if (strncmp(line, "position", 8) == 0) {
            ParsePosition(line, pos);
        } else if (strncmp(line, "ucinewgame", 10) == 0) {
            ClearHashTable(HashTable);
            ParsePosition("position startpos\n", pos);
        } else if (strncmp(line, "go", 2) == 0) {
            ParseGo(line, info, pos);
        } else if (strncmp(line, "setoption", 9) == 0) {
            ParseSetOption(line, pos);
        } else if (strncmp(line, "uci", 3) == 0) {
            printf("id name BadBishop\n");
            printf("id author BadBishop Contributors\n");
            printf("option name Hash type spin default %d min 1 max 96\n", DEFAULT_HASH_MB);
            printf("option name Personality type combo default Balanced var Balanced var "
                   "Positional var Sharp\n");
            printf("uciok\n");
        } else if (strncmp(line, "quit", 4) == 0) {
            info->quit = TRUE;
            break;
        }
    }
}