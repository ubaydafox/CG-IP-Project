#pragma once

// ============================================================================
// Input & Window handling
// ============================================================================
extern int gVpX, gVpY, gVpW, gVpH;

void doJump();
void startDuck();
void releaseJump();
void keyDown(unsigned char k, int x, int y);
void keyUp(unsigned char k, int x, int y);
void specialDown(int k, int x, int y);
void specialUp(int k, int x, int y);
void mouseCB(int button, int st, int x, int y);
void reshape(int w, int h);
