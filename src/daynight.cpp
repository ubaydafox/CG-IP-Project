#define _USE_MATH_DEFINES
#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "daynight.h"
#include <cmath>

ColorRGB skyTopCol;

// Blend from the horizon colour (bgCol) up to the top colour (skyTopCol)
ColorRGB skyAt(float y) {
    float k = (y - GROUND_Y) / (WIN_H - GROUND_Y);
    if (k < 0.f) k = 0.f;
    if (k > 1.f) k = 1.f;
    return lerp(bgCol, skyTopCol, k);
}

// Jump to the start of the next phase so a demo can show the whole cycle quickly
void skipToNextPhase() {
    float t = fmodf(cycleT, CYCLE);
    const float marks[] = { 0.f, 20.f, 70.f, 88.f, 144.f };   // dawn, day, dusk, night fade, late night
    float next = CYCLE;                                        // default: wrap to the next dawn
    for (float m : marks) if (m > t + 0.5f) { next = m; break; }
    cycleT = (cycleT - t) + next;
}

// ============================================================================
// Day-night palette (Chrome-style: dark ink by day, inverted at night)
// ============================================================================
void updatePalette() {
    float t = fmodf(cycleT, CYCLE);
    ColorRGB dawnBg(1.00f, 0.86f, 0.74f), dayBg(0.97f, 0.97f, 0.97f),
             duskBg(0.99f, 0.74f, 0.56f), nightBg(0.11f, 0.12f, 0.16f);

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

    // Top of the sky gradient (same timeline as the horizon colour)
    ColorRGB dawnTop(0.78f, 0.72f, 0.90f), dayTop(0.80f, 0.89f, 0.98f),
             duskTop(0.62f, 0.55f, 0.76f), nightTop(0.04f, 0.05f, 0.10f);
    ColorRGB dTop;
    if (t >= NIGHT_END)  dTop = dawnTop;
    else if (t < 20.f)   dTop = lerp(dawnTop, dayTop, t / 20.f);
    else if (t < 70.f)   dTop = dayTop;
    else                 dTop = lerp(dayTop, duskTop, (t - 70.f) / 20.f);
    skyTopCol = lerp(dTop, nightTop, nightF);
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
