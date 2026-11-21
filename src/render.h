#pragma once
#include "colors.h"
#include "state.h"
#include <string>

void renderBackground();
void renderGround();
void renderRunner();
void drawCone(float ox);
void renderObstacles();
void renderParticles();
RectAABB runnerBox();
RectAABB obstBox(const Obstacle& o);
void renderHitboxes();
void drawRestartIcon(float cx, float cy);
void renderHUD();
void display();
