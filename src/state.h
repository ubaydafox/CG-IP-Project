#pragma once

#include <vector>
#include "config.h"
#include "colors.h"

// ============================================================================
// World data — enums and structs
// ============================================================================
enum ObstType { OBS_CONE = 0, OBS_CONE2, OBS_BARRIER, OBS_BOLLARD, OBS_DRONE_HIGH, OBS_DRONE_LOW };
struct Obstacle  { ObstType type; float x, y, w, h, anim; };
struct Particle  { float x, y, vx, vy, life, decay, size; };
struct Cloud     { float x, y, s; };
struct Dash      { float x, y, w; };
struct Bump      { float x, w; };
struct Star      { float x, y, tw; };
struct Building  { float x, w, h; int cols, rows; bool lit[4][6]; };

enum GameState { STATE_START = 0, STATE_PLAYING, STATE_PAUSED, STATE_GAME_OVER };
enum DayPhase  { PHASE_DAWN = 0, PHASE_DAY, PHASE_DUSK, PHASE_NIGHT };

// ============================================================================
// Theme struct (defined here so state.h and sprites.h can both use it)
// ============================================================================
struct ThemeColors {
    const char* name;
    ColorRGB body;
    ColorRGB belly;
    ColorRGB eye;
};

// ============================================================================
// Global variable declarations (definitions live in .cpp files)
// ============================================================================

// Day/night state
extern GameState gState;
extern DayPhase  gPhase;

// Current scene palette (recomputed every frame)
extern ColorRGB bgCol, inkCol;
extern float    nightF;   // 0 = day, 1 = night

// Runner
extern float rY, rVY, runAnim;
extern bool  grounded, ducking;

// Progress
extern float gameSpeed, score, cycleT;
extern int   highScore, lastMile;
extern float blinkTimer;
extern float spawnAcc, nextSpawn;
extern bool  showHB;
extern int   lastTime;

// World collections
extern std::vector<Obstacle> obstacles;
extern std::vector<Particle> particles;
extern std::vector<Cloud>    clouds;
extern std::vector<Dash>     dashes;
extern std::vector<Bump>     bumps;
extern std::vector<Star>     stars;
extern std::vector<Building> farB, nearB;

// Utility
float frand(float a, float b);
