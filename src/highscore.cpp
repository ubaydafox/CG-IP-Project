#include <cstdio>
#include "highscore.h"
#include "state.h"

int prevBest = 0;
static const char* HS_FILE = "highscore.txt";

void loadHighScore() {
    FILE* f = fopen(HS_FILE, "r");
    if (!f) return;                                   // no file yet: keep 0
    int v = 0;
    if (fscanf(f, "%d", &v) == 1 && v > 0 && v < 10000000) highScore = v;
    fclose(f);
}

void saveHighScore() {
    FILE* f = fopen(HS_FILE, "w");
    if (!f) return;
    fprintf(f, "%d\n", highScore);
    fclose(f);
}
