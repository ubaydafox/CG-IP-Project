/*
 ============================================================================
  Project Title : CHRONO RUN: DN DASH  (v3.0 - Minimal City Edition)
  Course        : Computer Graphics & Image Processing Lab (CSE-402)
  Institution   : Metropolitan University, Sylhet
  Lecturer      : Shrabanti Chowdhury

  Submitted by  :
    1. Nahdiul Islam Rony (ID: 231-115-069)
    2. Abu Ubayda         (ID: 231-115-080)
  Section       : B, 58th Batch, Dept. of CSE

  Description   :
    A clean, minimal 2D endless runner built with C++, OpenGL and FreeGLUT.
    Visual style is inspired by the Chrome offline runner: a high-contrast
    pixel-art character on a light background, a thin ground line, faint
    clouds and a faint modern city skyline.

    Graphics concepts demonstrated:
      - 2D orthographic projection (gluOrtho2D)
      - Raster pixel-art sprite rendering using GL_QUADS (bitmap -> quads)
      - Frame-based sprite animation (run cycle, jump, duck)
      - Linear colour interpolation (lerp) for the day-night cycle
      - Colour inversion at night (light ink on dark sky) for contrast
      - Multi-layer parallax scrolling (skyline far/near, clouds, ground)
      - Delta-time (frame-rate independent) physics and movement
      - AABB collision detection with live hitbox debugger [H]
 ============================================================================
*/

#define _USE_MATH_DEFINES
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>
#include <iostream>

#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "audio.h"
#include "draw.h"
#include "world.h"
#include "daynight.h"
#include "render.h"
#include "game.h"
#include "input.h"

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Chrono Run: DN Dash  |  Made by - Ubayda & Rony");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutIgnoreKeyRepeat(1);

    initWorld();
    initSounds();
    lastTime = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseCB);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc(FRAME_MS, timerCB, 0);

    std::cout << "CHRONO RUN: DN DASH v3.0 - CSE-402 CG & IP Lab\n"
              << "Nahdiul Islam Rony (231-115-069) & Abu Ubayda (231-115-080)\n"
              << "Controls: SPACE/UP jump | DOWN duck | P pause | R restart | H hitbox | M sound | ESC quit\n"
              << "Mouse:    left click jump | right click (hold) duck\n";

    glutMainLoop();
    return 0;
}
