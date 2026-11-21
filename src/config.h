#pragma once

// ============================================================================
// Configuration constants
// ============================================================================

const int   WIN_W    = 1000;
const int   WIN_H    = 450;
const float GROUND_Y = 110.0f;
const int   FRAME_MS = 16;          // ~60 FPS timer
const float PX       = 4.0f;        // size of one sprite "pixel"
const float DINO_PX  = 2.0f;        // pixel size for dinosaur (37x2 = 74px tall)

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
