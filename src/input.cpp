#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "audio.h"
#include "render.h"
#include "world.h"
#include "game.h"
#include "input.h"
#include "highscore.h"
#include "imgproc.h"
#include <GL/glut.h>
#include <cstdlib>

// ============================================================================
// Input
// ============================================================================
void doJump() {
    jumpHeld = true;
    if (gState == STATE_START)     { resetGame(); return; }
    if (gState == STATE_GAME_OVER) { resetGame(); return; }
    if (gState == STATE_PLAYING) {
        if (grounded) { rVY = JUMP_FORCE; grounded = false; playSnd(sndJump); }
        else          jumpBuf = 6.f;                // remember the press for ~0.1 s
    }
}

void startDuck() {
    if (gState != STATE_PLAYING) return;
    if (!ducking && grounded) playSnd(sndDuck);
    ducking = true;
}

void releaseJump() {
    jumpHeld = false;
    if (!grounded && rVY > 4.f) rVY *= 0.55f;       // variable jump height
}

void keyDown(unsigned char k, int, int) {
    if (k == 27) { saveHighScore(); exit(0); }
    if (k == ' ') { doJump(); return; }
    if (k == 't' || k == 'T') {
        currentTheme = (currentTheme + 1) % THEME_COUNT;
        return;
    }
    if (k == 'f' || k == 'F') {
        cycleFilter();
        glutPostRedisplay();
        return;
    }
    if (k == 's' || k == 'S') {
        saveScreenshot();
        glutPostRedisplay();
        return;
    }
    if (k == 'p' || k == 'P') {
        if (gState == STATE_PLAYING) gState = STATE_PAUSED;
        else if (gState == STATE_PAUSED) gState = STATE_PLAYING;
        return;
    }
    if (k == 'r' || k == 'R') { resetGame(); return; }
    if (k == 'h' || k == 'H') { showHB = !showHB; return; }
    if (k == 'm' || k == 'M') { soundOn = !soundOn; if (!soundOn) PlaySoundA(NULL, NULL, 0); return; }
}

void keyUp(unsigned char k, int, int) {
    if (k == ' ') releaseJump();
}

void specialDown(int k, int, int) {
    if (k == GLUT_KEY_UP) doJump();
    else if (k == GLUT_KEY_DOWN) startDuck();
}

void specialUp(int k, int, int) {
    if (k == GLUT_KEY_DOWN) ducking = false;
    if (k == GLUT_KEY_UP) releaseJump();
}

// Mouse / touchscreen laptop: left click = jump, right click (hold) = duck
void mouseCB(int button, int st, int, int) {
    if (button == GLUT_LEFT_BUTTON) {
        if (st == GLUT_DOWN) doJump(); else releaseJump();
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (st == GLUT_DOWN) startDuck(); else ducking = false;
    }
}

// Game viewport (the 1000x450 area), centered inside the window
int gVpX = 0, gVpY = 0, gVpW = WIN_W, gVpH = WIN_H;

void reshape(int w, int h) {
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    float target = (float)WIN_W / (float)WIN_H;
    int vw = w, vh = (int)(w / target);
    if (vh > h) { vh = h; vw = (int)(h * target); }   // fit inside the window
    gVpX = (w - vw) / 2;  gVpY = (h - vh) / 2;
    gVpW = vw;            gVpH = vh;

    glViewport(gVpX, gVpY, gVpW, gVpH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);   // 2D orthographic projection
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

