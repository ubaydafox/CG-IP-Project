#pragma once

#include "state.h"

// ============================================================================
// AABB collision detection & Game logic
// ============================================================================
struct RectAABB { float x, y, w, h; };

bool aabbCollide(const RectAABB& a, const RectAABB& b);
RectAABB runnerBox();
RectAABB obstBox(const Obstacle& o);
void resetGame();
void updateGame(float dt);
void timerCB(int val);

extern float jumpBuf;    // frames left in which an early jump press is remembered
extern bool  jumpHeld;   // is the jump key still held down?

extern float shakeT;                    // frames of screen shake left
const float SHAKE_FRAMES = 18.0f;


