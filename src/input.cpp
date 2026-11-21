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
#include <GL/glut.h>

// ============================================================================
// Input
// ============================================================================
void doJump() {
    if (gState == STATE_START)     { resetGame(); return; }
    if (gState == STATE_GAME_OVER) { resetGame(); return; }
    if (gState == STATE_PLAYING && grounded) { rVY = JUMP_FORCE; grounded = false; playSnd(sndJump); }
}

void startDuck() {
    if (gState != STATE_PLAYING) return;
    if (!ducking && grounded) playSnd(sndDuck);
    ducking = true;
}

void releaseJump() {
    if (!grounded && rVY > 4.f) rVY *= 0.55f;   // variable jump height
}

void keyDown(unsigned char k, int, int) {
    if (k == 27) exit(0);
    if (k == ' ') { doJump(); return; }
    if (k == 't' || k == 'T') {
        currentTheme = (currentTheme + 1) % THEME_COUNT;
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

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);   // 2D orthographic projection
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
