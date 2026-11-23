#pragma once
#include "state.h"

// ============================================================================
// World generation & scrolling
// ============================================================================
Building makeBuilding(float x, bool isFar);
void initWorld();
void spawnDust(float x, float y, int n);
void spawnObstacle();
void scrollWorld(float dt);
