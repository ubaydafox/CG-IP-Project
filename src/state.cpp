#include "state.h"
#include <cstdlib>

// ============================================================================
// Global variable definitions
// ============================================================================

// Day/night state
GameState gState = STATE_START;
DayPhase  gPhase = PHASE_DAWN;

// Current scene palette (recomputed every frame)
ColorRGB bgCol, inkCol;
float    nightF = 0.f;   // 0 = day, 1 = night

// Runner
float rY = GROUND_Y, rVY = 0.f, runAnim = 0.f;
bool  grounded = true, ducking = false;

// Progress
float gameSpeed = BASE_SPEED, score = 0.f, cycleT = 0.f;
int   highScore = 0, lastMile = 0;
float blinkTimer = 0.f;
float spawnAcc = 0.f, nextSpawn = 400.f;
bool  showHB = false;
int   lastTime = 0;

// World collections
std::vector<Obstacle> obstacles;
std::vector<Particle> particles;
std::vector<Cloud>    clouds;
std::vector<Dash>     dashes;
std::vector<Bump>     bumps;
std::vector<Star>     stars;
std::vector<Building> farB, nearB;

float frand(float a, float b) {
    return a + (b - a) * (rand() % 10000) / 10000.f;
}
