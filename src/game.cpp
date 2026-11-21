#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "draw.h"
#include "world.h"
#include "daynight.h"
#include "audio.h"
#include "render.h"
#include "game.h"
#include <GL/glut.h>
#include <vector>
#include <algorithm>
#include <cmath>

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
