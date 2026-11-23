#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "audio.h"
#include "world.h"
#include "game.h"
#include "highscore.h"
#include <GL/glut.h>
#include <vector>
#include <algorithm>
#include <cmath>

float jumpBuf  = 0.f;
bool  jumpHeld = false;

bool aabbCollide(const RectAABB& a, const RectAABB& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

RectAABB runnerBox() {
    if (ducking && grounded) return { rX + 24.f, rY + 2.f, 84.f, 32.f };
    return                          { rX + 44.f, rY + 2.f, 50.f, 62.f };
}
// Obstacles are narrower at the top (cones, bollards), so shrink the box a bit more
RectAABB obstBox(const Obstacle& o) { return { o.x + 6.f, o.y, o.w - 12.f, o.h - 4.f }; }

void resetGame() {
    obstacles.clear();
    particles.clear();
    rY = GROUND_Y;
    rVY = 0.f;
    jumpBuf = 0.f;
    grounded = true;
    ducking = false;
    runAnim = 0.f;
    gameSpeed = BASE_SPEED;
    score = 0.f;
    cycleT = 0.f;
    prevBest = highScore;
    lastMile = 0;
    blinkTimer = 0.f;
    spawnAcc = 0.f;
    nextSpawn = 400.f;
    gState = STATE_PLAYING;
}

void updateGame(float dt) {
    if (gState == STATE_START) cycleT += dt / 60.f * 4.f;   // title screen: sky drifts 4x faster
    if (gState != STATE_PLAYING) {
        // Let crash particles settle on game over
        for (size_t i = 0; i < particles.size(); ) {
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            particles[i].life -= particles[i].decay * dt;
            if (particles[i].life <= 0.f) particles.erase(particles.begin() + i);
            else ++i;
        }
        return;
    }

    cycleT += dt / 60.f;

    // Score and gentle speed progression
    score += 0.1f * (gameSpeed / BASE_SPEED) * dt;
    if ((int)score > highScore) highScore = (int)score;
    gameSpeed = BASE_SPEED + score * SPEED_RAMP;
    if (gameSpeed > MAX_SPEED) gameSpeed = MAX_SPEED;

    if ((int)score / 100 > lastMile) {
        lastMile = (int)score / 100;
        blinkTimer = 1.2f;
        playSnd(sndScore);
    }
    if (blinkTimer > 0.f) blinkTimer -= dt / 60.f;

    // Jump physics (holding DOWN in the air = fast fall)
    if (jumpBuf > 0.f) jumpBuf -= dt;
    if (!grounded) {
        rVY -= GRAVITY * dt;
        if (ducking) rVY -= GRAVITY * 1.5f * dt;
        rY += rVY * dt;
        if (rY <= GROUND_Y) {
            rY = GROUND_Y; rVY = 0.f; grounded = true; spawnDust(rX + 20.f, GROUND_Y, 5);
            if (jumpBuf > 0.f) {                    // jump was pressed just before landing
                rVY = JUMP_FORCE; grounded = false; playSnd(sndJump);
                if (!jumpHeld) rVY *= 0.55f;        // key already released -> short hop
                jumpBuf = 0.f;
            }
        }
    }

    runAnim += 0.15f * (gameSpeed / BASE_SPEED) * dt;

    scrollWorld(dt);

    // Spawning: generous gaps that grow with speed
    spawnAcc += gameSpeed * dt;
    if (spawnAcc >= nextSpawn) {
        spawnObstacle();
        spawnAcc  = 0.f;
        // Fair gap: enough distance for one full jump plus ~20 frames to react,
        // measured at the CURRENT speed, plus margin for obstacle width.
        float airFrames = 2.f * JUMP_FORCE / GRAVITY;          // time in the air, in frames
        float minGap    = std::max(380.f, gameSpeed * (airFrames + 20.f) + 80.f);
        bool  tight     = (score > 300.f && rand() % 100 < 25); // rhythm section: back-to-back
        nextSpawn = tight ? minGap : minGap + frand(0, 340);
    }

    // Obstacles + AABB collision
    RectAABB rb = runnerBox();
    for (size_t i = 0; i < obstacles.size(); ) {
        obstacles[i].x    -= gameSpeed * dt;
        obstacles[i].anim += dt / 60.f;
        if (aabbCollide(rb, obstBox(obstacles[i]))) {
            gState = STATE_GAME_OVER;
            saveHighScore();
            playSnd(sndOver);
            for (int k = 0; k < 14; ++k)
                particles.push_back({ rX + 30.f, rY + 40.f, frand(-3, 3), frand(-1, 4), 1.f, frand(0.02f, 0.04f), frand(3, 5) });
            break;
        }
        if (obstacles[i].x + obstacles[i].w < -50.f) obstacles.erase(obstacles.begin() + i);
        else ++i;
    }

    for (size_t i = 0; i < particles.size(); ) {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].life -= particles[i].decay * dt;
        if (particles[i].life <= 0.f) particles.erase(particles.begin() + i);
        else ++i;
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
