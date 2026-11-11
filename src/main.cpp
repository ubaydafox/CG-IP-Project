/*
 ============================================================================
  Project Title : CHRONO RUN: DN DASH  (v2.0 - City Edition)
  Course        : Computer Graphics & Image Processing Lab (CSE-402)
  Institution   : Metropolitan University, Sylhet
  Lecturer      : Shrabanti Chowdhury

  Submitted by  :
    1. Nahdiul Islam Rony (ID: 231-115-069)
    2. Abu Ubayda         (ID: 231-115-080)
  Section       : B, 58th Batch, Dept. of CSE

  Description   :
    An interactive 2D endless runner desktop game built with C++, OpenGL,
    and FreeGLUT. Set in a modern city at any hour of the day, featuring:
      - Seamless 3-minute dynamic day-night cycle over a neon-lit skyline
      - Multi-layer parallax city building scroll
      - Animated window flickering during night / dusk phases
      - Cyberpunk hoodie runner with neon cyan visor and trim
      - City obstacles: traffic cones, road barriers, bollards, drones
      - Softer, human-survivable speed progression (5 to 11 px/frame)
      - Score milestone flash with particle burst every 100 points
      - AABB collision detection with live hitbox debugger [H]
      - Dust particles on run, landing, and neon spark explosion on crash
      - Clean silent operation (Beep() removed - no unsynchronized noise)
 ============================================================================
*/

#define _USE_MATH_DEFINES
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

// Note on Audio: Windows Beep() was removed. It runs on a system timer
// thread and cannot be synchronized with the OpenGL render loop, producing
// random unsynchronized beeping noise. The game runs clean with no audio.

// ============================================================================
// Constants
// ============================================================================
const int   WIN_W          = 1000;
const int   WIN_H          = 600;
const float GROUND_Y       = 100.0f;
const int   TARGET_FPS     = 60;
const int   FRAME_MS       = 1000 / TARGET_FPS;
const float CYCLE_DURATION = 180.0f; // 3-minute full day-night cycle
const float BASE_SPEED     = 5.0f;
const float MAX_SPEED      = 11.0f;

// ============================================================================
// Math / Color Helpers
// ============================================================================
struct ColorRGB {
    float r, g, b;
    ColorRGB(float r = 0.f, float g = 0.f, float b = 0.f) : r(r), g(g), b(b) {}
    ColorRGB operator*(float s) const { return ColorRGB(r*s, g*s, b*s); }
    ColorRGB operator+(const ColorRGB& o) const { return ColorRGB(r+o.r, g+o.g, b+o.b); }
};

// Linear color interpolation (Lerp) - CG concept: C(t) = C1 + (C2-C1)*t
ColorRGB lerp(const ColorRGB& a, const ColorRGB& b, float t) {
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return ColorRGB(a.r+(b.r-a.r)*t, a.g+(b.g-a.g)*t, a.b+(b.b-a.b)*t);
}

// ============================================================================
// AABB Collision Detection
// ============================================================================
struct RectAABB { float x, y, w, h; };

bool aabbCollide(const RectAABB& a, const RectAABB& b) {
    return (a.x < b.x + b.w  &&
            a.x + a.w > b.x  &&
            a.y < b.y + b.h  &&
            a.y + a.h > b.y);
}

// ============================================================================
// Data Structures
// ============================================================================

// Dust / crash particle
struct Particle {
    float x, y, vx, vy, life, decay, size;
    ColorRGB color;
};

// City building (used for parallax skyline)
struct Building {
    float x, y, w, h, scrollSpd;
    ColorRGB color;
    int wCols, wRows;           // window grid dimensions
    bool  winLit[6][8];         // per-window on/off state
    float winFlicker[6][8];     // per-window timer until next flicker
};

// Road surface detail (cracks, pebbles, manholes)
struct RoadDetail {
    float x, y, w, h;
    int type; // 0=crack, 1=pebble, 2=manhole
};

// City obstacle types
enum ObstType {
    OBS_CONE = 0,    // Single traffic cone  - jump over
    OBS_CONE2,       // Double traffic cone  - bigger jump
    OBS_BARRIER,     // Jersey road barrier  - jump over
    OBS_BOLLARD,     // Bollard cluster      - jump over
    OBS_DRONE_HIGH,  // Delivery drone high  - duck under!
    OBS_DRONE_LOW    // Delivery drone low   - jump over!
};

struct Obstacle {
    ObstType type;
    float x, y, w, h, animT;
    bool  passed;
};

struct Star { float x, y, size, twinkle; };

// Day-night phase enum
enum DayPhase  { PHASE_DAWN=0, PHASE_DAY, PHASE_DUSK, PHASE_NIGHT };
// Game state enum
enum GameState { STATE_START=0, STATE_PLAYING, STATE_PAUSED, STATE_GAME_OVER };

// ============================================================================
// Global State
// ============================================================================
GameState gState  = STATE_START;
DayPhase  gPhase  = PHASE_DAY;
float ambientLight = 1.0f;

// Runner
float rX = 130.f, rY = GROUND_Y, rVY = 0.f;
const float JUMP_FORCE = 14.5f;
const float GRAVITY    = 0.72f;
// Runner hitbox dimensions
const float RW_STAND = 36.f, RH_STAND = 68.f;
const float RW_DUCK  = 58.f, RH_DUCK  = 36.f;
const float HB_PX    = 5.f,  HB_PY    = 4.f;
bool grounded = true, ducking = false;
float runAnim = 0.f;

// Progression
float gameSpeed = BASE_SPEED, score = 0.f;
int   highScore = 0;
float cycleT    = 0.f;

// Obstacle spawn accumulators (global so resetGame() can clear them)
float spawnAcc = 0.f, nextSpawn = 300.f;

// Score milestone flash
float       flashTimer = 0.f;
int         lastMile   = 0;
std::string flashText  = "";

bool showHB = false; // hitbox debugger toggle

// World collections
std::vector<Obstacle>   obstacles;
std::vector<Particle>   particles;
std::vector<Building>   buildings;
std::vector<RoadDetail> roadDets;
std::vector<Star>       stars;
float laneX[14]; // scrolling road center-lane stripe positions

// ============================================================================
// OpenGL Drawing Primitives
// ============================================================================
void fillCircle(float cx, float cy, float r, int seg = 20) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; ++i) {
        float a = i * 2.f * (float)M_PI / seg;
        glVertex2f(cx + cosf(a)*r, cy + sinf(a)*r);
    }
    glEnd();
}

void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x,   y);   glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x,   y+h);
    glEnd();
}

void outlineRect(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,   y);   glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x,   y+h);
    glEnd();
}

void drawStr(float x, float y, const std::string& s, void* f = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(f, c);
}

// ============================================================================
// Particle Emitters
// ============================================================================
void spawnDust(float x, float y, int n = 4, float bvx = -2.f, float bvy = 1.f) {
    for (int i = 0; i < n; ++i) {
        Particle p;
        p.x = x + (float)(rand()%12 - 6);
        p.y = y + (float)(rand()%6);
        p.vx = bvx + ((rand()%100)/100.f - 0.5f) * 2.f;
        p.vy = bvy + (rand()%100)/100.f * 2.5f;
        p.life  = 1.f;
        p.decay = 0.04f + (rand()%100)/100.f * 0.04f;
        p.size  = 2.f + (float)(rand()%3);
        p.color = ColorRGB(0.42f, 0.42f, 0.48f); // dark asphalt dust
        particles.push_back(p);
    }
}

void spawnCrash(float x, float y) {
    for (int i = 0; i < 36; ++i) {
        Particle p;
        p.x = x; p.y = y;
        float a = (float)(rand()%360) * (float)M_PI / 180.f;
        float sp = 2.f + (rand()%60)/10.f;
        p.vx = cosf(a)*sp; p.vy = sinf(a)*sp;
        p.life  = 1.f;
        p.decay = 0.02f + (rand()%20)/1000.f;
        p.size  = 3.f + (float)(rand()%4);
        // Neon spark colours: cyan / orange / white
        switch (i % 3) {
            case 0: p.color = ColorRGB(0.f,  1.f,  0.9f); break;
            case 1: p.color = ColorRGB(1.f,  0.5f, 0.f);  break;
            default:p.color = ColorRGB(1.f,  1.f,  1.f);  break;
        }
        particles.push_back(p);
    }
}

void spawnMilestone(float x, float y) {
    for (int i = 0; i < 28; ++i) {
        Particle p;
        p.x = x + (float)(rand()%320 - 160);
        p.y = y + (float)(rand()%120);
        p.vx = ((rand()%100)/100.f - 0.5f) * 4.f;
        p.vy = 1.f + (rand()%40)/10.f;
        p.life  = 1.f;
        p.decay = 0.020f;
        p.size  = 2.f + (float)(rand()%4);
        p.color = (i%2==0) ? ColorRGB(0.f, 0.95f, 1.f) : ColorRGB(1.f, 0.85f, 0.f);
        particles.push_back(p);
    }
}

// ============================================================================
// Building Window Initialization
// ============================================================================
void initWins(Building& b) {
    for (int c = 0; c < b.wCols; ++c)
        for (int r = 0; r < b.wRows; ++r) {
            b.winLit[c][r]     = (rand()%3 != 0);           // 2/3 windows lit
            b.winFlicker[c][r] = 4.f + (rand()%80)/10.f;   // seconds until next flicker
        }
}

// ============================================================================
// World Initialization
// ============================================================================
void initWorld() {
    srand((unsigned)time(0));

    // --- Stars ---
    stars.clear();
    for (int i = 0; i < 90; ++i) {
        Star s;
        s.x      = (float)(rand() % WIN_W);
        s.y      = GROUND_Y + 100.f + (float)(rand() % (WIN_H - (int)GROUND_Y - 120));
        s.size   = 1.f + (rand()%3)*0.6f;
        s.twinkle= (rand()%100)/10.f;
        stars.push_back(s);
    }

    // --- City buildings: far layer (slower, taller, darker) ---
    buildings.clear();
    float bx = -100.f;
    while (bx < WIN_W * 2.6f) {
        Building b;
        b.x = bx; b.y = GROUND_Y;
        b.w = 55.f + (float)(rand()%90);
        b.h = 160.f + (float)(rand()%240);
        b.scrollSpd = 0.14f;
        float sh = 0.11f + (rand()%8)/100.f;
        b.color = ColorRGB(sh, sh, sh + 0.04f);
        b.wCols = 2 + rand()%3; b.wRows = 3 + rand()%5;
        if (b.wCols > 6) b.wCols = 6;
        if (b.wRows > 8) b.wRows = 8;
        initWins(b);
        buildings.push_back(b);
        bx += b.w + 2.f + (float)(rand()%16);
    }

    // --- City buildings: near layer (faster, shorter, slightly lighter) ---
    bx = -60.f;
    while (bx < WIN_W * 2.0f) {
        Building b;
        b.x = bx; b.y = GROUND_Y;
        b.w = 38.f + (float)(rand()%58);
        b.h = 78.f + (float)(rand()%115);
        b.scrollSpd = 0.32f;
        float sh = 0.17f + (rand()%8)/100.f;
        b.color = ColorRGB(sh, sh + 0.01f, sh + 0.05f);
        b.wCols = 2 + rand()%4; b.wRows = 2 + rand()%3;
        if (b.wCols > 6) b.wCols = 6;
        if (b.wRows > 8) b.wRows = 8;
        initWins(b);
        buildings.push_back(b);
        bx += b.w + 2.f + (float)(rand()%12);
    }

    // --- Road details ---
    roadDets.clear();
    for (int i = 0; i < 22; ++i) {
        RoadDetail rd;
        rd.x    = (float)(rand() % WIN_W);
        rd.y    = 15.f + (float)(rand() % (int)(GROUND_Y - 25));
        rd.w    = 8.f  + (float)(rand()%24);
        rd.h    = 2.f  + (float)(rand()%4);
        rd.type = rand()%3;
        roadDets.push_back(rd);
    }

    // --- Lane stripe starting positions ---
    for (int i = 0; i < 14; ++i) laneX[i] = i * 80.f;

    obstacles.clear();
    particles.clear();

    rX = 130.f; rY = GROUND_Y; rVY = 0.f;
    grounded = true; ducking = false; runAnim = 0.f;
    gameSpeed = BASE_SPEED; score = 0.f; cycleT = 0.f;
    spawnAcc = 0.f; nextSpawn = 300.f;
    flashTimer = 0.f; lastMile = 0; flashText = "";
}

void resetGame() {
    obstacles.clear();
    particles.clear();
    rX = 130.f; rY = GROUND_Y; rVY = 0.f;
    grounded = true; ducking = false; runAnim = 0.f;
    gameSpeed = BASE_SPEED; score = 0.f; cycleT = 0.f;
    spawnAcc = 0.f; nextSpawn = 300.f;
    flashTimer = 0.f; lastMile = 0; flashText = "";
    gState = STATE_PLAYING;
}

// ============================================================================
// Obstacle Spawning
// ============================================================================
void spawnObstacle() {
    Obstacle o;
    o.x = WIN_W + 40.f; o.animT = 0.f; o.passed = false;

    int ch = rand() % 100;
    if (score > 200.f && ch < 28) {
        // Aerial drone obstacles (unlock after 200 pts)
        if (rand()%2 == 0) {
            // High drone: duck under!
            // y=GROUND_Y+50 so bottom of hitbox = GROUND_Y+53
            // Duck top = GROUND_Y+36 < GROUND_Y+53 → safe to duck ✓
            // Stand top = GROUND_Y+64 > GROUND_Y+53 → must duck ✓
            o.type = OBS_DRONE_HIGH;
            o.w = 52.f; o.h = 28.f; o.y = GROUND_Y + 50.f;
        } else {
            // Low drone: jump over!
            // y=GROUND_Y+10 so it hits standing runner → must jump ✓
            o.type = OBS_DRONE_LOW;
            o.w = 52.f; o.h = 28.f; o.y = GROUND_Y + 10.f;
        }
    } else {
        // Ground obstacles
        int t = rand() % 5;
        switch (t) {
            case 0: o.type=OBS_CONE;    o.w=24.f; o.h=42.f; o.y=GROUND_Y; break;
            case 1: o.type=OBS_CONE2;   o.w=50.f; o.h=42.f; o.y=GROUND_Y; break;
            case 2: o.type=OBS_BARRIER; o.w=50.f; o.h=38.f; o.y=GROUND_Y; break;
            case 3: o.type=OBS_BOLLARD; o.w=46.f; o.h=46.f; o.y=GROUND_Y; break;
            default:o.type=OBS_CONE;    o.w=24.f; o.h=42.f; o.y=GROUND_Y; break;
        }
    }
    obstacles.push_back(o);
}

// ============================================================================
// Sky + City Rendering
// ============================================================================
void renderSky() {
    // t = normalized time in [0, 1] over the 3-minute cycle
    float t = fmodf(cycleT, CYCLE_DURATION) / CYCLE_DURATION;

    // ---- Sky keyframe colours (city aesthetic) ----
    ColorRGB dawnTop(0.14f, 0.20f, 0.40f), dawnBot(0.78f, 0.44f, 0.28f);
    ColorRGB dayTop (0.20f, 0.50f, 0.86f), dayBot (0.56f, 0.76f, 1.00f);
    ColorRGB duskTop(0.26f, 0.10f, 0.34f), duskBot(0.94f, 0.38f, 0.14f);
    ColorRGB nightTop(0.02f, 0.03f, 0.10f), nightBot(0.05f, 0.07f, 0.18f);

    ColorRGB skyTop, skyBot;
    float nf = 0.f; // night factor: 0.0 = full day, 1.0 = full night

    if (t < 0.25f) {
        // Dawn → Day
        float f = t / 0.25f;
        skyTop = lerp(dawnTop, dayTop, f);
        skyBot = lerp(dawnBot, dayBot, f);
        ambientLight = 0.70f + 0.30f * f;
        gPhase = (f < 0.5f) ? PHASE_DAWN : PHASE_DAY;
        nf = (1.f - f) * 0.30f;
    } else if (t < 0.50f) {
        // Day → Dusk
        float f = (t - 0.25f) / 0.25f;
        skyTop = lerp(dayTop, duskTop, f);
        skyBot = lerp(dayBot, duskBot, f);
        ambientLight = 1.0f - 0.28f * f;
        gPhase = (f < 0.6f) ? PHASE_DAY : PHASE_DUSK;
        nf = f * 0.22f;
    } else if (t < 0.75f) {
        // Dusk → Night
        float f = (t - 0.50f) / 0.25f;
        skyTop = lerp(duskTop, nightTop, f);
        skyBot = lerp(duskBot, nightBot, f);
        ambientLight = 0.72f - 0.37f * f;
        gPhase = (f < 0.4f) ? PHASE_DUSK : PHASE_NIGHT;
        nf = 0.22f + 0.78f * f;
    } else {
        // Night → Dawn
        float f = (t - 0.75f) / 0.25f;
        skyTop = lerp(nightTop, dawnTop, f);
        skyBot = lerp(nightBot, dawnBot, f);
        ambientLight = 0.35f + 0.35f * f;
        gPhase = (f < 0.6f) ? PHASE_NIGHT : PHASE_DAWN;
        nf = 1.0f - f * 0.70f;
    }

    // ---- Sky gradient (horizon to top) ----
    glBegin(GL_QUADS);
    glColor3f(skyBot.r, skyBot.g, skyBot.b);
    glVertex2f(0,     GROUND_Y); glVertex2f(WIN_W, GROUND_Y);
    glColor3f(skyTop.r, skyTop.g, skyTop.b);
    glVertex2f(WIN_W, WIN_H);   glVertex2f(0,     WIN_H);
    glEnd();

    // ---- Stars (visible during dusk → night → dawn) ----
    if (nf > 0.05f) {
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        for (const auto& s : stars) {
            float tw = 0.5f + 0.5f * sinf(glutGet(GLUT_ELAPSED_TIME)*0.004f + s.twinkle);
            float a  = nf * tw;
            glColor3f(a*0.88f, a*0.92f, a);
            glVertex2f(s.x, s.y);
        }
        glEnd();
    }

    // ---- Sun (first half of cycle, t in [0, 0.5]) ----
    if (t < 0.50f) {
        float sp = t / 0.50f; // 0 → 1 as sun crosses sky
        float sx = 80.f + (WIN_W - 160.f) * sp;
        float sy = GROUND_Y + 60.f + sinf(sp * (float)M_PI) * 340.f;
        // Outer glow
        glColor3f(1.f, 0.82f, 0.30f); fillCircle(sx, sy, 30.f, 24);
        // Inner bright core
        glColor3f(1.f, 0.95f, 0.62f); fillCircle(sx, sy, 22.f, 20);
        // Rotating rays
        float rt = glutGet(GLUT_ELAPSED_TIME) * 0.0015f;
        glColor3f(1.f, 0.80f, 0.20f);
        glLineWidth(2.f);
        glBegin(GL_LINES);
        for (int i = 0; i < 8; ++i) {
            float a = rt + i * (float)M_PI * 0.25f;
            glVertex2f(sx + cosf(a)*32.f, sy + sinf(a)*32.f);
            glVertex2f(sx + cosf(a)*42.f, sy + sinf(a)*42.f);
        }
        glEnd();
        glLineWidth(1.f);
    } else {
        // ---- Moon (second half of cycle, t in [0.5, 1.0]) ----
        float mp = (t - 0.50f) / 0.50f;
        float mx = 80.f + (WIN_W - 160.f) * mp;
        float my = GROUND_Y + 60.f + sinf(mp * (float)M_PI) * 340.f;
        glColor3f(0.80f, 0.85f, 1.0f);  fillCircle(mx, my, 24.f, 22);
        glColor3f(0.95f, 0.97f, 1.0f);  fillCircle(mx, my, 20.f, 20);
        // Craters
        glColor3f(0.72f, 0.78f, 0.90f);
        fillCircle(mx-6.f, my+4.f,  4.f,  10);
        fillCircle(mx+7.f, my-3.f,  3.f,  10);
        fillCircle(mx+2.f, my+8.f,  2.5f, 8);
    }

    // ---- City Skyline: far layer (slow, tall, dark) ----
    for (auto& b : buildings) {
        if (b.scrollSpd > 0.20f) continue; // skip near layer here

        // Buildings are lighter in day, darker at night
        ColorRGB bc = lerp(
            ColorRGB(b.color.r*1.4f, b.color.g*1.4f, b.color.b*1.5f),
            b.color, nf);
        glColor3f(bc.r, bc.g, bc.b);
        fillRect(b.x, b.y, b.w, b.h);

        // Window grid
        float ww = b.w / (b.wCols + 1) * 0.65f;
        float wh = b.h / (b.wRows + 2) * 0.50f;
        for (int c = 0; c < b.wCols; ++c) {
            for (int r = 0; r < b.wRows; ++r) {
                float wx = b.x + (c+1) * b.w / (b.wCols+1) - ww*0.5f;
                float wy = b.y + (r+1) * b.h / (b.wRows+1) - wh*0.5f;
                if (b.winLit[c][r]) {
                    // Warm yellow in day, neon cyan/yellow at night
                    ColorRGB wd(0.95f, 0.90f, 0.65f);
                    ColorRGB wn = ((c+r)%2==0)
                        ? ColorRGB(0.f, 0.90f, 1.0f)      // cyan
                        : ColorRGB(0.95f, 0.80f, 0.20f);  // yellow
                    ColorRGB wc = lerp(wd, wn, nf);
                    glColor3f(wc.r * ambientLight, wc.g * ambientLight, wc.b * ambientLight);
                } else {
                    glColor3f(0.05f, 0.05f, 0.08f);
                }
                fillRect(wx, wy, ww, wh);
            }
        }
    }

    // ---- City Skyline: near layer (faster, shorter) ----
    for (auto& b : buildings) {
        if (b.scrollSpd < 0.25f) continue; // skip far layer here

        ColorRGB bc = lerp(
            ColorRGB(b.color.r*1.3f, b.color.g*1.3f, b.color.b*1.4f),
            b.color, nf);
        glColor3f(bc.r, bc.g, bc.b);
        fillRect(b.x, b.y, b.w, b.h);

        float ww = b.w / (b.wCols + 1) * 0.60f;
        float wh = b.h / (b.wRows + 1) * 0.45f;
        for (int c = 0; c < b.wCols; ++c) {
            for (int r = 0; r < b.wRows; ++r) {
                float wx = b.x + (c+1) * b.w / (b.wCols+1) - ww*0.5f;
                float wy = b.y + (r+1) * b.h / (b.wRows+1) - wh*0.5f;
                if (b.winLit[c][r]) {
                    ColorRGB wd(0.90f, 0.85f, 0.60f);
                    ColorRGB wn = ((c*r)%2==0)
                        ? ColorRGB(0.f,  0.85f, 0.95f)
                        : ColorRGB(1.0f, 0.75f, 0.0f);
                    ColorRGB wc = lerp(wd, wn, nf);
                    glColor3f(wc.r * ambientLight, wc.g * ambientLight, wc.b * ambientLight);
                } else {
                    glColor3f(0.08f, 0.08f, 0.12f);
                }
                fillRect(wx, wy, ww, wh);
            }
        }
    }
}

// ============================================================================
// Ground / Road Rendering
// ============================================================================
void renderGround() {
    float nf = (gPhase==PHASE_NIGHT) ? 0.90f : (gPhase==PHASE_DUSK) ? 0.45f : 0.f;

    // Asphalt gradient
    ColorRGB aT = lerp(ColorRGB(0.22f, 0.22f, 0.25f), ColorRGB(0.09f, 0.09f, 0.13f), nf);
    ColorRGB aB = lerp(ColorRGB(0.15f, 0.15f, 0.17f), ColorRGB(0.05f, 0.05f, 0.08f), nf);
    glBegin(GL_QUADS);
    glColor3f(aT.r, aT.g, aT.b); glVertex2f(0,    GROUND_Y); glVertex2f(WIN_W, GROUND_Y);
    glColor3f(aB.r, aB.g, aB.b); glVertex2f(WIN_W, 0);       glVertex2f(0,     0);
    glEnd();

    // Ground border line
    glColor3f(0.35f, 0.35f, 0.42f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(0, GROUND_Y); glVertex2f(WIN_W, GROUND_Y);
    glEnd();
    glLineWidth(1.f);

    // Centre lane markings (white scrolling dashes)
    float lY = GROUND_Y * 0.52f;
    ColorRGB lC = lerp(ColorRGB(0.74f, 0.74f, 0.74f), ColorRGB(0.32f, 0.32f, 0.40f), nf);
    glColor3f(lC.r, lC.g, lC.b);
    for (int i = 0; i < 14; ++i) fillRect(laneX[i], lY - 2.f, 46.f, 3.f);

    // Road details (cracks, pebbles, manholes)
    for (const auto& rd : roadDets) {
        if (rd.type == 0) {
            // Pavement crack
            glColor3f(0.13f, 0.13f, 0.15f);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
            glVertex2f(rd.x, rd.y); glVertex2f(rd.x + rd.w, rd.y + rd.h);
            glEnd();
            glLineWidth(1.f);
        } else if (rd.type == 1) {
            // Pebble
            glColor3f(0.24f, 0.23f, 0.25f);
            fillCircle(rd.x, rd.y, rd.h, 6);
        } else {
            // Manhole cover
            glColor3f(0.19f, 0.19f, 0.22f); fillCircle(rd.x, rd.y, rd.w*0.5f, 12);
            glColor3f(0.16f, 0.16f, 0.19f); fillCircle(rd.x, rd.y, rd.w*0.3f, 8);
            glColor3f(0.14f, 0.14f, 0.17f);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
            glVertex2f(rd.x - rd.w*0.4f, rd.y); glVertex2f(rd.x + rd.w*0.4f, rd.y);
            glVertex2f(rd.x, rd.y - rd.w*0.4f); glVertex2f(rd.x, rd.y + rd.w*0.4f);
            glEnd();
            glLineWidth(1.f);
        }
    }
}

// ============================================================================
// Runner Rendering — Cyberpunk Hoodie Runner
// ============================================================================
void renderRunner() {
    glPushMatrix();
    glTranslatef(rX, rY, 0.f);

    float nf = (gPhase == PHASE_NIGHT) ? 0.75f : 0.f;
    ColorRGB skin  = lerp(ColorRGB(0.90f, 0.75f, 0.62f), ColorRGB(0.52f, 0.50f, 0.58f), nf);
    ColorRGB hood  = ColorRGB(0.14f, 0.14f, 0.18f); // dark charcoal hoodie
    ColorRGB pants = ColorRGB(0.12f, 0.12f, 0.20f);
    ColorRGB shoe  = ColorRGB(0.92f, 0.92f, 0.95f); // white sneakers
    ColorRGB cyan  = ColorRGB(0.0f,  0.95f, 1.00f); // neon cyan trim

    if (ducking && grounded) {
        // --- Ducking/sliding pose ---
        if (rand()%3 == 0) spawnDust(rX - 10.f, GROUND_Y + 2.f, 1, -4.f, 0.5f);

        // Crouched hoodie torso
        glColor3f(hood.r,  hood.g,  hood.b);  fillRect(0.f, 10.f, 42.f, 16.f);
        // Head
        glColor3f(skin.r,  skin.g,  skin.b);  fillCircle(42.f, 20.f, 10.f, 14);
        // Hood (dark cap over head)
        glColor3f(hood.r, hood.g, hood.b);
        glBegin(GL_TRIANGLE_FAN); glVertex2f(42.f, 20.f);
        for (int i = 0; i <= 12; ++i) {
            float a = (float)M_PI + i * (float)M_PI / 12.f;
            glVertex2f(42.f + cosf(a)*12.f, 20.f + sinf(a)*12.f);
        }
        glEnd();
        // Neon visor (glow behind + bright strip)
        glColor3f(cyan.r*0.22f, cyan.g*0.22f, cyan.b*0.22f); fillRect(43.f, 16.f, 12.f, 8.f);
        glColor3f(cyan.r, cyan.g, cyan.b);                    fillRect(45.f, 18.f, 9.f,  4.f);
        // Crouched legs
        glColor3f(pants.r, pants.g, pants.b); fillRect(-14.f, 6.f, 26.f, 10.f);
        // White sneakers with neon stripe
        glColor3f(shoe.r, shoe.g, shoe.b);
        fillRect(-18.f, 0.f, 20.f, 7.f); fillRect(16.f, 0.f, 18.f, 7.f);
        glColor3f(cyan.r, cyan.g, cyan.b);
        fillRect(-18.f, 5.5f, 20.f, 1.5f); fillRect(16.f, 5.5f, 18.f, 1.5f);

    } else {
        // --- Standing / running / jumping pose ---
        float ls = grounded ? sinf(runAnim)*20.f : -13.f;
        float as = grounded ? cosf(runAnim)*22.f :  17.f;

        // Back leg
        glPushMatrix(); glTranslatef(14.f, 22.f, 0.f); glRotatef(-ls, 0,0,1);
        glColor3f(pants.r*0.8f, pants.g*0.8f, pants.b*0.8f); fillRect(-4.f,-13.f, 8.f,13.f);
        glColor3f(skin.r *0.85f,skin.g *0.85f,skin.b *0.85f); fillRect(-3.5f,-21.f,7.f, 9.f);
        glColor3f(shoe.r *0.85f,shoe.g *0.85f,shoe.b *0.85f); fillRect(-4.f,-25.f,13.f, 5.f);
        glColor3f(cyan.r*0.6f,  cyan.g*0.6f,  cyan.b*0.6f);   fillRect(-4.f,-21.5f,13.f,1.5f);
        glPopMatrix();

        // Hoodie torso + neon stripe
        glColor3f(hood.r, hood.g, hood.b); fillRect(4.f, 22.f, 22.f, 28.f);
        glColor3f(cyan.r*0.5f, cyan.g*0.5f, cyan.b*0.5f); fillRect(11.f, 22.f, 3.f, 28.f);

        // Back arm
        glPushMatrix(); glTranslatef(8.f, 44.f, 0.f); glRotatef(-as, 0,0,1);
        glColor3f(hood.r, hood.g, hood.b); fillRect(-3.f, -14.f, 6.f, 14.f);
        glPopMatrix();

        // Front leg
        glPushMatrix(); glTranslatef(14.f, 22.f, 0.f); glRotatef(ls, 0,0,1);
        glColor3f(pants.r, pants.g, pants.b); fillRect(-4.f,-13.f, 8.f,13.f);
        glColor3f(skin.r,  skin.g,  skin.b);  fillRect(-3.5f,-21.f, 7.f, 9.f);
        glColor3f(shoe.r,  shoe.g,  shoe.b);  fillRect(-4.f,-25.f,13.f, 5.f);
        glColor3f(cyan.r,  cyan.g,  cyan.b);  fillRect(-4.f,-21.5f,13.f,1.5f);
        glPopMatrix();

        // Head
        glColor3f(skin.r, skin.g, skin.b); fillCircle(14.f, 56.f, 10.f, 16);
        // Hood over head
        glColor3f(hood.r, hood.g, hood.b);
        glBegin(GL_TRIANGLE_FAN); glVertex2f(14.f, 56.f);
        for (int i = 0; i <= 14; ++i) {
            float a = (float)M_PI + i*(float)M_PI/14.f;
            glVertex2f(14.f + cosf(a)*13.f, 56.f + sinf(a)*13.f);
        }
        glEnd();
        // Neon visor glow + strip
        glColor3f(cyan.r*0.20f, cyan.g*0.20f, cyan.b*0.20f); fillRect(16.f, 54.f, 12.f, 7.f);
        glColor3f(cyan.r, cyan.g, cyan.b);                    fillRect(17.f, 55.f, 10.f, 4.f);

        // Front arm + fist
        glPushMatrix(); glTranslatef(18.f, 44.f, 0.f); glRotatef(as, 0,0,1);
        glColor3f(hood.r, hood.g, hood.b); fillRect(-3.f, -14.f, 6.f, 14.f);
        glColor3f(skin.r, skin.g, skin.b); fillCircle(0.f, -15.f, 3.5f, 8);
        glPopMatrix();
    }
    glPopMatrix();
}

// ============================================================================
// Obstacle Rendering — City Obstacles
// ============================================================================
void renderObstacles() {
    float nf = (gPhase==PHASE_NIGHT) ? 0.70f : (gPhase==PHASE_DUSK) ? 0.30f : 0.f;

    for (const auto& o : obstacles) {
        glPushMatrix();
        glTranslatef(o.x, o.y, 0.f);

        switch (o.type) {
            case OBS_CONE: {
                // Single orange traffic cone
                glColor3f(0.95f, 0.38f, 0.04f);
                glBegin(GL_TRIANGLES);
                glVertex2f(1.f,0.f); glVertex2f(23.f,0.f); glVertex2f(12.f,42.f);
                glEnd();
                glColor3f(0.95f, 0.95f, 0.95f);
                glBegin(GL_QUADS);
                glVertex2f(4.f,10.f); glVertex2f(20.f,10.f);
                glVertex2f(19.f,16.f); glVertex2f(5.f,16.f);
                glEnd();
                glColor3f(0.72f, 0.26f, 0.04f); fillRect(0.f,0.f,24.f,5.f);
                break;
            }
            case OBS_CONE2: {
                // Double traffic cones side by side
                for (int ci = 0; ci < 2; ++ci) {
                    float ox = ci * 26.f;
                    glColor3f(0.95f, 0.38f, 0.04f);
                    glBegin(GL_TRIANGLES);
                    glVertex2f(ox+1.f,0.f); glVertex2f(ox+23.f,0.f); glVertex2f(ox+12.f,42.f);
                    glEnd();
                    glColor3f(0.95f,0.95f,0.95f);
                    glBegin(GL_QUADS);
                    glVertex2f(ox+4.f,10.f); glVertex2f(ox+20.f,10.f);
                    glVertex2f(ox+19.f,16.f); glVertex2f(ox+5.f,16.f);
                    glEnd();
                    glColor3f(0.70f,0.24f,0.04f); fillRect(ox,0.f,24.f,5.f);
                }
                break;
            }
            case OBS_BARRIER: {
                // Jersey / road safety barrier
                ColorRGB bC = lerp(ColorRGB(0.78f,0.78f,0.82f), ColorRGB(0.30f,0.30f,0.38f), nf);
                glColor3f(bC.r, bC.g, bC.b);
                glBegin(GL_POLYGON);
                glVertex2f(0.f,0.f);  glVertex2f(50.f,0.f); glVertex2f(46.f,18.f);
                glVertex2f(42.f,38.f); glVertex2f(8.f,38.f); glVertex2f(4.f,18.f);
                glEnd();
                // Orange reflective stripe
                glColor3f(0.95f, 0.42f, 0.05f);
                glBegin(GL_QUADS);
                glVertex2f(6.f,22.f); glVertex2f(44.f,22.f);
                glVertex2f(43.f,27.f); glVertex2f(7.f,27.f);
                glEnd();
                // Reflective studs
                glColor3f(0.95f,0.95f,0.0f);
                fillCircle(14.f,38.5f,2.5f,8); fillCircle(36.f,38.5f,2.5f,8);
                break;
            }
            case OBS_BOLLARD: {
                // Cluster of 3 bollards
                for (int bi = 0; bi < 3; ++bi) {
                    float bx = bi * 15.f;
                    glColor3f(0.14f,0.14f,0.18f); fillRect(bx+3.f,0.f,10.f,42.f);
                    glColor3f(0.22f,0.22f,0.26f); fillCircle(bx+8.f,42.f,5.5f,12);
                    glColor3f(0.90f,0.75f,0.05f); fillRect(bx+3.f,20.f,10.f,4.f);
                }
                break;
            }
            case OBS_DRONE_HIGH:
            case OBS_DRONE_LOW: {
                // Delivery drone with spinning propellers
                float wf = sinf(o.animT * 18.f) * 7.f;
                // Propeller arms
                glColor3f(0.22f,0.22f,0.28f);
                fillRect(-2.f,12.f,14.f,4.f); fillRect(40.f,12.f,14.f,4.f);
                // Spinning neon cyan propellers
                glColor3f(0.0f, 0.90f, 1.0f);
                fillCircle(5.f,  14.f + wf*0.4f, 7.5f, 10);
                fillCircle(47.f, 14.f - wf*0.4f, 7.5f, 10);
                // Dark body
                glColor3f(0.20f,0.20f,0.24f); fillRect(8.f,8.f,36.f,14.f);
                glColor3f(0.28f,0.28f,0.34f); fillCircle(26.f,15.f,8.f,12);
                // Delivery package underneath
                glColor3f(0.58f,0.40f,0.20f); fillRect(13.f,0.f,24.f,10.f);
                glColor3f(0.80f,0.60f,0.28f);
                glLineWidth(1.5f);
                glBegin(GL_LINES);
                glVertex2f(13.f,5.f); glVertex2f(37.f,5.f);
                glVertex2f(25.f,0.f); glVertex2f(25.f,10.f);
                glEnd(); glLineWidth(1.f);
                // Green status LED
                glColor3f(0.0f,1.0f,0.4f); fillCircle(26.f,22.f,2.5f,8);
                break;
            }
        }
        glPopMatrix();
    }
}

// ============================================================================
// Particle Rendering
// ============================================================================
void renderParticles() {
    for (const auto& p : particles) {
        glColor3f(p.color.r*p.life, p.color.g*p.life, p.color.b*p.life);
        fillCircle(p.x, p.y, p.size*p.life, 6);
    }
}

// ============================================================================
// AABB Hitbox Debugger  [Toggle with H]
// ============================================================================
void renderHitboxes() {
    if (!showHB) return;
    glLineWidth(2.f);
    // Runner = green
    glColor3f(0.f, 1.f, 0.f);
    RectAABB rb;
    if (ducking && grounded) rb = { rX+HB_PX, rY, RW_DUCK-HB_PX*2, RH_DUCK };
    else                     rb = { rX+HB_PX, rY, RW_STAND-HB_PX*2, RH_STAND-HB_PY };
    outlineRect(rb.x, rb.y, rb.w, rb.h);
    // Obstacles = red
    glColor3f(1.f, 0.f, 0.f);
    for (const auto& o : obstacles) outlineRect(o.x+3.f, o.y, o.w-6.f, o.h-2.f);
    glLineWidth(1.f);
}

// ============================================================================
// HUD and Screen Overlays
// ============================================================================
void renderHUD() {
    // ---- Top ribbon ----
    glColor4f(0.f, 0.f, 0.f, 0.42f); fillRect(0.f, WIN_H-46.f, WIN_W, 46.f);

    std::stringstream ss, sh, sp;
    ss << "SCORE: " << std::setfill('0') << std::setw(5) << (int)score;
    sh << "HI: "   << std::setfill('0') << std::setw(5) << highScore;
    sp << "SPEED: " << std::fixed << std::setprecision(1) << (gameSpeed/BASE_SPEED) << "x";

    glColor3f(1.f,1.f,1.f);         drawStr(22.f,  WIN_H-30.f, ss.str());
    glColor3f(0.80f,0.80f,0.80f);   drawStr(210.f, WIN_H-30.f, sh.str());
    glColor3f(0.f, 0.95f, 1.0f);    drawStr(390.f, WIN_H-30.f, sp.str());

    // Phase label + icon
    std::string pStr; ColorRGB pc;
    switch (gPhase) {
        case PHASE_DAWN:  pStr="DAWN";  pc=ColorRGB(1.0f,0.65f,0.35f); break;
        case PHASE_DAY:   pStr="DAY";   pc=ColorRGB(1.0f,0.90f,0.20f); break;
        case PHASE_DUSK:  pStr="DUSK";  pc=ColorRGB(0.95f,0.42f,0.18f);break;
        default:          pStr="NIGHT"; pc=ColorRGB(0.4f,0.80f,1.0f);  break;
    }
    glColor3f(pc.r,pc.g,pc.b); fillCircle(575.f, WIN_H-26.f, 7.f, 12);
    glColor3f(0.90f,0.90f,0.90f); drawStr(590.f, WIN_H-30.f, "TIME: " + pStr);
    if (showHB) { glColor3f(0.2f,1.f,0.2f); drawStr(748.f, WIN_H-30.f, "[HITBOX]"); }

    // ---- Bottom controls hint ----
    glColor3f(0.65f, 0.65f, 0.65f);
    drawStr(16.f, 15.f,
        "[SPACE/UP] Jump  [DOWN] Duck  [P] Pause  [R] Restart  [H] Hitboxes  [ESC] Quit",
        GLUT_BITMAP_8_BY_13);

    // ---- Score milestone flash ----
    if (flashTimer > 0.f) {
        float a = flashTimer / 1.5f;
        glColor4f(0.f, 0.90f, 1.0f, a * 0.16f);
        fillRect(0.f, 0.f, WIN_W, WIN_H);
        glColor4f(1.f, 1.f, 1.f, a);
        drawStr(WIN_W*0.5f - 70.f, WIN_H*0.5f + 16.f, flashText, GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // ---- State overlays ----
    if (gState == STATE_START) {
        glColor4f(0.03f,0.05f,0.12f,0.86f); fillRect(0,0,WIN_W,WIN_H);
        glColor3f(0.08f,0.10f,0.20f); fillRect(118.f,132.f,764.f,340.f);
        glColor3f(0.0f,0.90f,1.0f);   outlineRect(118.f,132.f,764.f,340.f);

        glColor3f(0.0f,0.95f,1.0f);
        drawStr(275.f, 426.f, "CHRONO RUN: DN DASH  v2.0", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.60f,0.80f,1.0f);
        drawStr(222.f, 390.f, "Computer Graphics & Image Processing Lab (CSE-402)");
        glColor3f(1.0f,0.90f,0.0f);
        drawStr(258.f, 342.f, "Press  [SPACEBAR]  or  [UP]  to Start Running");
        glColor3f(0.85f,0.85f,0.90f);
        drawStr(295.f, 298.f, "JUMP over  traffic cones, barriers & bollards");
        drawStr(295.f, 274.f, "DUCK under  high-flying delivery drones");
        glColor3f(0.50f,0.65f,0.85f);
        drawStr(170.f, 200.f, "Project by: Nahdiul Islam Rony (231-115-069) & Abu Ubayda (231-115-080)");
        drawStr(316.f, 174.f, "Lecturer: Shrabanti Chowdhury  |  Section B");

    } else if (gState == STATE_PAUSED) {
        glColor4f(0.03f,0.05f,0.14f,0.70f); fillRect(0,0,WIN_W,WIN_H);
        glColor3f(0.10f,0.14f,0.28f); fillRect(295.f,218.f,410.f,162.f);
        glColor3f(0.0f,0.90f,1.0f);   outlineRect(295.f,218.f,410.f,162.f);
        glColor3f(0.0f,0.95f,1.0f);
        drawStr(430.f, 332.f, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.90f,0.90f,0.90f);
        drawStr(344.f, 280.f, "Press [P] Resume  |  [R] Restart");

    } else if (gState == STATE_GAME_OVER) {
        glColor4f(0.06f,0.0f,0.10f,0.82f); fillRect(0,0,WIN_W,WIN_H);
        glColor3f(0.14f,0.06f,0.18f); fillRect(238.f,168.f,524.f,258.f);
        glColor3f(0.0f,0.90f,1.0f);   outlineRect(238.f,168.f,524.f,258.f);
        glColor3f(0.0f,0.95f,1.0f);
        drawStr(390.f, 382.f, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        std::stringstream sf; sf << "FINAL SCORE: " << (int)score;
        glColor3f(1.f,1.f,1.f); drawStr(390.f, 332.f, sf.str());
        if ((int)score >= highScore && score > 5.f) {
            glColor3f(1.f,0.85f,0.f);
            drawStr(366.f, 292.f, "*** NEW HIGH SCORE! ***");
        } else {
            std::stringstream sh2; sh2 << "BEST SCORE: " << highScore;
            glColor3f(0.70f,0.80f,0.90f); drawStr(400.f, 292.f, sh2.str());
        }
        glColor3f(0.0f,0.95f,1.0f);
        drawStr(316.f, 232.f, "Press [R] or [SPACEBAR] to Play Again");
    }
}

// ============================================================================
// Main Display Callback
// ============================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    renderSky();         // 1. Sky gradient, sun/moon, stars, city skyline (parallax)
    renderGround();      // 2. Asphalt road, lane stripes, road details
    renderObstacles();   // 3. Cones, barriers, bollards, drones
    renderRunner();      // 4. Cyberpunk hoodie runner
    renderParticles();   // 5. Dust, neon spark explosion
    renderHitboxes();    // 6. AABB debug outlines [H]
    renderHUD();         // 7. Score, phase indicator, overlays

    glutSwapBuffers();
}

// ============================================================================
// Game Logic Update (called at 60 FPS)
// ============================================================================
void updateGame() {
    if (gState != STATE_PLAYING) return;

    // 1. Advance day-night timer
    cycleT += 1.f / TARGET_FPS;

    // 2. Score & Speed Progression
    //    Score increments proportionally to current speed.
    //    Speed ramp: BASE + score * 0.002  (soft ramp, reaches MAX at ~3000 pts)
    score += (gameSpeed / BASE_SPEED) * 0.16f;
    if ((int)score > highScore) highScore = (int)score;
    gameSpeed = BASE_SPEED + score * 0.002f;
    if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;

    // 3. Score milestone flash (every 100 pts)
    if ((int)score / 100 > lastMile) {
        lastMile = (int)score / 100;
        flashTimer = 1.5f;
        std::stringstream ms;
        ms << "NICE!  " << lastMile * 100 << " pts";
        flashText = ms.str();
        spawnMilestone(500.f, 250.f);
    }
    if (flashTimer > 0.f) { flashTimer -= 1.f/TARGET_FPS; if (flashTimer < 0.f) flashTimer = 0.f; }

    // 4. Jump / Gravity Physics
    if (!grounded) {
        rVY -= GRAVITY;
        rY  += rVY;
        if (rY <= GROUND_Y) {
            rY = GROUND_Y; rVY = 0.f; grounded = true;
            spawnDust(rX + 10.f, GROUND_Y, 6, -2.f, 1.5f); // landing dust puff
        }
    }

    // 5. Run animation & footstep dust
    runAnim += 0.26f * (gameSpeed / BASE_SPEED);
    if (grounded && !ducking && rand()%7 == 0)
        spawnDust(rX - 4.f, GROUND_Y + 2.f, 1, -3.f, 0.7f);

    // 6. Scroll lane stripes
    for (int i = 0; i < 14; ++i) {
        laneX[i] -= gameSpeed * 1.5f;
        if (laneX[i] < -50.f) laneX[i] += 14 * 80.f;
    }

    // 7. Scroll buildings (multi-layer parallax)
    for (auto& b : buildings) {
        b.x -= gameSpeed * b.scrollSpd;
        if (b.x + b.w < -10.f) {
            // Find rightmost building in the same parallax layer
            float maxX = 0.f, maxW = 60.f;
            for (const auto& ob2 : buildings) {
                if (fabsf(ob2.scrollSpd - b.scrollSpd) < 0.01f && ob2.x > maxX) {
                    maxX = ob2.x; maxW = ob2.w;
                }
            }
            if (maxX < 0.f) maxX = WIN_W;
            b.x = maxX + maxW + 2.f + (float)(rand()%20);
        }
    }

    // 8. Window flickering (dusk & night only)
    if (gPhase == PHASE_NIGHT || gPhase == PHASE_DUSK) {
        for (auto& b : buildings) {
            for (int c = 0; c < b.wCols; ++c) {
                for (int r = 0; r < b.wRows; ++r) {
                    b.winFlicker[c][r] -= 1.f / TARGET_FPS;
                    if (b.winFlicker[c][r] <= 0.f) {
                        if (rand()%10 < 2) b.winLit[c][r] = !b.winLit[c][r];
                        b.winFlicker[c][r] = 3.f + (rand()%120)/10.f;
                    }
                }
            }
        }
    }

    // 9. Scroll road details
    for (auto& rd : roadDets) {
        rd.x -= gameSpeed;
        if (rd.x < -25.f) {
            rd.x  = WIN_W + (float)(rand()%40);
            rd.type = rand()%3;
        }
    }

    // 10. Obstacle Spawning
    //     Gap scales with speed: more room to react at high speed
    spawnAcc += gameSpeed;
    if (spawnAcc >= nextSpawn) {
        spawnObstacle();
        spawnAcc  = 0.f;
        float mn  = 350.f + (gameSpeed - BASE_SPEED) * 24.f;
        nextSpawn = mn + (float)(rand() % 280);
    }

    // 11. Update Obstacles & AABB Collision Detection
    RectAABB rb;
    if (ducking && grounded) rb = { rX+HB_PX, rY, RW_DUCK-HB_PX*2, RH_DUCK };
    else                     rb = { rX+HB_PX, rY, RW_STAND-HB_PX*2, RH_STAND-HB_PY };

    for (size_t i = 0; i < obstacles.size(); ) {
        obstacles[i].x     -= gameSpeed;
        obstacles[i].animT += 1.f / TARGET_FPS;

        RectAABB ob = { obstacles[i].x+3.f, obstacles[i].y,
                        obstacles[i].w-6.f,  obstacles[i].h-2.f };
        if (aabbCollide(rb, ob)) {
            gState = STATE_GAME_OVER;
            spawnCrash(rX + 18.f, rY + 30.f);
            break;
        }
        if (obstacles[i].x + obstacles[i].w < -50.f) obstacles.erase(obstacles.begin()+i);
        else ++i;
    }

    // 12. Update Particles
    for (size_t i = 0; i < particles.size(); ) {
        particles[i].x    += particles[i].vx;
        particles[i].y    += particles[i].vy;
        particles[i].life -= particles[i].decay;
        if (particles[i].life <= 0.f) particles.erase(particles.begin()+i);
        else ++i;
    }
}

// ============================================================================
// 60 FPS Timer
// ============================================================================
void timerCB(int v) {
    updateGame();
    glutPostRedisplay();
    glutTimerFunc(FRAME_MS, timerCB, 0);
}

// ============================================================================
// Input Handlers
// ============================================================================
void keyDown(unsigned char k, int x, int y) {
    if (k == 27) exit(0); // ESC = quit

    if (k == 'p' || k == 'P') {
        if      (gState == STATE_PLAYING) gState = STATE_PAUSED;
        else if (gState == STATE_PAUSED)  gState = STATE_PLAYING;
        return;
    }
    if (k == 'r' || k == 'R') { resetGame(); return; }
    if (k == 'h' || k == 'H') { showHB = !showHB; return; }

    if (k == ' ') {
        if (gState == STATE_START)    { gState = STATE_PLAYING; return; }
        if (gState == STATE_GAME_OVER){ resetGame(); return; }
        if (gState == STATE_PLAYING && grounded) {
            rVY = JUMP_FORCE; grounded = false;
            spawnDust(rX+10.f, GROUND_Y, 6, -2.f, 1.2f);
        }
    }
}

void keyUp(unsigned char k, int x, int y) {
    // Variable jump height: releasing space early cuts upward velocity
    if (k == ' ' && !grounded && rVY > 4.f) rVY *= 0.55f;
}

void specialDown(int k, int x, int y) {
    if (k == GLUT_KEY_UP) {
        if (gState == STATE_START)    { gState = STATE_PLAYING; return; }
        if (gState == STATE_GAME_OVER){ resetGame(); return; }
        if (gState == STATE_PLAYING && grounded) {
            rVY = JUMP_FORCE; grounded = false;
            spawnDust(rX+10.f, GROUND_Y, 6, -2.f, 1.2f);
        }
    } else if (k == GLUT_KEY_DOWN && gState == STATE_PLAYING) {
        ducking = true;
    }
}

void specialUp(int k, int x, int y) {
    if (k == GLUT_KEY_DOWN) ducking = false;
    if (k == GLUT_KEY_UP && !grounded && rVY > 4.f) rVY *= 0.55f;
}

// ============================================================================
// Reshape / Window Resize
// ============================================================================
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H); // 2D Orthographic Projection
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ============================================================================
// Main Entry Point
// ============================================================================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Chrono Run: DN Dash v2.0 - City Edition | CSE-402");

    // Enable alpha blending for particle effects and HUD overlays
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initWorld();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc(FRAME_MS, timerCB, 0);

    std::cout << "============================================================\n";
    std::cout << "  CHRONO RUN: DN DASH  v2.0 - City Edition\n";
    std::cout << "  CSE-402 Computer Graphics & Image Processing Lab\n";
    std::cout << "  Metropolitan University, Sylhet\n";
    std::cout << "  Nahdiul Islam Rony (231-115-069)\n";
    std::cout << "  Abu Ubayda         (231-115-080)\n";
    std::cout << "============================================================\n";
    std::cout << "  Controls:\n";
    std::cout << "    [SPACE] / [UP]   Jump (variable height - release early for short hop)\n";
    std::cout << "    [DOWN]           Duck / Slide under drones\n";
    std::cout << "    [P]              Pause / Resume\n";
    std::cout << "    [R]              Restart\n";
    std::cout << "    [H]              Toggle AABB Hitbox Debugger\n";
    std::cout << "    [ESC]            Exit\n";
    std::cout << "============================================================\n";

    glutMainLoop();
    return 0;
}
