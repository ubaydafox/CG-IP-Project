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
#include <mmsystem.h>     // PlaySound (link with -lwinmm)
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

// ============================================================================
// Configuration
// ============================================================================
const int   WIN_W    = 1000;
const int   WIN_H    = 450;
const float GROUND_Y = 110.0f;
const int   FRAME_MS = 16;          // ~60 FPS timer
const float PX       = 4.0f;        // size of one sprite "pixel"

// Speed (pixels per 60fps-frame). Calm start, gentle ramp.
const float BASE_SPEED = 4.0f;
const float MAX_SPEED  = 8.0f;
const float SPEED_RAMP = 0.0008f;   // speed = BASE + score * RAMP (max at 5000 pts)

// Jump physics (floaty and forgiving)
const float JUMP_FORCE = 12.5f;
const float GRAVITY    = 0.55f;

// Day-night cycle timing (seconds) - long, calm phases
const float CYCLE     = 150.0f;     // full cycle length
const float DAY_END   = 90.0f;      // 0..90   : dawn -> day -> dusk
const float FADE      = 4.0f;       // 90..94  : fade to night
const float NIGHT_END = 146.0f;     // 94..146 : night, 146..150 : fade to dawn

// ============================================================================
// Colour helpers
// ============================================================================
struct ColorRGB {
    float r, g, b;
    ColorRGB(float r = 0.f, float g = 0.f, float b = 0.f) : r(r), g(g), b(b) {}
};

// Linear interpolation: C(t) = C1 + (C2 - C1) * t
ColorRGB lerp(const ColorRGB& a, const ColorRGB& b, float t) {
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return ColorRGB(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t);
}
float smooth01(float t) { if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f; return t * t * (3.f - 2.f * t); }
void  setColor(const ColorRGB& c) { glColor3f(c.r, c.g, c.b); }

// ============================================================================
// AABB
// ============================================================================
struct RectAABB { float x, y, w, h; };
bool aabbCollide(const RectAABB& a, const RectAABB& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

// ============================================================================
// Pixel-art sprites ('#' = filled pixel). Row 0 is the TOP of the sprite.
// ============================================================================
const char* RUN_A[18] = {
    ".......####...",
    "......######..",
    "......####.#..",
    "......######..",
    ".......####...",
    "........##....",
    "......#####...",
    ".....#######..",
    "....##.###.##.",
    "...##..###..##",
    "..##...###....",
    ".......###....",
    "......#####...",
    ".....##...##..",
    "....##.....##.",
    "...##.......##",
    "..##........##",
    ".###........##",
};
const char* RUN_B[18] = {
    ".......####...",
    "......######..",
    "......####.#..",
    "......######..",
    ".......####...",
    "........##....",
    "......#####...",
    ".....#######..",
    ".....#.###.#..",
    ".....#.###.#..",
    ".....#.###.#..",
    ".......###....",
    ".......####...",
    ".......##.##..",
    ".......##..##.",
    ".......##.#...",
    ".......##.....",
    ".......###....",
};
const char* JUMP_S[18] = {
    ".......####...",
    "......######..",
    "......####.#..",
    "......######..",
    ".......####...",
    "........##....",
    "......#####...",
    ".....#######..",
    "....##.###.##.",
    "...##..###..##",
    ".......###....",
    ".......###....",
    "......#####...",
    ".....##..###..",
    "....##.....##.",
    "...##.....##..",
    "..............",
    "..............",
};
const char* DUCK_A[10] = {
    "............####..",
    "...........######.",
    "...........####.#.",
    "..##########.####.",
    ".############.....",
    "##..########......",
    "......##..##......",
    ".....##....##.....",
    "....##......##....",
    "...###......###...",
};
const char* DUCK_B[10] = {
    "............####..",
    "...........######.",
    "...........####.#.",
    "..##########.####.",
    ".############.....",
    "##..########......",
    "......##.##.......",
    "......##.##.......",
    "......##..##......",
    ".....###..###.....",
};

// ============================================================================
// World data
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

GameState gState = STATE_START;
DayPhase  gPhase = PHASE_DAWN;

// Current scene palette (recomputed every frame)
ColorRGB bgCol, inkCol;
float    nightF = 0.f;   // 0 = day, 1 = night

// Runner
const float rX = 100.f;
float rY = GROUND_Y, rVY = 0.f, runAnim = 0.f;
bool  grounded = true, ducking = false;

// Progress
float gameSpeed = BASE_SPEED, score = 0.f, cycleT = 0.f;
int   highScore = 0, lastMile = 0;
float blinkTimer = 0.f;
float spawnAcc = 0.f, nextSpawn = 400.f;
bool  showHB = false;
int   lastTime = 0;

std::vector<Obstacle> obstacles;
std::vector<Particle> particles;
std::vector<Cloud>    clouds;
std::vector<Dash>     dashes;
std::vector<Bump>     bumps;
std::vector<Star>     stars;
std::vector<Building> farB, nearB;

float frand(float a, float b) { return a + (b - a) * (rand() % 10000) / 10000.f; }

// ============================================================================
// Gentle sound effects
// Soft sine tones are synthesised into in-memory WAV buffers at startup and
// played asynchronously with PlaySound. Each tone has a short fade-in and a
// smooth fade-out, so nothing clicks or beeps harshly. Only one sound plays at
// a time (a new sound replaces the old), so effects never pile up.
// ============================================================================
struct Tone { float f0, f1, dur, delay, vol; };
std::vector<char> sndJump, sndDuck, sndScore, sndOver;
bool soundOn = true;

std::vector<char> synthWav(const std::vector<Tone>& tones) {
    const int SR = 22050;
    float total = 0.f;
    for (const auto& t : tones) total = std::max(total, t.delay + t.dur);
    int N = (int)(total * SR) + 64;
    std::vector<float> mix(N, 0.f);

    for (const auto& t : tones) {
        int s0 = (int)(t.delay * SR), len = (int)(t.dur * SR);
        double ph = 0.0;
        for (int i = 0; i < len && s0 + i < N; ++i) {
            float u   = (float)i / len;
            float f   = t.f0 * powf(t.f1 / t.f0, u);            // smooth pitch glide
            ph       += 2.0 * M_PI * f / SR;
            float att = std::min(1.f, i / (0.012f * SR));        // 12 ms fade-in
            float env = att * (1.f - u) * (1.f - u);             // soft fade-out
            mix[s0 + i] += (float)(sin(ph) * 0.85 + sin(2.0 * ph) * 0.15) * env * t.vol;
        }
    }

    std::vector<char> wav(44 + N * 2);
    auto w32 = [&](int off, uint32_t v) { memcpy(&wav[off], &v, 4); };
    auto w16 = [&](int off, uint16_t v) { memcpy(&wav[off], &v, 2); };
    memcpy(&wav[0], "RIFF", 4); w32(4, 36 + N * 2); memcpy(&wav[8], "WAVEfmt ", 8);
    w32(16, 16); w16(20, 1); w16(22, 1); w32(24, SR); w32(28, SR * 2); w16(32, 2); w16(34, 16);
    memcpy(&wav[36], "data", 4); w32(40, N * 2);
    for (int i = 0; i < N; ++i) {
        float s = std::max(-1.f, std::min(1.f, mix[i]));
        int16_t v = (int16_t)(s * 32767.f);
        memcpy(&wav[44 + i * 2], &v, 2);
    }
    return wav;
}

void initSounds() {
    sndJump  = synthWav({ { 520.f, 720.f, 0.11f, 0.00f, 0.28f } });
    sndDuck  = synthWav({ { 320.f, 240.f, 0.07f, 0.00f, 0.18f } });
    sndScore = synthWav({ { 660.f, 660.f, 0.14f, 0.00f, 0.22f },
                          { 880.f, 880.f, 0.24f, 0.11f, 0.22f } });
    sndOver  = synthWav({ { 440.f, 330.f, 0.25f, 0.00f, 0.26f },
                          { 330.f, 247.f, 0.40f, 0.20f, 0.24f } });
}

void playSnd(const std::vector<char>& w) {
    if (soundOn && !w.empty())
        PlaySoundA(w.data(), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

// ============================================================================
// Drawing primitives
// ============================================================================
void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}
void outlineRect(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}
void fillCircle(float cx, float cy, float r, int seg = 20) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; ++i) {
        float a = i * 2.f * (float)M_PI / seg;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}
void drawStr(float x, float y, const std::string& s, void* f) {
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(f, c);
}
float strW(const std::string& s, void* f) {
    return (float)glutBitmapLength(f, (const unsigned char*)s.c_str());
}
void drawStrC(float cx, float y, const std::string& s, void* f) { drawStr(cx - strW(s, f) * 0.5f, y, s, f); }

// Raster sprite -> quads. (x, y) = bottom-left corner.
void drawSprite(const char* const* rows, int n, float x, float y) {
    glBegin(GL_QUADS);
    for (int r = 0; r < n; ++r) {
        const char* row = rows[r];
        int len = (int)strlen(row);
        float yy = y + (n - 1 - r) * PX;
        for (int c = 0; c < len; ++c) {
            if (row[c] != '#') continue;
            float xx = x + c * PX;
            glVertex2f(xx, yy); glVertex2f(xx + PX, yy);
            glVertex2f(xx + PX, yy + PX); glVertex2f(xx, yy + PX);
        }
    }
    glEnd();
}

// ============================================================================
// World generation
// ============================================================================
Building makeBuilding(float x, bool isFar) {
    Building b;
    b.x = x;
    b.w = isFar ? frand(40, 110) : frand(30, 80);
    b.h = isFar ? frand(50, 150) : frand(30, 90);
    b.cols = 2 + rand() % 3;
    b.rows = 3 + rand() % 4;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 6; ++r) b.lit[c][r] = (rand() % 3 == 0);
    return b;
}

void initWorld() {
    srand((unsigned)time(0));

    farB.clear(); nearB.clear();
    for (float x = 0; x < WIN_W + 200; ) { Building b = makeBuilding(x, true);  farB.push_back(b);  x += b.w + frand(4, 30); }
    for (float x = 0; x < WIN_W + 200; ) { Building b = makeBuilding(x, false); nearB.push_back(b); x += b.w + frand(20, 90); }

    clouds.clear();
    for (int i = 0; i < 4; ++i) clouds.push_back({ frand(0, WIN_W), frand(250, 360), frand(0.7f, 1.2f) });

    dashes.clear();
    for (int i = 0; i < 40; ++i) dashes.push_back({ frand(0, WIN_W), frand(GROUND_Y - 30, GROUND_Y - 6), frand(2, 10) });

    bumps.clear();
    for (int i = 0; i < 4; ++i) bumps.push_back({ frand(0, WIN_W), frand(14, 30) });

    stars.clear();
    for (int i = 0; i < 45; ++i) stars.push_back({ frand(0, WIN_W), frand(240, WIN_H - 50), frand(0, 10) });
}

void resetGame() {
    obstacles.clear(); particles.clear();
    rY = GROUND_Y; rVY = 0.f; grounded = true; ducking = false; runAnim = 0.f;
    gameSpeed = BASE_SPEED; score = 0.f; cycleT = 0.f;
    lastMile = 0; blinkTimer = 0.f;
    spawnAcc = 0.f; nextSpawn = 400.f;
    gState = STATE_PLAYING;
}

void spawnDust(float x, float y, int n) {
    for (int i = 0; i < n; ++i)
        particles.push_back({ x + frand(-6, 6), y + frand(0, 4), frand(-2.5f, -0.5f), frand(0.3f, 1.8f),
                              1.f, frand(0.03f, 0.06f), frand(2, 4) });
}

void spawnObstacle() {
    Obstacle o; o.x = WIN_W + 40.f; o.anim = 0.f;
    if (score > 150.f && rand() % 100 < 25) {
        if (rand() % 2 == 0) { o.type = OBS_DRONE_HIGH; o.w = 54; o.h = 24; o.y = GROUND_Y + 50; } // duck under
        else                 { o.type = OBS_DRONE_LOW;  o.w = 54; o.h = 24; o.y = GROUND_Y + 20; } // jump over
    } else {
        o.y = GROUND_Y;
        switch (rand() % 4) {
            case 0:  o.type = OBS_CONE;    o.w = 26; o.h = 44; break;
            case 1:  o.type = OBS_CONE2;   o.w = 54; o.h = 44; break;
            case 2:  o.type = OBS_BARRIER; o.w = 56; o.h = 40; break;
            default: o.type = OBS_BOLLARD; o.w = 44; o.h = 48; break;
        }
    }
    obstacles.push_back(o);
}

// ============================================================================
// Day-night palette (Chrome-style: dark ink by day, inverted at night)
// ============================================================================
void updatePalette() {
    float t = fmodf(cycleT, CYCLE);
    ColorRGB dawnBg(0.98f, 0.94f, 0.90f), dayBg(0.97f, 0.97f, 0.97f),
             duskBg(0.98f, 0.90f, 0.84f), nightBg(0.11f, 0.12f, 0.16f);

    ColorRGB dBg;
    if (t >= NIGHT_END)  dBg = dawnBg;
    else if (t < 20.f)   dBg = lerp(dawnBg, dayBg, t / 20.f);
    else if (t < 70.f)   dBg = dayBg;
    else                 dBg = lerp(dayBg, duskBg, (t - 70.f) / 20.f);

    if (t < DAY_END)              nightF = 0.f;
    else if (t < DAY_END + FADE)  nightF = smooth01((t - DAY_END) / FADE);
    else if (t < NIGHT_END)       nightF = 1.f;
    else                          nightF = 1.f - smooth01((t - NIGHT_END) / FADE);

    bgCol  = lerp(dBg, nightBg, nightF);
    // Ink flips at mid-fade so the runner always contrasts with the sky
    inkCol = (nightF < 0.5f) ? ColorRGB(0.33f, 0.33f, 0.33f) : ColorRGB(0.86f, 0.86f, 0.88f);

    if      (t < 20.f)       gPhase = PHASE_DAWN;
    else if (t < 70.f)       gPhase = PHASE_DAY;
    else if (t < DAY_END + FADE * 0.5f) gPhase = PHASE_DUSK;
    else if (t < NIGHT_END + FADE * 0.5f) gPhase = PHASE_NIGHT;
    else                     gPhase = PHASE_DAWN;
}

// Mix between background and ink: 0 = invisible, 1 = full ink
ColorRGB tone(float k) { return lerp(bgCol, inkCol, k); }

// ============================================================================
// Rendering
// ============================================================================
void renderBackground() {
    float t = fmodf(cycleT, CYCLE);

    // Sun (day) - soft pale gold disc on an arc
    if (nightF < 0.5f) {
        float p = t < DAY_END ? t / DAY_END : 1.f;
        float sx = 80.f + 840.f * p, sy = GROUND_Y + 60.f + sinf(p * (float)M_PI) * 230.f;
        setColor(lerp(bgCol, ColorRGB(0.99f, 0.80f, 0.40f), 0.55f));
        fillCircle(sx, sy, 18.f, 28);
    } else {
        // Stars
        glBegin(GL_QUADS);
        for (const auto& s : stars) {
            float tw = 0.4f + 0.6f * (0.5f + 0.5f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.003f + s.tw));
            setColor(tone(tw * 0.8f));
            glVertex2f(s.x, s.y); glVertex2f(s.x + 2, s.y); glVertex2f(s.x + 2, s.y + 2); glVertex2f(s.x, s.y + 2);
        }
        glEnd();
        // Crescent moon
        float p = (t - DAY_END) / (NIGHT_END + FADE - DAY_END);
        float mx = 80.f + 840.f * p, my = GROUND_Y + 60.f + sinf(p * (float)M_PI) * 230.f;
        setColor(tone(0.9f)); fillCircle(mx, my, 15.f, 28);
        setColor(bgCol);      fillCircle(mx + 7.f, my + 4.f, 13.f, 28);
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

void renderRunner() {
    setColor(inkCol);
    int frame = ((int)runAnim) % 2;
    if (gState == STATE_START) frame = 0;
    if (ducking && grounded)  drawSprite(frame ? DUCK_B : DUCK_A, 10, rX, rY);
    else if (!grounded)       drawSprite(JUMP_S, 18, rX, rY);
    else                      drawSprite(frame ? RUN_B : RUN_A, 18, rX, rY);
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

RectAABB runnerBox() {
    if (ducking && grounded) return { rX + 4.f,  rY, 64.f, 38.f };
    return                          { rX + 12.f, rY, 36.f, 68.f };
}
RectAABB obstBox(const Obstacle& o) { return { o.x + 4.f, o.y, o.w - 8.f, o.h - 2.f }; }

void renderHitboxes() {
    if (!showHB) return;
    glLineWidth(2.f);
    glColor3f(0.1f, 0.8f, 0.2f);
    RectAABB r = runnerBox(); outlineRect(r.x, r.y, r.w, r.h);
    glColor3f(0.9f, 0.15f, 0.15f);
    for (const auto& o : obstacles) { RectAABB b = obstBox(o); outlineRect(b.x, b.y, b.w, b.h); }
    glLineWidth(1.f);
}

void drawRestartIcon(float cx, float cy) {
    setColor(inkCol);
    glLineWidth(3.f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; ++i) {
        float a = (40.f + i * (280.f / 24.f)) * (float)M_PI / 180.f;
        glVertex2f(cx + cosf(a) * 14.f, cy + sinf(a) * 14.f);
    }
    glEnd();
    glLineWidth(1.f);
    float ex = cx + cosf(40.f * (float)M_PI / 180.f) * 14.f;
    float ey = cy + sinf(40.f * (float)M_PI / 180.f) * 14.f;
    glBegin(GL_TRIANGLES);
    glVertex2f(ex - 7, ey + 2); glVertex2f(ex + 5, ey + 6); glVertex2f(ex + 3, ey - 7);
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

    // Bottom help line
    setColor(tone(0.50f));
    if (gState != STATE_START)
        drawStrC(WIN_W * 0.5f, 14.f, std::string("SPACE/UP jump   DOWN duck   P pause   R restart   H hitbox   M sound: ")
                 + (soundOn ? "on" : "off") + "   ESC quit",
                 GLUT_BITMAP_8_BY_13);

    if (gState == STATE_START) {
        setColor(inkCol);
        drawStrC(WIN_W * 0.5f, 300.f, "CHRONO RUN : DN DASH", GLUT_BITMAP_TIMES_ROMAN_24);
        setColor(tone(0.80f));
        drawStrC(WIN_W * 0.5f, 265.f, "Press SPACE to start", mono);
        setColor(tone(0.60f));
        drawStrC(WIN_W * 0.5f, 240.f, "Jump over cones, barriers & bollards  -  Duck under high drones", GLUT_BITMAP_8_BY_13);
        setColor(tone(0.50f));
        drawStrC(WIN_W * 0.5f, 50.f, "Nahdiul Islam Rony (231-115-069)  &  Abu Ubayda (231-115-080)", GLUT_BITMAP_8_BY_13);
        drawStrC(WIN_W * 0.5f, 32.f, "CSE-402 CG & IP Lab  |  Lecturer: Shrabanti Chowdhury  |  Section B", GLUT_BITMAP_8_BY_13);
    } else if (gState == STATE_PAUSED) {
        setColor(inkCol);
        drawStrC(WIN_W * 0.5f, 280.f, "P A U S E D", GLUT_BITMAP_TIMES_ROMAN_24);
        setColor(tone(0.60f));
        drawStrC(WIN_W * 0.5f, 250.f, "Press P to resume", mono);
    } else if (gState == STATE_GAME_OVER) {
        setColor(inkCol);
        drawStrC(WIN_W * 0.5f, 300.f, "G A M E   O V E R", GLUT_BITMAP_TIMES_ROMAN_24);
        drawRestartIcon(WIN_W * 0.5f, 262.f);
        setColor(tone(0.60f));
        drawStrC(WIN_W * 0.5f, 222.f, "Press SPACE or R to restart", mono);
        if ((int)score >= highScore && score > 5.f) {
            setColor(inkCol);
            drawStrC(WIN_W * 0.5f, 198.f, "NEW HIGH SCORE!", mono);
        }
    }
}

void display() {
    updatePalette();
    glClearColor(bgCol.r, bgCol.g, bgCol.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderBackground();
    renderGround();
    renderObstacles();
    renderRunner();
    renderParticles();
    renderHitboxes();
    renderHUD();

    glutSwapBuffers();
}

// ============================================================================
// Update (dt = elapsed time measured in 60fps frames -> frame-rate independent)
// ============================================================================
void scrollWorld(float dt) {
    // Parallax layers
    auto scrollLayer = [&](std::vector<Building>& L, float k, bool isFar) {
        float maxRight = 0.f;
        for (auto& b : L) { b.x -= gameSpeed * k * dt; maxRight = std::max(maxRight, b.x + b.w); }
        for (auto& b : L) {
            if (b.x + b.w < 0.f) {
                Building nb = makeBuilding(maxRight + (isFar ? frand(4, 30) : frand(20, 90)), isFar);
                b = nb;
                maxRight = b.x + b.w;
            }
        }
    };
    scrollLayer(farB, 0.10f, true);
    scrollLayer(nearB, 0.25f, false);

    for (auto& c : clouds) {
        c.x -= 0.25f * dt;
        if (c.x < -100.f) { c.x = WIN_W + frand(20, 200); c.y = frand(250, 360); }
    }
    for (auto& d : dashes) {
        d.x -= gameSpeed * dt;
        if (d.x < -12.f) { d.x = WIN_W + frand(0, 40); d.y = frand(GROUND_Y - 30, GROUND_Y - 6); d.w = frand(2, 10); }
    }
    for (auto& b : bumps) {
        b.x -= gameSpeed * dt;
        if (b.x < -40.f) { b.x = WIN_W + frand(50, 500); b.w = frand(14, 30); }
    }
}

void updateGame(float dt) {
    if (gState != STATE_PLAYING) {
        // Let crash particles settle on game over
        for (size_t i = 0; i < particles.size(); ) {
            particles[i].x += particles[i].vx * dt; particles[i].y += particles[i].vy * dt;
            particles[i].life -= particles[i].decay * dt;
            if (particles[i].life <= 0.f) particles.erase(particles.begin() + i); else ++i;
        }
        return;
    }

    cycleT += dt / 60.f;

    // Score and gentle speed progression
    score += 0.1f * (gameSpeed / BASE_SPEED) * dt;
    if ((int)score > highScore) highScore = (int)score;
    gameSpeed = BASE_SPEED + score * SPEED_RAMP;
    if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;

    if ((int)score / 100 > lastMile) { lastMile = (int)score / 100; blinkTimer = 1.2f; playSnd(sndScore); }
    if (blinkTimer > 0.f) blinkTimer -= dt / 60.f;

    // Jump physics (holding DOWN in the air = fast fall)
    if (!grounded) {
        rVY -= GRAVITY * dt;
        if (ducking) rVY -= GRAVITY * 1.5f * dt;
        rY += rVY * dt;
        if (rY <= GROUND_Y) { rY = GROUND_Y; rVY = 0.f; grounded = true; spawnDust(rX + 20.f, GROUND_Y, 5); }
    }

    runAnim += 0.15f * (gameSpeed / BASE_SPEED) * dt;

    scrollWorld(dt);

    // Spawning: generous gaps that grow with speed
    spawnAcc += gameSpeed * dt;
    if (spawnAcc >= nextSpawn) {
        spawnObstacle();
        spawnAcc  = 0.f;
        nextSpawn = 380.f + (gameSpeed - BASE_SPEED) * 40.f + frand(0, 340);
    }

    // Obstacles + AABB collision
    RectAABB rb = runnerBox();
    for (size_t i = 0; i < obstacles.size(); ) {
        obstacles[i].x    -= gameSpeed * dt;
        obstacles[i].anim += dt / 60.f;
        if (aabbCollide(rb, obstBox(obstacles[i]))) {
            gState = STATE_GAME_OVER;
            playSnd(sndOver);
            for (int k = 0; k < 14; ++k)
                particles.push_back({ rX + 30.f, rY + 40.f, frand(-3, 3), frand(-1, 4), 1.f, frand(0.02f, 0.04f), frand(3, 5) });
            break;
        }
        if (obstacles[i].x + obstacles[i].w < -50.f) obstacles.erase(obstacles.begin() + i);
        else ++i;
    }

    for (size_t i = 0; i < particles.size(); ) {
        particles[i].x += particles[i].vx * dt; particles[i].y += particles[i].vy * dt;
        particles[i].life -= particles[i].decay * dt;
        if (particles[i].life <= 0.f) particles.erase(particles.begin() + i); else ++i;
    }
}

void timerCB(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / (1000.f / 60.f);
    lastTime = now;
    if (dt < 0.f) dt = 0.f;
    if (dt > 3.f) dt = 3.f;   // avoid huge jumps after window drag / stall
    updateGame(dt);
    glutPostRedisplay();
    glutTimerFunc(FRAME_MS, timerCB, 0);
}

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

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Chrono Run: DN Dash  |  CSE-402 CG & IP Lab");

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
