#include "config.h"
#include "colors.h"
#include "state.h"
#include "world.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>

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
