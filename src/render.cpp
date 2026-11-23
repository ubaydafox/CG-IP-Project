#define _USE_MATH_DEFINES
#define NOMINMAX
#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "draw.h"
#include "world.h"
#include "daynight.h"
#include "audio.h"
#include "game.h"
#include "render.h"
#include "highscore.h"
#include "imgproc.h"
#include <GL/glut.h>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <string>

extern int gVpX, gVpY, gVpW, gVpH;

// ============================================================================
// Rendering
// ============================================================================
void renderSky() {
    glBegin(GL_QUADS);                          // vertical gradient, interpolated by OpenGL
    setColor(bgCol);     glVertex2f(0.f,   GROUND_Y); glVertex2f((float)WIN_W, GROUND_Y);
    setColor(skyTopCol); glVertex2f((float)WIN_W, (float)WIN_H); glVertex2f(0.f, (float)WIN_H);
    glEnd();
}

void renderBackground() {
    float t = fmodf(cycleT, CYCLE);
    renderSky();

    // ---- Sun with a soft glow ----
    {
        float span   = DAY_END + FADE;
        float sunVis = smooth01(t / 4.f) * (1.f - nightF);
        if (t < span && sunVis > 0.01f) {
            float p  = t / span;
            float sx = 80.f + 840.f * p;
            float sy = GROUND_Y + 60.f + sinf(p * (float)M_PI) * 230.f;

            // glow: concentric translucent discs (alpha blending)
            const float gr[4] = { 64.f, 48.f, 34.f, 24.f };
            for (int i = 0; i < 4; ++i) {
                glColor4f(1.00f, 0.82f, 0.45f, 0.07f * sunVis);
                fillCircle(sx, sy, gr[i], 32);
            }
            setColor(lerp(skyAt(sy), ColorRGB(0.99f, 0.80f, 0.40f), 0.75f * sunVis));
            fillCircle(sx, sy, 18.f, 28);
        }
    }

    // ---- Stars and moon (fade in with nightF) ----
    if (nightF > 0.01f) {
        ColorRGB light(0.86f, 0.86f, 0.88f);
        glBegin(GL_QUADS);
        for (const auto& s : stars) {
            float tw = 0.4f + 0.6f * (0.5f + 0.5f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.003f + s.tw));
            setColor(lerp(skyAt(s.y), light, nightF * tw * 0.8f));
            glVertex2f(s.x, s.y); glVertex2f(s.x + 2, s.y);
            glVertex2f(s.x + 2, s.y + 2); glVertex2f(s.x, s.y + 2);
        }
        glEnd();

        float p = (t - DAY_END) / (CYCLE - DAY_END);
        if (p < 0.f) p = 0.f;
        if (p > 1.f) p = 1.f;
        float mx = 80.f + 840.f * p;
        float my = GROUND_Y + 60.f + sinf(p * (float)M_PI) * 230.f;

        glColor4f(0.86f, 0.86f, 0.95f, 0.06f * nightF);          // faint moon glow
        fillCircle(mx, my, 38.f, 32);
        fillCircle(mx, my, 26.f, 32);

        setColor(lerp(skyAt(my), light, nightF)); fillCircle(mx, my, 15.f, 28);       // moon disc
        setColor(skyAt(my));                      fillCircle(mx + 7.f, my + 4.f, 13.f, 28); // crescent cut
    }

    // Clouds (flat, faint)
    setColor(tone(0.10f));
    for (const auto& c : clouds) {
        float s = c.s;
        fillRect(c.x, c.y, 70.f * s, 10.f * s);
        fillCircle(c.x + 18.f * s, c.y + 10.f * s, 12.f * s, 16);
        fillCircle(c.x + 38.f * s, c.y + 14.f * s, 16.f * s, 16);
        fillCircle(c.x + 56.f * s, c.y + 8.f * s,  10.f * s, 16);
    }

    // Far skyline (very faint) then near skyline (faint)
    setColor(tone(0.07f));
    for (const auto& b : farB) fillRect(b.x, GROUND_Y, b.w, b.h);
    setColor(tone(0.13f));
    for (const auto& b : nearB) fillRect(b.x, GROUND_Y, b.w, b.h);

    // Lit windows at night only
    if (nightF > 0.5f) {
        setColor(tone(0.40f));
        for (const auto& b : nearB)
            for (int c = 0; c < b.cols && c < 4; ++c)
                for (int r = 0; r < b.rows && r < 6; ++r) {
                    if (!b.lit[c][r]) continue;
                    float wx = b.x + (c + 1) * b.w / (b.cols + 1) - 2.f;
                    float wy = GROUND_Y + (r + 1) * b.h / (b.rows + 1) - 2.f;
                    fillRect(wx, wy, 4.f, 4.f);
                }
    }
}

void renderGround() {
    setColor(inkCol);
    fillRect(0, GROUND_Y - 1.f, WIN_W, 2.f);           // ground line

    glLineWidth(2.f);
    for (const auto& b : bumps) {                        // small road bumps
        glBegin(GL_LINE_STRIP);
        glVertex2f(b.x, GROUND_Y); glVertex2f(b.x + 4, GROUND_Y + 4);
        glVertex2f(b.x + b.w - 4, GROUND_Y + 4); glVertex2f(b.x + b.w, GROUND_Y);
        glEnd();
    }
    glLineWidth(1.f);

    setColor(tone(0.75f));
    for (const auto& d : dashes) fillRect(d.x, d.y, d.w, 2.f);
}

// Soft ellipse under the runner; smaller and fainter the higher the jump
static void drawShadow() {
    float h = rY - GROUND_Y; if (h < 0.f) h = 0.f;
    float k = 1.f - std::min(h / 160.f, 1.f) * 0.65f;
    bool  duck = ducking && grounded;
    float cx = rX + (duck ? 62.f : 66.f);
    float rx = (duck ? 46.f : 34.f) * k, ry = 4.f * k;
    glColor4f(0.f, 0.f, 0.f, 0.22f * k);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, GROUND_Y - 3.f);
    for (int i = 0; i <= 24; ++i) {
        float a = i * 2.f * (float)M_PI / 24.f;
        glVertex2f(cx + cosf(a) * rx, GROUND_Y - 3.f + sinf(a) * ry);
    }
    glEnd();
}

void renderRunner() {
    drawShadow();
    int frame = ((int)runAnim) % 2;
    if (gState == STATE_START) frame = 0;

    if (gState == STATE_GAME_OVER) {
        drawDinoSprite(DINO_CRASH, 37, rX - 16.f, rY, DINO_PX, curBodyCol, curBellyCol, curEyeCol);
    } else if (ducking && grounded) {
        drawDinoSprite(frame ? DINO_DUCK_B : DINO_DUCK_A, 19, rX, rY, DINO_PX, curBodyCol, curBellyCol, curEyeCol);
    } else if (!grounded) {
        drawDinoSprite(DINO_JUMP, 37, rX, rY, DINO_PX, curBodyCol, curBellyCol, curEyeCol);
    } else {
        drawDinoSprite(frame ? DINO_RUN_B : DINO_RUN_A, 37, rX, rY, DINO_PX, curBodyCol, curBellyCol, curEyeCol);
    }
}

// Traffic cone silhouette with two background-coloured reflective bands
void drawCone(float ox) {
    setColor(inkCol);
    fillRect(ox, 0, 26, 4);
    glBegin(GL_QUADS);
    glVertex2f(ox + 3, 4); glVertex2f(ox + 23, 4); glVertex2f(ox + 15, 44); glVertex2f(ox + 11, 44);
    glEnd();
    setColor(bgCol);
    glBegin(GL_QUADS);
    glVertex2f(ox + 5.8f, 18); glVertex2f(ox + 20.2f, 18); glVertex2f(ox + 18.8f, 25); glVertex2f(ox + 7.2f, 25);
    glVertex2f(ox + 8.6f, 32); glVertex2f(ox + 17.4f, 32); glVertex2f(ox + 16.6f, 36); glVertex2f(ox + 9.4f, 36);
    glEnd();
}

void renderObstacles() {
    for (const auto& o : obstacles) {
        glPushMatrix();
        glTranslatef(o.x, o.y, 0.f);
        switch (o.type) {
            case OBS_CONE:  drawCone(0);  break;
            case OBS_CONE2: drawCone(0); drawCone(28); break;
            case OBS_BARRIER: {
                setColor(inkCol);
                fillRect(6, 0, 5, 24);  fillRect(45, 0, 5, 24);   // legs
                fillRect(2, 0, 13, 3);  fillRect(41, 0, 13, 3);   // feet
                fillRect(0, 22, 56, 16);                          // plank
                fillRect(26, 38, 4, 4);                           // warning lamp
                setColor(bgCol);                                  // diagonal stripes
                glBegin(GL_QUADS);
                for (int i = 0; i < 4; ++i) {
                    float x0 = 4.f + i * 13.f;
                    float x1 = std::min(x0 + 12.f, 54.f);
                    glVertex2f(x0, 25); glVertex2f(x0 + 5, 25);
                    glVertex2f(x1, 35); glVertex2f(x1 - 5, 35);
                }
                glEnd();
                break;
            }
            case OBS_BOLLARD: {
                float hs[3] = { 38.f, 48.f, 38.f };
                for (int i = 0; i < 3; ++i) {
                    float bx = i * 16.f, h = hs[i];
                    setColor(inkCol);
                    fillRect(bx, 0, 12, h - 6);
                    fillCircle(bx + 6, h - 6, 6, 14);
                    setColor(bgCol);
                    fillRect(bx, h - 20, 12, 4);
                }
                break;
            }
            case OBS_DRONE_HIGH:
            case OBS_DRONE_LOW: {
                setColor(inkCol);
                fillRect(10, 6, 34, 10);          // body
                fillCircle(27, 16, 7, 16);        // dome
                fillRect(0, 16, 54, 3);           // arms
                fillRect(14, 0, 3, 6); fillRect(37, 0, 3, 6); // legs
                float rw = 4.f + 14.f * fabsf(cosf(o.anim * 20.f)); // spinning rotors
                fillRect(5 - rw * 0.5f, 21, rw, 2); fillRect(49 - rw * 0.5f, 21, rw, 2);
                setColor(bgCol);
                fillRect(32, 10, 5, 3);           // camera lens
                if (nightF > 0.05f) {                 // glowing lens at night
                    glColor4f(1.f, 0.30f, 0.25f, 0.20f * nightF); fillCircle(34.5f, 11.5f, 12.f, 20);
                    glColor4f(1.f, 0.45f, 0.35f, 0.90f * nightF); fillCircle(34.5f, 11.5f, 2.5f, 12);
                }
                break;
            }
        }
        glPopMatrix();
    }
}

void renderParticles() {
    glBegin(GL_QUADS);
    for (const auto& p : particles) {
        setColor(tone(p.life * 0.8f));
        glVertex2f(p.x, p.y); glVertex2f(p.x + p.size, p.y);
        glVertex2f(p.x + p.size, p.y + p.size); glVertex2f(p.x, p.y + p.size);
    }
    glEnd();
}

void renderHitboxes() {
    if (!showHB) return;
    glLineWidth(2.f);
    glColor3f(0.1f, 0.8f, 0.2f);
    RectAABB r = runnerBox(); outlineRect(r.x, r.y, r.w, r.h);
    glColor3f(0.9f, 0.15f, 0.15f);
    for (const auto& o : obstacles) { RectAABB b = obstBox(o); outlineRect(b.x, b.y, b.w, b.h); }
    glLineWidth(1.f);
}

// ---- 5x7 pixel font, only the letters the titles need ----
struct Glyph { char ch; const char* rows[7]; };
static const Glyph FONT[] = {
    {'A', {".###.","#...#","#...#","#####","#...#","#...#","#...#"}},
    {'C', {".###.","#...#","#....","#....","#....","#...#",".###."}},
    {'D', {"####.","#...#","#...#","#...#","#...#","#...#","####."}},
    {'E', {"#####","#....","#....","####.","#....","#....","#####"}},
    {'G', {".###.","#...#","#....","#.###","#...#","#...#",".###."}},
    {'H', {"#...#","#...#","#...#","#####","#...#","#...#","#...#"}},
    {'M', {"#...#","##.##","#.#.#","#.#.#","#...#","#...#","#...#"}},
    {'N', {"#...#","##..#","##..#","#.#.#","#..##","#..##","#...#"}},
    {'O', {".###.","#...#","#...#","#...#","#...#","#...#",".###."}},
    {'P', {"####.","#...#","#...#","####.","#....","#....","#...."}},
    {'R', {"####.","#...#","#...#","####.","#.#..","#..#.","#...#"}},
    {'S', {".####","#....","#....",".###.","....#","....#","####."}},
    {'U', {"#...#","#...#","#...#","#...#","#...#","#...#",".###."}},
    {'V', {"#...#","#...#","#...#","#...#","#...#",".#.#.","..#.."}},
    {':', {".....","..#..","..#..",".....","..#..","..#..","....."}},
};

static void drawPixelText(float x, float y, const std::string& s, float sc) {
    float cx = x;
    for (char c : s) {
        for (const Glyph& g : FONT) {
            if (g.ch != c) continue;
            for (int r = 0; r < 7; ++r)
                for (int col = 0; col < 5; ++col)
                    if (g.rows[r][col] == '#') fillRect(cx + col * sc, y + (6 - r) * sc, sc, sc);
            break;
        }
        cx += 6.f * sc;                                   // a space just advances
    }
}
static void drawPixelTextC(float cx, float y, const std::string& s, float sc) {
    float w = s.empty() ? 0.f : s.size() * 6.f * sc - sc;
    drawPixelText(cx - w * 0.5f, y, s, sc);
}

// Translucent panel so text stays readable over the skyline
static void drawPanel(float cx, float cy, float w, float h) {
    glColor4f(bgCol.r, bgCol.g, bgCol.b, 0.90f);
    fillRect(cx - w * 0.5f, cy - h * 0.5f, w, h);
    setColor(tone(0.25f));
    glLineWidth(2.f);
    outlineRect(cx - w * 0.5f, cy - h * 0.5f, w, h);
    glLineWidth(1.f);
}

void drawRestartIcon(float cx, float cy) {
    const float R = 14.f, D2R = (float)M_PI / 180.f;
    setColor(inkCol);
    glLineWidth(3.f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; ++i) {
        float a = (40.f + i * (280.f / 24.f)) * D2R;
        glVertex2f(cx + cosf(a) * R, cy + sinf(a) * R);
    }
    glEnd();
    glLineWidth(1.f);

    // arrowhead at the arc start, pointing along the clockwise tangent
    float a  = 40.f * D2R;
    float ex = cx + cosf(a) * R, ey = cy + sinf(a) * R;
    float tx = sinf(a), ty = -cosf(a);          // tangent direction
    float rx = cosf(a), ry = sinf(a);           // radial direction
    glBegin(GL_TRIANGLES);
    glVertex2f(ex + tx * 9.f, ey + ty * 9.f);
    glVertex2f(ex + rx * 7.f, ey + ry * 7.f);
    glVertex2f(ex - rx * 7.f, ey - ry * 7.f);
    glEnd();
}

void renderHUD() {
    void* mono = GLUT_BITMAP_9_BY_15;

    // Score (top-right, Chrome style). Current score blinks at every 100 pts.
    std::stringstream hi, cur;
    hi  << "HI " << std::setfill('0') << std::setw(5) << highScore;
    cur << std::setfill('0') << std::setw(5) << (int)score;
    float curW = strW(cur.str(), mono);
    setColor(tone(0.70f));
    drawStr(WIN_W - 30.f - curW - 20.f - strW(hi.str(), mono), WIN_H - 40.f, hi.str(), mono);
    bool hideCur = blinkTimer > 0.f && fmodf(blinkTimer, 0.3f) < 0.15f;
    if (!hideCur) { setColor(inkCol); drawStr(WIN_W - 30.f - curW, WIN_H - 40.f, cur.str(), mono); }

    // Time-of-day indicator (top-left)
    const char* names[4] = { "DAWN", "DAY", "DUSK", "NIGHT" };
    setColor(tone(0.60f));
    drawStr(30.f, WIN_H - 40.f, names[gPhase], mono);
    if (showHB) drawStr(100.f, WIN_H - 40.f, "[HITBOX]", mono);

    // Help line: always on pause / game over, and only for the first seconds of a run
    bool showHelp = (gState == STATE_PAUSED || gState == STATE_GAME_OVER ||
                     (gState == STATE_PLAYING && score < 60.f));
    if (showHelp) {
        setColor(tone(0.78f));
        drawStrC(WIN_W * 0.5f, 14.f,
                 std::string("SPACE jump | DOWN duck | T theme: ") + THEMES[currentTheme].name +
                 " | F filter | N time | S shot | P pause | H hitbox | M sound: " + (soundOn ? "on" : "off"),
                 GLUT_BITMAP_8_BY_13);
    }

    if (gState == STATE_START) {
        drawPanel(WIN_W * 0.5f, 256.f, 560.f, 152.f);
        setColor(inkCol);
        drawPixelTextC(WIN_W * 0.5f, 292.f, "CHRONO RUN : DN DASH", 4.f);
        setColor(tone(0.85f));
        drawStrC(WIN_W * 0.5f, 262.f, "Press SPACE to start", mono);
        setColor(tone(0.65f));
        drawStrC(WIN_W * 0.5f, 240.f, "Jump over cones, barriers & bollards  -  Duck under high drones", GLUT_BITMAP_8_BY_13);
        setColor(curBodyCol);
        drawStrC(WIN_W * 0.5f, 216.f, std::string("Dino Theme [T]: ") + THEMES[currentTheme].name, mono);

        // theme swatches, current one outlined
        float sx = WIN_W * 0.5f - THEME_COUNT * 22.f * 0.5f + 3.f;
        for (int i = 0; i < THEME_COUNT; ++i) {
            float x = sx + i * 22.f;
            setColor(THEMES[i].belly); fillRect(x, 191.f, 16.f, 5.f);
            setColor(THEMES[i].body);  fillRect(x, 196.f, 16.f, 9.f);
            if (i == currentTheme) { setColor(inkCol); outlineRect(x - 2.f, 189.f, 20.f, 18.f); }
        }

        setColor(tone(0.60f));
        drawStrC(WIN_W * 0.5f, 50.f, "Nahdiul Islam Rony (231-115-069)  &  Abu Ubayda (231-115-080)", GLUT_BITMAP_8_BY_13);
        drawStrC(WIN_W * 0.5f, 32.f, "CSE-402 CG & IP Lab  |  Lecturer: Shrabanti Chowdhury  |  Section B", GLUT_BITMAP_8_BY_13);
    } else if (gState == STATE_PAUSED) {
        drawPanel(WIN_W * 0.5f, 277.f, 340.f, 82.f);
        setColor(inkCol);
        drawPixelTextC(WIN_W * 0.5f, 272.f, "PAUSED", 5.f);
        setColor(tone(0.70f));
        drawStrC(WIN_W * 0.5f, 246.f, "Press P to resume", mono);
    } else if (gState == STATE_GAME_OVER) {
        drawPanel(WIN_W * 0.5f, 261.f, 440.f, 150.f);
        setColor(inkCol);
        drawPixelTextC(WIN_W * 0.5f, 292.f, "GAME OVER", 5.f);
        drawRestartIcon(WIN_W * 0.5f, 262.f);
        setColor(tone(0.70f));
        drawStrC(WIN_W * 0.5f, 222.f, "Press SPACE or R to restart", mono);
        if ((int)score > prevBest && score > 5.f) {
            setColor(inkCol);
            drawStrC(WIN_W * 0.5f, 198.f, "NEW HIGH SCORE!", mono);
        }
    }
}

void display() {
    updatePalette();

    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.05f, 0.05f, 0.07f, 1.f);       // dark letterbox bars
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(gVpX, gVpY, gVpW, gVpH);
    glClearColor(bgCol.r, bgCol.g, bgCol.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();                               // shake moves the world, not the HUD
    if (shakeT > 0.f) {
        float m = 7.f * std::min(shakeT / SHAKE_FRAMES, 1.f);
        glTranslatef(((rand() % 2001) / 1000.f - 1.f) * m, ((rand() % 2001) / 1000.f - 1.f) * m, 0.f);
    }
    renderBackground();
    renderGround();
    renderObstacles();
    renderRunner();
    renderParticles();
    renderHitboxes();
    glPopMatrix();

    if (shakeT > 0.f) {                           // white impact flash
        glColor4f(1.f, 1.f, 1.f, 0.30f * std::min(shakeT / SHAKE_FRAMES, 1.f));
        fillRect(0, 0, (float)WIN_W, (float)WIN_H);
    }

    renderHUD();
    applyPostFilter();

    glDisable(GL_SCISSOR_TEST);
    glutSwapBuffers();
}
