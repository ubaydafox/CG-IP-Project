#define _USE_MATH_DEFINES
#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "daynight.h"
#include <cmath>

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

    // Dynamic theme ambient lighting for the dinosaur runner
    const ThemeColors& th = THEMES[currentTheme];
    if (nightF > 0.01f) {
        // At night/dusk, tint smoothly into luminous neon/glow shades and light up the eyes
        ColorRGB nightBody = lerp(th.body, ColorRGB(0.2f, 0.8f, 1.0f), 0.25f);
        curBodyCol  = lerp(th.body, nightBody, nightF);
        curBellyCol = lerp(th.belly, ColorRGB(1.0f, 0.95f, 0.5f), nightF * 0.4f);
        curEyeCol   = lerp(th.eye, ColorRGB(0.95f, 0.95f, 0.95f), nightF);
    } else {
        curBodyCol  = th.body;
        curBellyCol = th.belly;
        curEyeCol   = th.eye;
    }

    if      (t < 20.f)       gPhase = PHASE_DAWN;
    else if (t < 70.f)       gPhase = PHASE_DAY;
    else if (t < DAY_END + FADE * 0.5f) gPhase = PHASE_DUSK;
    else if (t < NIGHT_END + FADE * 0.5f) gPhase = PHASE_NIGHT;
    else                     gPhase = PHASE_DAWN;
}

// Mix between background and ink: 0 = invisible, 1 = full ink
ColorRGB tone(float k) { return lerp(bgCol, inkCol, k); }
