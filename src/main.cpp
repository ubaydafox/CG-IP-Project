/*
 ============================================================================
  Project Title : CHRONO RUN: DN DASH
  Course        : Computer Graphics & Image Processing Lab (CSE-402)
  Institution   : Metropolitan University, Sylhet
  Lecturer      : Shrabanti Chowdhury

  Submitted by  :
    1. Nahdiul Islam Rony (ID: 231-115-069)
    2. Abu Ubayda         (ID: 231-115-080)
  Section       : B, 58th Batch, Dept. of CSE

  Description   :
    An interactive 2D endless runner desktop game built with C++, OpenGL,
    and FreeGLUT. Features a seamless dynamic day-night cycle, parallax
    background scrolling, realistic jump physics, ducking mechanics,
    procedurally spawned ground & aerial obstacles, AABB collision detection,
    particle effects, sound feedback, and comprehensive HUD score tracking.
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
#include <thread>
#include <windows.h>

// -----------------------------------------------------------------------------
// Constants & Configuration
// -----------------------------------------------------------------------------
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 600;
const float GROUND_Y    = 110.0f;
const int TARGET_FPS    = 60;
const int FRAME_DELAY_MS = 1000 / TARGET_FPS;

// Day-Night Cycle Period in seconds
const float CYCLE_DURATION = 40.0f; // 40 seconds for full Dawn -> Day -> Sunset -> Night cycle

// -----------------------------------------------------------------------------
// Data Structures
// -----------------------------------------------------------------------------
struct ColorRGB {
    float r, g, b;
    ColorRGB(float r = 0.0f, float g = 0.0f, float b = 0.0f) : r(r), g(g), b(b) {}
    ColorRGB operator*(float scalar) const {
        return ColorRGB(r * scalar, g * scalar, b * scalar);
    }
    ColorRGB operator+(const ColorRGB& o) const {
        return ColorRGB(r + o.r, g + o.g, b + o.b);
    }
};

ColorRGB lerp(const ColorRGB& a, const ColorRGB& b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return ColorRGB(
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t
    );
}

struct RectAABB {
    float x, y, width, height;
};

bool checkAABBCollision(const RectAABB& a, const RectAABB& b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

// Particle for dust, jump landing, and crash explosion
struct Particle {
    float x, y;
    float vx, vy;
    float life;      // 1.0 down to 0.0
    float decay;
    float size;
    ColorRGB color;
};

// Background Cloud
struct Cloud {
    float x, y;
    float speed;
    float scale;
};

// Ground detail (pebbles, grass tufts)
struct GroundDetail {
    float x, y;
    float width, height;
    int type; // 0 = pebble, 1 = dash line, 2 = grass tuft
};

// Obstacle Types
enum ObstacleType {
    OBS_CACTUS_SMALL = 0,
    OBS_CACTUS_DOUBLE,
    OBS_CACTUS_TALL,
    OBS_ROCK,
    OBS_BIRD_HIGH,   // Duck under!
    OBS_BIRD_LOW     // Jump over!
};

struct Obstacle {
    ObstacleType type;
    float x, y;
    float width, height;
    float animTimer; // for wing flapping or animated features
    bool passed;     // scored?
};

// Star in the night sky
struct Star {
    float x, y;
    float size;
    float twinkleOffset;
};

// Game States
enum GameState {
    STATE_START = 0,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
};

// Day-Night Stages
enum DayNightPhase {
    PHASE_DAWN = 0,
    PHASE_DAY,
    PHASE_SUNSET,
    PHASE_NIGHT
};

// -----------------------------------------------------------------------------
// Global Variables & State
// -----------------------------------------------------------------------------
GameState gameState = STATE_START;

// Runner State
float runnerX = 140.0f;
float runnerY = GROUND_Y;
float runnerVelocityY = 0.0f;
const float JUMP_FORCE = 15.2f;
const float GRAVITY    = 0.78f;
bool isGrounded = true;
bool isDucking  = false;
float runAnimPhase = 0.0f;
float runnerTilt = 0.0f;

// Dimensions
const float RUNNER_WIDTH_STAND  = 44.0f;
const float RUNNER_HEIGHT_STAND = 70.0f;
const float RUNNER_WIDTH_DUCK   = 64.0f;
const float RUNNER_HEIGHT_DUCK  = 38.0f;

// Hitbox shrinkage for fair, responsive gameplay
const float HITBOX_PADDING_X = 6.0f;
const float HITBOX_PADDING_Y = 4.0f;

// Game Progression
float gameSpeed = 6.8f;
const float BASE_SPEED = 6.8f;
const float MAX_SPEED  = 16.0f;
float score = 0.0f;
int highScore = 0;
float cycleTimer = 0.0f;
DayNightPhase currentPhase = PHASE_DAY;
float ambientLight = 1.0f;

// Debug and options
bool showHitboxes = false;
bool soundEnabled = true;

// Collections
std::vector<Obstacle> obstacles;
std::vector<Particle> particles;
std::vector<Cloud> clouds;
std::vector<GroundDetail> groundDetails;
std::vector<Star> stars;

// Parallax Mountain Points
const int MOUNTAIN_COUNT = 16;
float farMountainX[MOUNTAIN_COUNT];
float farMountainH[MOUNTAIN_COUNT];
float nearHillX[MOUNTAIN_COUNT];
float nearHillH[MOUNTAIN_COUNT];

// Sound System (Non-blocking async sound using Windows Beep)
void playBeepSound(int freq, int durationMs) {
    if (!soundEnabled) return;
    std::thread([freq, durationMs]() {
        Beep(freq, durationMs);
    }).detach();
}

void soundJump() {
    if (!soundEnabled) return;
    std::thread([]() {
        Beep(520, 45);
        Beep(780, 50);
    }).detach();
}

void soundDuck() {
    if (!soundEnabled) return;
    std::thread([]() {
        Beep(330, 40);
    }).detach();
}

void soundScoreMilestone() {
    if (!soundEnabled) return;
    std::thread([]() {
        Beep(880, 60);
        Beep(1174, 90);
    }).detach();
}

void soundGameOver() {
    if (!soundEnabled) return;
    std::thread([]() {
        Beep(440, 100);
        Beep(330, 120);
        Beep(220, 250);
    }).detach();
}

// -----------------------------------------------------------------------------
// Drawing Helper Primitives
// -----------------------------------------------------------------------------
void drawFilledCircle(float cx, float cy, float r, int segments = 24) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float angle = i * 2.0f * M_PI / segments;
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r);
    }
    glEnd();
}

void drawCircleOutline(float cx, float cy, float r, int segments = 24) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float angle = i * 2.0f * M_PI / segments;
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r);
    }
    glEnd();
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawRectOutline(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Bitmap Text Helper
void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

// -----------------------------------------------------------------------------
// Particle Emitters
// -----------------------------------------------------------------------------
void spawnDust(float x, float y, int count = 4, float baseVx = -2.0f, float baseVy = 1.0f) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x + (rand() % 12 - 6);
        p.y = y + (rand() % 6);
        p.vx = baseVx + ((rand() % 100) / 100.0f - 0.5f) * 2.0f;
        p.vy = baseVy + ((rand() % 100) / 100.0f) * 2.5f;
        p.life = 1.0f;
        p.decay = 0.04f + ((rand() % 100) / 100.0f) * 0.04f;
        p.size = 2.5f + (rand() % 3);
        p.color = ColorRGB(0.75f, 0.70f, 0.65f);
        particles.push_back(p);
    }
}

void spawnCrashExplosion(float x, float y) {
    for (int i = 0; i < 40; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        float angle = (rand() % 360) * M_PI / 180.0f;
        float speed = 2.0f + (rand() % 60) / 10.0f;
        p.vx = cosf(angle) * speed;
        p.vy = sinf(angle) * speed;
        p.life = 1.0f;
        p.decay = 0.02f + (rand() % 20) / 1000.0f;
        p.size = 3.0f + (rand() % 4);
        if (i % 3 == 0) p.color = ColorRGB(1.0f, 0.3f, 0.1f);
        else if (i % 3 == 1) p.color = ColorRGB(1.0f, 0.8f, 0.2f);
        else p.color = ColorRGB(0.9f, 0.9f, 0.9f);
        particles.push_back(p);
    }
}

// -----------------------------------------------------------------------------
// World Initialization
// -----------------------------------------------------------------------------
void initWorld() {
    srand(static_cast<unsigned int>(time(NULL)));

    // Stars
    stars.clear();
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = (rand() % WINDOW_WIDTH);
        s.y = GROUND_Y + 120.0f + (rand() % (WINDOW_HEIGHT - (int)GROUND_Y - 140));
        s.size = 1.0f + (rand() % 3) * 0.7f;
        s.twinkleOffset = (rand() % 100) / 10.0f;
        stars.push_back(s);
    }

    // Clouds
    clouds.clear();
    for (int i = 0; i < 5; ++i) {
        Cloud c;
        c.x = i * (WINDOW_WIDTH / 5.0f) + (rand() % 80);
        c.y = 360.0f + (rand() % 160);
        c.speed = 0.6f + (rand() % 50) / 100.0f;
        c.scale = 0.8f + (rand() % 50) / 100.0f;
        clouds.push_back(c);
    }

    // Ground details
    groundDetails.clear();
    for (int i = 0; i < 28; ++i) {
        GroundDetail gd;
        gd.x = (rand() % WINDOW_WIDTH);
        gd.y = (rand() % (int)GROUND_Y);
        gd.width = 6.0f + (rand() % 14);
        gd.height = 2.0f + (rand() % 3);
        gd.type = rand() % 3;
        groundDetails.push_back(gd);
    }

    // Parallax Mountains
    float farStep = (float)WINDOW_WIDTH / (MOUNTAIN_COUNT - 2);
    for (int i = 0; i < MOUNTAIN_COUNT; ++i) {
        farMountainX[i] = i * farStep;
        farMountainH[i] = 130.0f + (rand() % 80);
        nearHillX[i]    = i * farStep;
        nearHillH[i]    = 70.0f + (rand() % 55);
    }

    obstacles.clear();
    particles.clear();

    runnerY = GROUND_Y;
    runnerVelocityY = 0.0f;
    isGrounded = true;
    isDucking = false;
    runAnimPhase = 0.0f;
    runnerTilt = 0.0f;

    gameSpeed = BASE_SPEED;
    score = 0.0f;
    cycleTimer = 0.0f;
}

// Reset / Restart
void resetGame() {
    obstacles.clear();
    particles.clear();
    runnerY = GROUND_Y;
    runnerVelocityY = 0.0f;
    isGrounded = true;
    isDucking = false;
    runAnimPhase = 0.0f;
    runnerTilt = 0.0f;
    gameSpeed = BASE_SPEED;
    score = 0.0f;
    cycleTimer = 0.0f;
    gameState = STATE_PLAYING;
}

// -----------------------------------------------------------------------------
// Procedural Obstacle Spawning
// -----------------------------------------------------------------------------
void spawnObstacle() {
    Obstacle obs;
    obs.x = WINDOW_WIDTH + 40.0f;
    obs.animTimer = 0.0f;
    obs.passed = false;

    // Determine type: aerial bird or ground obstacle
    // Birds start appearing once score > 150
    int choice = rand() % 100;
    if (score > 150.0f && choice < 32) {
        if (rand() % 2 == 0) {
            // High bird: Duck under!
            obs.type = OBS_BIRD_HIGH;
            obs.width = 46.0f;
            obs.height = 30.0f;
            obs.y = GROUND_Y + 44.0f; // High enough to duck under safely
        } else {
            // Low bird: Jump over!
            obs.type = OBS_BIRD_LOW;
            obs.width = 46.0f;
            obs.height = 30.0f;
            obs.y = GROUND_Y + 14.0f;
        }
    } else {
        int gType = rand() % 4;
        if (gType == 0) {
            obs.type = OBS_CACTUS_SMALL;
            obs.width = 24.0f;
            obs.height = 46.0f;
            obs.y = GROUND_Y;
        } else if (gType == 1) {
            obs.type = OBS_CACTUS_DOUBLE;
            obs.width = 42.0f;
            obs.height = 54.0f;
            obs.y = GROUND_Y;
        } else if (gType == 2) {
            obs.type = OBS_CACTUS_TALL;
            obs.width = 28.0f;
            obs.height = 68.0f;
            obs.y = GROUND_Y;
        } else {
            obs.type = OBS_ROCK;
            obs.width = 38.0f;
            obs.height = 26.0f;
            obs.y = GROUND_Y;
        }
    }

    obstacles.push_back(obs);
}

// -----------------------------------------------------------------------------
// Day-Night Environment Rendering
// -----------------------------------------------------------------------------
void renderSkyAndLighting() {
    // Phase calculation: cycleTimer loops over CYCLE_DURATION
    float t = fmodf(cycleTimer, CYCLE_DURATION) / CYCLE_DURATION; // 0.0 to 1.0

    // Keyframes for Sky Colors:
    // 0.00 - 0.25: Dawn -> Day
    // 0.25 - 0.50: Day -> Sunset
    // 0.50 - 0.75: Sunset -> Deep Night
    // 0.75 - 1.00: Deep Night -> Dawn

    ColorRGB dawnTop(0.20f, 0.28f, 0.58f),    dawnBot(0.96f, 0.52f, 0.38f);
    ColorRGB dayTop(0.18f, 0.58f, 0.92f),     dayBot(0.68f, 0.88f, 1.00f);
    ColorRGB sunsetTop(0.36f, 0.14f, 0.44f),  sunsetBot(0.98f, 0.38f, 0.18f);
    ColorRGB nightTop(0.02f, 0.04f, 0.12f),   nightBot(0.08f, 0.12f, 0.24f);

    ColorRGB skyTop, skyBot;
    float sunMoonAngle = 0.0f; // radians for celestial arc
    float nightFactor = 0.0f;  // 0.0 at noon, 1.0 at midnight

    if (t < 0.25f) {
        // Dawn to Day
        float f = t / 0.25f;
        skyTop = lerp(dawnTop, dayTop, f);
        skyBot = lerp(dawnBot, dayBot, f);
        ambientLight = 0.75f + 0.25f * f;
        currentPhase = (f < 0.5f) ? PHASE_DAWN : PHASE_DAY;
        sunMoonAngle = f * (M_PI * 0.5f); // 0 to pi/2
        nightFactor = (1.0f - f) * 0.4f;
    } else if (t < 0.50f) {
        // Day to Sunset
        float f = (t - 0.25f) / 0.25f;
        skyTop = lerp(dayTop, sunsetTop, f);
        skyBot = lerp(dayBot, sunsetBot, f);
        ambientLight = 1.0f - 0.25f * f;
        currentPhase = (f < 0.6f) ? PHASE_DAY : PHASE_SUNSET;
        sunMoonAngle = (M_PI * 0.5f) + f * (M_PI * 0.5f); // pi/2 to pi
        nightFactor = f * 0.2f;
    } else if (t < 0.75f) {
        // Sunset to Night
        float f = (t - 0.50f) / 0.25f;
        skyTop = lerp(sunsetTop, nightTop, f);
        skyBot = lerp(sunsetBot, nightBot, f);
        ambientLight = 0.75f - 0.35f * f;
        currentPhase = (f < 0.4f) ? PHASE_SUNSET : PHASE_NIGHT;
        sunMoonAngle = f * (M_PI * 0.5f); // Moon rises
        nightFactor = 0.2f + 0.8f * f;
    } else {
        // Night to Dawn
        float f = (t - 0.75f) / 0.25f;
        skyTop = lerp(nightTop, dawnTop, f);
        skyBot = lerp(nightBot, dawnBot, f);
        ambientLight = 0.40f + 0.35f * f;
        currentPhase = (f < 0.7f) ? PHASE_NIGHT : PHASE_DAWN;
        sunMoonAngle = (M_PI * 0.5f) + f * (M_PI * 0.5f);
        nightFactor = 1.0f - f;
    }

    // Render Sky Gradient (from horizon GROUND_Y up to WINDOW_HEIGHT)
    glBegin(GL_QUADS);
    glColor3f(skyBot.r, skyBot.g, skyBot.b);
    glVertex2f(0.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH, GROUND_Y);
    glColor3f(skyTop.r, skyTop.g, skyTop.b);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0.0f, WINDOW_HEIGHT);
    glEnd();

    // Render Twinkling Stars (intensity scaled by nightFactor)
    if (nightFactor > 0.05f) {
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        for (const auto& s : stars) {
            float twinkle = 0.5f + 0.5f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.005f + s.twinkleOffset);
            float alpha = nightFactor * twinkle;
            glColor3f(alpha * 0.95f, alpha * 0.95f, alpha * 1.0f);
            glVertex2f(s.x, s.y);
        }
        glEnd();
    }

    // Render Celestial Bodies (Sun and Moon)
    // Celestial arc calculation:
    // Sun active during t in [0.0, 0.50]
    // Moon active during t in [0.50, 1.00]
    if (t < 0.50f) {
        // Sun
        float sunProgress = t / 0.50f; // 0 to 1
        float sunAngle = sunProgress * M_PI; // 0 to pi
        float sunX = 100.0f + sunProgress * (WINDOW_WIDTH - 200.0f);
        float sunY = GROUND_Y + 40.0f + sinf(sunAngle) * 360.0f;

        // Glowing outer halo
        glColor3f(1.0f, 0.85f, 0.35f);
        drawFilledCircle(sunX, sunY, 32.0f, 28);
        glColor3f(1.0f, 0.96f, 0.60f);
        drawFilledCircle(sunX, sunY, 24.0f, 24);

        // Rotating Sun Rays
        float rayTime = glutGet(GLUT_ELAPSED_TIME) * 0.002f;
        glColor3f(1.0f, 0.82f, 0.20f);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        for (int i = 0; i < 8; ++i) {
            float a = rayTime + i * (M_PI * 0.25f);
            glVertex2f(sunX + cosf(a) * 34.0f, sunY + sinf(a) * 34.0f);
            glVertex2f(sunX + cosf(a) * 44.0f, sunY + sinf(a) * 44.0f);
        }
        glEnd();
        glLineWidth(1.0f);
    } else {
        // Moon
        float moonProgress = (t - 0.50f) / 0.50f; // 0 to 1
        float moonAngle = moonProgress * M_PI;
        float moonX = 100.0f + moonProgress * (WINDOW_WIDTH - 200.0f);
        float moonY = GROUND_Y + 40.0f + sinf(moonAngle) * 360.0f;

        // Outer soft glow
        glColor3f(0.85f, 0.90f, 1.0f);
        drawFilledCircle(moonX, moonY, 26.0f, 26);
        glColor3f(0.96f, 0.98f, 1.0f);
        drawFilledCircle(moonX, moonY, 22.0f, 24);

        // Moon Craters
        glColor3f(0.80f, 0.85f, 0.92f);
        drawFilledCircle(moonX - 6.0f, moonY + 4.0f, 4.5f, 12);
        drawFilledCircle(moonX + 7.0f, moonY - 3.0f, 3.5f, 10);
        drawFilledCircle(moonX + 2.0f, moonY + 8.0f, 2.5f, 10);
    }

    // Render Distant Mountain Silhouettes (Far layer, slow scroll)
    ColorRGB mtnColor = lerp(ColorRGB(0.40f, 0.45f, 0.60f), ColorRGB(0.08f, 0.10f, 0.22f), nightFactor);
    glColor3f(mtnColor.r, mtnColor.g, mtnColor.b);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i < MOUNTAIN_COUNT; ++i) {
        glVertex2f(farMountainX[i], GROUND_Y + farMountainH[i]);
        glVertex2f(farMountainX[i], GROUND_Y);
    }
    glEnd();

    // Render Near Hills (Middle layer)
    ColorRGB hillColor = lerp(ColorRGB(0.32f, 0.50f, 0.42f), ColorRGB(0.06f, 0.14f, 0.18f), nightFactor);
    glColor3f(hillColor.r, hillColor.g, hillColor.b);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i < MOUNTAIN_COUNT; ++i) {
        glVertex2f(nearHillX[i], GROUND_Y + nearHillH[i]);
        glVertex2f(nearHillX[i], GROUND_Y);
    }
    glEnd();

    // Render Clouds (Floating puffy clouds)
    for (const auto& c : clouds) {
        ColorRGB cloudCol = lerp(ColorRGB(1.0f, 1.0f, 1.0f), ColorRGB(0.25f, 0.28f, 0.40f), nightFactor);
        glColor3f(cloudCol.r, cloudCol.g, cloudCol.b);
        float s = c.scale;
        drawFilledCircle(c.x, c.y, 22.0f * s, 16);
        drawFilledCircle(c.x + 18.0f * s, c.y + 6.0f * s, 26.0f * s, 18);
        drawFilledCircle(c.x + 40.0f * s, c.y, 20.0f * s, 16);
        drawFilledCircle(c.x + 22.0f * s, c.y - 6.0f * s, 18.0f * s, 14);
    }
}

// -----------------------------------------------------------------------------
// Ground Terrain Rendering
// -----------------------------------------------------------------------------
void renderGround() {
    float nightFactor = (currentPhase == PHASE_NIGHT) ? 0.8f : (currentPhase == PHASE_SUNSET ? 0.4f : 0.0f);
    ColorRGB groundBedTop = lerp(ColorRGB(0.85f, 0.76f, 0.58f), ColorRGB(0.18f, 0.18f, 0.28f), nightFactor);
    ColorRGB groundBedBot = lerp(ColorRGB(0.58f, 0.48f, 0.35f), ColorRGB(0.10f, 0.10f, 0.18f), nightFactor);

    // Bedrock gradient below ground line
    glBegin(GL_QUADS);
    glColor3f(groundBedTop.r, groundBedTop.g, groundBedTop.b);
    glVertex2f(0.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH, GROUND_Y);
    glColor3f(groundBedBot.r, groundBedBot.g, groundBedBot.b);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    // Ground Surface Border Line
    ColorRGB lineCol = lerp(ColorRGB(0.35f, 0.28f, 0.18f), ColorRGB(0.30f, 0.35f, 0.50f), nightFactor);
    glColor3f(lineCol.r, lineCol.g, lineCol.b);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, GROUND_Y);
    glVertex2f(WINDOW_WIDTH, GROUND_Y);
    glEnd();
    glLineWidth(1.0f);

    // Dynamic Scrolling Ground Details
    for (const auto& gd : groundDetails) {
        if (gd.type == 0) {
            // Pebble
            glColor3f(lineCol.r * 1.1f, lineCol.g * 1.1f, lineCol.b * 1.1f);
            drawFilledCircle(gd.x, gd.y, gd.height, 8);
        } else if (gd.type == 1) {
            // Dirt Dash Line
            glColor3f(lineCol.r * 0.9f, lineCol.g * 0.9f, lineCol.b * 0.9f);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex2f(gd.x, gd.y);
            glVertex2f(gd.x + gd.width, gd.y);
            glEnd();
            glLineWidth(1.0f);
        } else {
            // Small Grass Tuft
            glColor3f(0.38f * (1.0f - nightFactor * 0.5f), 0.65f * (1.0f - nightFactor * 0.5f), 0.25f);
            glBegin(GL_LINES);
            glVertex2f(gd.x, GROUND_Y);
            glVertex2f(gd.x - 3.0f, GROUND_Y + 7.0f);
            glVertex2f(gd.x, GROUND_Y);
            glVertex2f(gd.x + 1.0f, GROUND_Y + 9.0f);
            glVertex2f(gd.x, GROUND_Y);
            glVertex2f(gd.x + 4.0f, GROUND_Y + 6.0f);
            glEnd();
        }
    }
}

// -----------------------------------------------------------------------------
// Runner Character Rendering
// -----------------------------------------------------------------------------
void renderRunner() {
    glPushMatrix();
    glTranslatef(runnerX, runnerY, 0.0f);

    if (runnerTilt != 0.0f) {
        glRotatef(runnerTilt, 0.0f, 0.0f, 1.0f);
    }

    float nightFactor = (currentPhase == PHASE_NIGHT) ? 0.7f : 0.0f;
    ColorRGB skinColor = lerp(ColorRGB(0.98f, 0.82f, 0.68f), ColorRGB(0.65f, 0.60f, 0.70f), nightFactor);
    ColorRGB shirtColor = ColorRGB(0.95f, 0.22f, 0.28f);  // Athletic crimson
    ColorRGB pantsColor = ColorRGB(0.18f, 0.24f, 0.38f);  // Navy shorts
    ColorRGB shoeColor  = ColorRGB(1.00f, 0.90f, 0.20f);  // Vibrant yellow shoes
    ColorRGB visorColor = ColorRGB(0.20f, 0.90f, 1.00f);  // Cyan cyber visor

    if (isDucking && isGrounded) {
        // ---------------------------------------------------------------------
        // DUCKING POSE (Low slide crouch)
        // ---------------------------------------------------------------------
        // Sliding dust emission
        if (rand() % 3 == 0) {
            spawnDust(runnerX - 10.0f, GROUND_Y + 2.0f, 1, -4.0f, 0.5f);
        }

        // Tilted crouching torso
        glColor3f(shirtColor.r, shirtColor.g, shirtColor.b);
        drawRect(0.0f, 10.0f, 38.0f, 18.0f);

        // Head tucked forward
        glColor3f(skinColor.r, skinColor.g, skinColor.b);
        drawFilledCircle(38.0f, 20.0f, 11.0f, 16);

        // Cyber Visor
        glColor3f(visorColor.r, visorColor.g, visorColor.b);
        drawRect(42.0f, 19.0f, 8.0f, 5.0f);

        // Streamer Headband trailing back
        glColor3f(0.95f, 0.85f, 0.15f);
        glBegin(GL_TRIANGLES);
        glVertex2f(32.0f, 24.0f);
        glVertex2f(10.0f, 27.0f + sinf(runAnimPhase * 2.0f) * 3.0f);
        glVertex2f(32.0f, 21.0f);
        glEnd();

        // Crouched bent legs
        glColor3f(pantsColor.r, pantsColor.g, pantsColor.b);
        drawRect(-12.0f, 6.0f, 22.0f, 12.0f);

        // Sliding shoes on ground
        glColor3f(shoeColor.r, shoeColor.g, shoeColor.b);
        drawRect(-16.0f, 0.0f, 20.0f, 7.0f);
        drawRect(18.0f, 0.0f, 16.0f, 7.0f);

        // Arm bracing forward
        glColor3f(skinColor.r, skinColor.g, skinColor.b);
        drawRect(20.0f, 12.0f, 14.0f, 6.0f);
    } else {
        // ---------------------------------------------------------------------
        // STANDING / RUNNING / JUMPING POSE
        // ---------------------------------------------------------------------
        float legSwing = isGrounded ? sinf(runAnimPhase) * 22.0f : -15.0f;
        float armSwing = isGrounded ? cosf(runAnimPhase) * 24.0f : 20.0f;

        // --- LEGS ---
        // Back leg (Pants + Calf + Shoe)
        glPushMatrix();
        glTranslatef(14.0f, 24.0f, 0.0f);
        glRotatef(-legSwing, 0.0f, 0.0f, 1.0f);
        glColor3f(pantsColor.r * 0.8f, pantsColor.g * 0.8f, pantsColor.b * 0.8f);
        drawRect(-4.0f, -14.0f, 8.0f, 14.0f);
        glColor3f(skinColor.r * 0.85f, skinColor.g * 0.85f, skinColor.b * 0.85f);
        drawRect(-3.5f, -22.0f, 7.0f, 9.0f);
        glColor3f(shoeColor.r * 0.85f, shoeColor.g * 0.85f, shoeColor.b * 0.85f);
        drawRect(-4.0f, -26.0f, 14.0f, 6.0f);
        glPopMatrix();

        // --- TORSO ---
        glColor3f(shirtColor.r, shirtColor.g, shirtColor.b);
        drawRect(4.0f, 24.0f, 20.0f, 28.0f);
        // Athletic stripe on jersey
        glColor3f(1.0f, 1.0f, 1.0f);
        drawRect(12.0f, 24.0f, 4.0f, 28.0f);

        // --- BACK ARM ---
        glPushMatrix();
        glTranslatef(8.0f, 46.0f, 0.0f);
        glRotatef(-armSwing, 0.0f, 0.0f, 1.0f);
        glColor3f(skinColor.r * 0.85f, skinColor.g * 0.85f, skinColor.b * 0.85f);
        drawRect(-3.0f, -16.0f, 6.0f, 16.0f);
        glPopMatrix();

        // --- FRONT LEG ---
        glPushMatrix();
        glTranslatef(14.0f, 24.0f, 0.0f);
        glRotatef(legSwing, 0.0f, 0.0f, 1.0f);
        glColor3f(pantsColor.r, pantsColor.g, pantsColor.b);
        drawRect(-4.0f, -14.0f, 8.0f, 14.0f);
        glColor3f(skinColor.r, skinColor.g, skinColor.b);
        drawRect(-3.5f, -22.0f, 7.0f, 9.0f);
        glColor3f(shoeColor.r, shoeColor.g, shoeColor.b);
        drawRect(-4.0f, -26.0f, 14.0f, 6.0f);
        glPopMatrix();

        // --- HEAD ---
        glColor3f(skinColor.r, skinColor.g, skinColor.b);
        drawFilledCircle(14.0f, 58.0f, 11.0f, 18);

        // Cyber Visor
        glColor3f(visorColor.r, visorColor.g, visorColor.b);
        drawRect(18.0f, 57.0f, 9.0f, 5.0f);

        // Streamer Headband trailing back
        glColor3f(0.95f, 0.85f, 0.15f);
        glBegin(GL_TRIANGLES);
        glVertex2f(8.0f, 63.0f);
        glVertex2f(-16.0f, 64.0f + sinf(runAnimPhase * 2.0f) * 5.0f);
        glVertex2f(8.0f, 59.0f);
        glEnd();

        // --- FRONT ARM ---
        glPushMatrix();
        glTranslatef(18.0f, 46.0f, 0.0f);
        glRotatef(armSwing, 0.0f, 0.0f, 1.0f);
        glColor3f(skinColor.r, skinColor.g, skinColor.b);
        drawRect(-3.0f, -16.0f, 6.0f, 16.0f);
        // Fist/glove
        glColor3f(pantsColor.r, pantsColor.g, pantsColor.b);
        drawFilledCircle(0.0f, -17.0f, 4.0f, 8);
        glPopMatrix();
    }

    glPopMatrix();
}

// -----------------------------------------------------------------------------
// Obstacles Rendering
// -----------------------------------------------------------------------------
void renderObstacles() {
    float nightFactor = (currentPhase == PHASE_NIGHT) ? 0.7f : (currentPhase == PHASE_SUNSET ? 0.3f : 0.0f);

    for (const auto& obs : obstacles) {
        glPushMatrix();
        glTranslatef(obs.x, obs.y, 0.0f);

        if (obs.type == OBS_CACTUS_SMALL) {
            ColorRGB cactusCol = lerp(ColorRGB(0.18f, 0.62f, 0.28f), ColorRGB(0.10f, 0.32f, 0.20f), nightFactor);
            glColor3f(cactusCol.r, cactusCol.g, cactusCol.b);
            // Main trunk
            drawRect(8.0f, 0.0f, 10.0f, 46.0f);
            drawFilledCircle(13.0f, 46.0f, 5.0f, 12);
            // Left branch
            drawRect(0.0f, 16.0f, 8.0f, 6.0f);
            drawRect(0.0f, 22.0f, 6.0f, 14.0f);
            drawFilledCircle(3.0f, 36.0f, 3.0f, 8);
            // Right branch
            drawRect(18.0f, 22.0f, 8.0f, 6.0f);
            drawRect(20.0f, 28.0f, 6.0f, 12.0f);
            drawFilledCircle(23.0f, 40.0f, 3.0f, 8);
        } else if (obs.type == OBS_CACTUS_DOUBLE) {
            ColorRGB cactusCol = lerp(ColorRGB(0.15f, 0.58f, 0.25f), ColorRGB(0.08f, 0.28f, 0.18f), nightFactor);
            glColor3f(cactusCol.r, cactusCol.g, cactusCol.b);
            // First taller cactus
            drawRect(6.0f, 0.0f, 11.0f, 54.0f);
            drawFilledCircle(11.5f, 54.0f, 5.5f, 12);
            drawRect(0.0f, 24.0f, 7.0f, 6.0f);
            drawRect(0.0f, 30.0f, 6.0f, 14.0f);
            // Second smaller cactus
            drawRect(24.0f, 0.0f, 10.0f, 40.0f);
            drawFilledCircle(29.0f, 40.0f, 5.0f, 12);
            drawRect(33.0f, 18.0f, 7.0f, 5.0f);
            drawRect(35.0f, 23.0f, 5.0f, 10.0f);
        } else if (obs.type == OBS_CACTUS_TALL) {
            ColorRGB cactusCol = lerp(ColorRGB(0.14f, 0.54f, 0.24f), ColorRGB(0.07f, 0.26f, 0.16f), nightFactor);
            glColor3f(cactusCol.r, cactusCol.g, cactusCol.b);
            // Very tall cactus with multiple arms
            drawRect(10.0f, 0.0f, 12.0f, 68.0f);
            drawFilledCircle(16.0f, 68.0f, 6.0f, 12);
            // Left low arm
            drawRect(0.0f, 20.0f, 10.0f, 7.0f);
            drawRect(0.0f, 27.0f, 7.0f, 18.0f);
            drawFilledCircle(3.5f, 45.0f, 3.5f, 8);
            // Right high arm
            drawRect(22.0f, 34.0f, 10.0f, 7.0f);
            drawRect(25.0f, 41.0f, 7.0f, 16.0f);
            drawFilledCircle(28.5f, 57.0f, 3.5f, 8);
        } else if (obs.type == OBS_ROCK) {
            // Jagged Boulder
            ColorRGB rockCol = lerp(ColorRGB(0.55f, 0.50f, 0.45f), ColorRGB(0.20f, 0.22f, 0.30f), nightFactor);
            glColor3f(rockCol.r, rockCol.g, rockCol.b);
            glBegin(GL_POLYGON);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(6.0f, 18.0f);
            glVertex2f(16.0f, 26.0f);
            glVertex2f(28.0f, 22.0f);
            glVertex2f(38.0f, 12.0f);
            glVertex2f(36.0f, 0.0f);
            glEnd();
            // Highlight facet
            glColor3f(rockCol.r * 1.25f, rockCol.g * 1.25f, rockCol.b * 1.25f);
            glBegin(GL_TRIANGLES);
            glVertex2f(6.0f, 18.0f);
            glVertex2f(16.0f, 26.0f);
            glVertex2f(20.0f, 10.0f);
            glEnd();
        } else if (obs.type == OBS_BIRD_HIGH || obs.type == OBS_BIRD_LOW) {
            // Flying Pterodactyl / Cyber Bird
            ColorRGB birdCol = lerp(ColorRGB(0.85f, 0.30f, 0.25f), ColorRGB(0.40f, 0.20f, 0.45f), nightFactor);
            glColor3f(birdCol.r, birdCol.g, birdCol.b);

            // Flapping wing calculation
            float wingPhase = sinf(obs.animTimer * 12.0f);
            float wingOffset = wingPhase * 16.0f;

            // Torso / Body
            glBegin(GL_POLYGON);
            glVertex2f(12.0f, 10.0f);
            glVertex2f(30.0f, 12.0f);
            glVertex2f(38.0f, 16.0f);
            glVertex2f(30.0f, 18.0f);
            glVertex2f(10.0f, 14.0f);
            glVertex2f(0.0f, 12.0f); // tail
            glEnd();

            // Head and sharp beak
            glColor3f(0.95f, 0.75f, 0.15f); // Gold beak
            glBegin(GL_TRIANGLES);
            glVertex2f(36.0f, 18.0f);
            glVertex2f(46.0f, 14.0f);
            glVertex2f(36.0f, 12.0f);
            glEnd();

            // Glowing eye
            glColor3f(0.1f, 1.0f, 0.9f);
            drawFilledCircle(34.0f, 16.0f, 2.0f, 6);

            // Wings (Flapping dynamically)
            glColor3f(birdCol.r * 1.15f, birdCol.g * 1.15f, birdCol.b * 1.15f);
            glBegin(GL_TRIANGLES);
            glVertex2f(18.0f, 14.0f);
            glVertex2f(28.0f, 14.0f);
            glVertex2f(22.0f, 14.0f + wingOffset);
            glEnd();
        }

        glPopMatrix();
    }
}

// -----------------------------------------------------------------------------
// Particles Rendering
// -----------------------------------------------------------------------------
void renderParticles() {
    for (const auto& p : particles) {
        glColor3f(p.color.r * p.life, p.color.g * p.life, p.color.b * p.life);
        drawFilledCircle(p.x, p.y, p.size * p.life, 8);
    }
}

// -----------------------------------------------------------------------------
// Hitbox Visualizer (For Professor / Lab Grading Demo)
// -----------------------------------------------------------------------------
void renderHitboxes() {
    if (!showHitboxes) return;

    glLineWidth(2.0f);

    // Runner Hitbox (Bright Green)
    glColor3f(0.0f, 1.0f, 0.0f);
    RectAABB runnerBox;
    if (isDucking && isGrounded) {
        runnerBox = { runnerX + HITBOX_PADDING_X, runnerY, RUNNER_WIDTH_DUCK - HITBOX_PADDING_X * 2, RUNNER_HEIGHT_DUCK };
    } else {
        runnerBox = { runnerX + HITBOX_PADDING_X, runnerY, RUNNER_WIDTH_STAND - HITBOX_PADDING_X * 2, RUNNER_HEIGHT_STAND - HITBOX_PADDING_Y };
    }
    drawRectOutline(runnerBox.x, runnerBox.y, runnerBox.width, runnerBox.height);

    // Obstacle Hitboxes (Bright Red)
    glColor3f(1.0f, 0.0f, 0.0f);
    for (const auto& obs : obstacles) {
        drawRectOutline(obs.x + 3.0f, obs.y, obs.width - 6.0f, obs.height - 2.0f);
    }

    glLineWidth(1.0f);
}

// -----------------------------------------------------------------------------
// HUD & UI Overlays
// -----------------------------------------------------------------------------
void renderHUD() {
    // Top Bar Background Ribbon (Subtle translucent glass)
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
    drawRect(0.0f, WINDOW_HEIGHT - 48.0f, WINDOW_WIDTH, 48.0f);

    // Score & High Score Text
    std::stringstream ssScore, ssHigh, ssSpeed;
    ssScore << "SCORE: " << std::setfill('0') << std::setw(5) << (int)score;
    ssHigh  << "HI: "    << std::setfill('0') << std::setw(5) << highScore;
    ssSpeed << "SPEED: " << std::fixed << std::setprecision(1) << (gameSpeed / BASE_SPEED) << "x";

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(24.0f, WINDOW_HEIGHT - 32.0f, ssScore.str());

    glColor3f(0.85f, 0.85f, 0.85f);
    drawText(220.0f, WINDOW_HEIGHT - 32.0f, ssHigh.str());

    glColor3f(0.3f, 0.95f, 0.85f);
    drawText(400.0f, WINDOW_HEIGHT - 32.0f, ssSpeed.str());

    // Day/Night Indicator Icon and Text
    std::string phaseStr = "DAY";
    ColorRGB phaseColor(1.0f, 0.9f, 0.2f);
    if (currentPhase == PHASE_DAWN) {
        phaseStr = "DAWN";
        phaseColor = ColorRGB(1.0f, 0.6f, 0.3f);
    } else if (currentPhase == PHASE_SUNSET) {
        phaseStr = "SUNSET";
        phaseColor = ColorRGB(1.0f, 0.4f, 0.2f);
    } else if (currentPhase == PHASE_NIGHT) {
        phaseStr = "NIGHT";
        phaseColor = ColorRGB(0.5f, 0.8f, 1.0f);
    }

    // Mini Celestial Icon
    glColor3f(phaseColor.r, phaseColor.g, phaseColor.b);
    drawFilledCircle(585.0f, WINDOW_HEIGHT - 28.0f, 7.0f, 12);

    drawText(602.0f, WINDOW_HEIGHT - 32.0f, "TIME: " + phaseStr);

    // Hitbox Debug Indicator
    if (showHitboxes) {
        glColor3f(0.2f, 1.0f, 0.2f);
        drawText(750.0f, WINDOW_HEIGHT - 32.0f, "[HITBOX ON]");
    }

    // Sound Indicator
    if (soundEnabled) {
        glColor3f(0.7f, 1.0f, 0.7f);
        drawText(890.0f, WINDOW_HEIGHT - 32.0f, "SND: ON");
    } else {
        glColor3f(0.9f, 0.4f, 0.4f);
        drawText(890.0f, WINDOW_HEIGHT - 32.0f, "SND: OFF");
    }

    // Bottom Help Controls Ribbon
    glColor3f(0.85f, 0.85f, 0.85f);
    drawText(18.0f, 16.0f, "[SPACE / UP] Jump    [DOWN] Duck    [P] Pause    [R] Restart    [H] Hitboxes    [M] Mute    [ESC] Quit", GLUT_BITMAP_8_BY_13);

    // Overlay Screens based on Game State
    if (gameState == STATE_START) {
        // Dark translucent overlay
        glColor4f(0.05f, 0.08f, 0.16f, 0.82f);
        drawRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Title Banner Box
        glColor3f(0.12f, 0.16f, 0.26f);
        drawRect(150.0f, 150.0f, 700.0f, 320.0f);
        glColor3f(0.25f, 0.75f, 0.95f);
        drawRectOutline(150.0f, 150.0f, 700.0f, 320.0f);

        // Title
        glColor3f(1.0f, 0.85f, 0.20f);
        drawText(310.0f, 410.0f, "CHRONO RUN: DN DASH", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(0.75f, 0.85f, 0.95f);
        drawText(275.0f, 375.0f, "Computer Graphics & Image Processing Lab (CSE-402)");

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(295.0f, 325.0f, "Press [SPACEBAR] or [UP ARROW] to Start Running");

        glColor3f(0.85f, 0.85f, 0.85f);
        drawText(320.0f, 280.0f, "Controls: JUMP over rocks & low birds");
        drawText(320.0f, 255.0f, "          DUCK under high-flying birds");

        glColor3f(0.60f, 0.70f, 0.85f);
        drawText(220.0f, 190.0f, "Project by: Nahdiul Islam Rony (231-115-069) & Abu Ubayda (231-115-080)");
        drawText(340.0f, 168.0f, "Lecturer: Shrabanti Chowdhury | Section B");
    } else if (gameState == STATE_PAUSED) {
        glColor4f(0.05f, 0.08f, 0.15f, 0.65f);
        drawRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);

        glColor3f(0.15f, 0.20f, 0.32f);
        drawRect(300.0f, 220.0f, 400.0f, 160.0f);
        glColor3f(0.35f, 0.85f, 1.0f);
        drawRectOutline(300.0f, 220.0f, 400.0f, 160.0f);

        glColor3f(1.0f, 0.85f, 0.20f);
        drawText(430.0f, 325.0f, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(360.0f, 275.0f, "Press [P] to Resume  |  [R] to Restart");
    } else if (gameState == STATE_GAME_OVER) {
        // Red tinted game over overlay
        glColor4f(0.15f, 0.02f, 0.04f, 0.75f);
        drawRect(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);

        glColor3f(0.22f, 0.10f, 0.12f);
        drawRect(260.0f, 180.0f, 480.0f, 240.0f);
        glColor3f(0.95f, 0.25f, 0.30f);
        drawRectOutline(260.0f, 180.0f, 480.0f, 240.0f);

        // Game Over Banner
        glColor3f(1.0f, 0.2f, 0.25f);
        drawText(410.0f, 370.0f, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

        std::stringstream ssFinal;
        ssFinal << "FINAL SCORE: " << (int)score;
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(415.0f, 320.0f, ssFinal.str());

        if ((int)score >= highScore && score > 10.0f) {
            glColor3f(1.0f, 0.85f, 0.20f);
            drawText(400.0f, 285.0f, "*** NEW HIGH SCORE! ***");
        } else {
            std::stringstream ssHiCmp;
            ssHiCmp << "BEST SCORE: " << highScore;
            glColor3f(0.8f, 0.85f, 0.9f);
            drawText(415.0f, 285.0f, ssHiCmp.str());
        }

        glColor3f(0.3f, 0.9f, 1.0f);
        drawText(340.0f, 225.0f, "Press [R] or [SPACEBAR] to Play Again");
    }
}

// -----------------------------------------------------------------------------
// Display Callback
// -----------------------------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Sky Gradient, Sun, Moon, Stars, Distant Mountains, Clouds
    renderSkyAndLighting();

    // 2. Ground and Dynamic Ground Pebbles / Grass
    renderGround();

    // 3. Obstacles (Cacti, Rocks, Birds)
    renderObstacles();

    // 4. Runner Character
    renderRunner();

    // 5. Particle Effects (Dust, Sparks, Explosions)
    renderParticles();

    // 6. Debug Hitboxes (Toggle with 'H')
    renderHitboxes();

    // 7. HUD Text and Overlays
    renderHUD();

    glutSwapBuffers();
}

// -----------------------------------------------------------------------------
// Physics and Game Logic Update
// -----------------------------------------------------------------------------
void updateGame() {
    if (gameState != STATE_PLAYING) return;

    // 1. Update Day-Night Cycle Timer
    cycleTimer += 1.0f / TARGET_FPS;

    // 2. Update Score and Progressive Speed
    score += (gameSpeed / BASE_SPEED) * 0.18f;
    if ((int)score > highScore) {
        highScore = (int)score;
    }

    // Sound milestone every 100 points
    static int lastMilestone = 0;
    if ((int)score / 100 > lastMilestone) {
        lastMilestone = (int)score / 100;
        soundScoreMilestone();
    }

    // Gradually scale speed with score
    if (gameSpeed < MAX_SPEED) {
        gameSpeed = BASE_SPEED + (score * 0.0042f);
        if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;
    }

    // 3. Runner Physics & Jump Trajectory
    if (!isGrounded) {
        runnerVelocityY -= GRAVITY;
        runnerY += runnerVelocityY;

        if (runnerY <= GROUND_Y) {
            runnerY = GROUND_Y;
            runnerVelocityY = 0.0f;
            isGrounded = true;
            // Spawn landing dust puff
            spawnDust(runnerX + 10.0f, GROUND_Y, 8, -2.5f, 1.8f);
        }
    }

    // Run animation phase
    runAnimPhase += 0.28f * (gameSpeed / BASE_SPEED);
    if (isGrounded && !isDucking) {
        // Kick up subtle running dust occasionally
        if (rand() % 6 == 0) {
            spawnDust(runnerX - 4.0f, GROUND_Y + 2.0f, 1, -3.5f, 0.8f);
        }
    }

    // 4. Update Clouds & Mountains Parallax
    for (auto& c : clouds) {
        c.x -= c.speed * (gameSpeed / BASE_SPEED) * 0.5f;
        if (c.x < -80.0f) {
            c.x = WINDOW_WIDTH + 40.0f + (rand() % 60);
            c.y = 360.0f + (rand() % 160);
        }
    }

    float mtnStep = (float)WINDOW_WIDTH / (MOUNTAIN_COUNT - 2);
    for (int i = 0; i < MOUNTAIN_COUNT; ++i) {
        farMountainX[i] -= gameSpeed * 0.12f;
        nearHillX[i]    -= gameSpeed * 0.35f;
    }
    // Wrap mountains smoothly
    for (int i = 0; i < MOUNTAIN_COUNT; ++i) {
        if (farMountainX[i] < -mtnStep) {
            farMountainX[i] += (MOUNTAIN_COUNT - 1) * mtnStep;
        }
        if (nearHillX[i] < -mtnStep) {
            nearHillX[i] += (MOUNTAIN_COUNT - 1) * mtnStep;
        }
    }

    // 5. Update Ground Details
    for (auto& gd : groundDetails) {
        gd.x -= gameSpeed;
        if (gd.x < -20.0f) {
            gd.x = WINDOW_WIDTH + (rand() % 40);
            gd.y = (rand() % (int)GROUND_Y);
            gd.width = 6.0f + (rand() % 14);
            gd.type = rand() % 3;
        }
    }

    // 6. Obstacle Spawning Logic
    static float spawnDistanceAccum = 0.0f;
    static float nextSpawnDistance = 320.0f;

    spawnDistanceAccum += gameSpeed;
    if (spawnDistanceAccum >= nextSpawnDistance) {
        spawnObstacle();
        spawnDistanceAccum = 0.0f;
        // Higher speeds require larger spacing for reaction time
        float minGap = 260.0f + (gameSpeed - BASE_SPEED) * 18.0f;
        float randGap = (float)(rand() % 240);
        nextSpawnDistance = minGap + randGap;
    }

    // 7. Update Obstacles and Collision Checking
    RectAABB runnerBox;
    if (isDucking && isGrounded) {
        runnerBox = { runnerX + HITBOX_PADDING_X, runnerY, RUNNER_WIDTH_DUCK - HITBOX_PADDING_X * 2, RUNNER_HEIGHT_DUCK };
    } else {
        runnerBox = { runnerX + HITBOX_PADDING_X, runnerY, RUNNER_WIDTH_STAND - HITBOX_PADDING_X * 2, RUNNER_HEIGHT_STAND - HITBOX_PADDING_Y };
    }

    for (size_t i = 0; i < obstacles.size(); ) {
        obstacles[i].x -= gameSpeed;
        obstacles[i].animTimer += 1.0f / TARGET_FPS;

        // Collision Check
        RectAABB obsBox = { obstacles[i].x + 3.0f, obstacles[i].y, obstacles[i].width - 6.0f, obstacles[i].height - 2.0f };
        if (checkAABBCollision(runnerBox, obsBox)) {
            // CRASH / GAME OVER!
            gameState = STATE_GAME_OVER;
            soundGameOver();
            spawnCrashExplosion(runnerX + 20.0f, runnerY + 30.0f);
            break;
        }

        // Clean up off-screen obstacles
        if (obstacles[i].x + obstacles[i].width < -50.0f) {
            obstacles.erase(obstacles.begin() + i);
        } else {
            ++i;
        }
    }

    // 8. Update Particles
    for (size_t i = 0; i < particles.size(); ) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life -= particles[i].decay;

        if (particles[i].life <= 0.0f) {
            particles.erase(particles.begin() + i);
        } else {
            ++i;
        }
    }
}

// -----------------------------------------------------------------------------
// Timer Callback for 60 FPS Loop
// -----------------------------------------------------------------------------
void timerCallback(int value) {
    updateGame();
    glutPostRedisplay();
    glutTimerFunc(FRAME_DELAY_MS, timerCallback, 0);
}

// -----------------------------------------------------------------------------
// Keyboard Input Callbacks
// -----------------------------------------------------------------------------
void handleKeyDown(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        exit(0);
    }

    if (key == 'p' || key == 'P') {
        if (gameState == STATE_PLAYING) {
            gameState = STATE_PAUSED;
        } else if (gameState == STATE_PAUSED) {
            gameState = STATE_PLAYING;
        }
        return;
    }

    if (key == 'r' || key == 'R') {
        resetGame();
        return;
    }

    if (key == 'h' || key == 'H') {
        showHitboxes = !showHitboxes;
        return;
    }

    if (key == 'm' || key == 'M') {
        soundEnabled = !soundEnabled;
        return;
    }

    if (key == ' ') { // Spacebar Jump
        if (gameState == STATE_START) {
            gameState = STATE_PLAYING;
        } else if (gameState == STATE_GAME_OVER) {
            resetGame();
        } else if (gameState == STATE_PLAYING) {
            if (isGrounded) {
                runnerVelocityY = JUMP_FORCE;
                isGrounded = false;
                soundJump();
                spawnDust(runnerX + 10.0f, GROUND_Y, 6, -2.0f, 1.2f);
            }
        }
    }
}

void handleKeyUp(unsigned char key, int x, int y) {
    // Variable jump height: release early cuts upward velocity
    if (key == ' ') {
        if (!isGrounded && runnerVelocityY > 4.0f) {
            runnerVelocityY *= 0.55f;
        }
    }
}

void handleSpecialKeyDown(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        if (gameState == STATE_START) {
            gameState = STATE_PLAYING;
        } else if (gameState == STATE_GAME_OVER) {
            resetGame();
        } else if (gameState == STATE_PLAYING && isGrounded) {
            runnerVelocityY = JUMP_FORCE;
            isGrounded = false;
            soundJump();
            spawnDust(runnerX + 10.0f, GROUND_Y, 6, -2.0f, 1.2f);
        }
    } else if (key == GLUT_KEY_DOWN) {
        if (gameState == STATE_PLAYING) {
            isDucking = true;
            soundDuck();
        }
    }
}

void handleSpecialKeyUp(int key, int x, int y) {
    if (key == GLUT_KEY_DOWN) {
        isDucking = false;
    }
    if (key == GLUT_KEY_UP) {
        if (!isGrounded && runnerVelocityY > 4.0f) {
            runnerVelocityY *= 0.55f;
        }
    }
}

// -----------------------------------------------------------------------------
// Window Reshape Callback
// -----------------------------------------------------------------------------
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// -----------------------------------------------------------------------------
// Main Entry Point
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Chrono Run: DN Dash - CSE-402 CG&IP Lab Project");

    // Enable Alpha Blending for smooth transparencies and particle effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initWorld();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutSpecialFunc(handleSpecialKeyDown);
    glutSpecialUpFunc(handleSpecialKeyUp);
    glutTimerFunc(FRAME_DELAY_MS, timerCallback, 0);

    std::cout << "==========================================================" << std::endl;
    std::cout << "  CHRONO RUN: DN DASH - CSE-402 CG&IP LAB PROJECT" << std::endl;
    std::cout << "  Metropolitan University, Sylhet" << std::endl;
    std::cout << "  Submitted by:" << std::endl;
    std::cout << "    - Nahdiul Islam Rony (231-115-069)" << std::endl;
    std::cout << "    - Abu Ubayda         (231-115-080)" << std::endl;
    std::cout << "  Section: B, 58th Batch" << std::endl;
    std::cout << "  Lecturer: Shrabanti Chowdhury" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  [SPACE] / [UP]  : Jump" << std::endl;
    std::cout << "  [DOWN]          : Duck / Slide" << std::endl;
    std::cout << "  [P]             : Pause / Resume" << std::endl;
    std::cout << "  [R]             : Restart" << std::endl;
    std::cout << "  [H]             : Toggle Hitbox Outlines (Debug)" << std::endl;
    std::cout << "  [M]             : Toggle Audio Sound Effects" << std::endl;
    std::cout << "  [ESC]           : Exit" << std::endl;
    std::cout << "==========================================================" << std::endl;

    glutMainLoop();
    return 0;
}
