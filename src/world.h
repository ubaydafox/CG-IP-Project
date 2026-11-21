#pragma once
#include "state.h"

Building makeBuilding(float x, bool isFar);
void initWorld();
void resetGame();
void spawnDust(float x, float y, int n);
void spawnObstacle();
