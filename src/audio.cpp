// audio.cpp — Sound synthesis and playback for Chrono Run
// Generated as part of the Chrono Run refactor.

#define _USE_MATH_DEFINES
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "audio.h"
#include "config.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>

// ============================================================================
// Sound buffers and global flag
// ============================================================================
std::vector<char> sndJump, sndDuck, sndScore, sndOver;
bool soundOn = true;

/* ---- BACKGROUND MUSIC PLACEHOLDER ----
   Add your background music buffer and playback here.
   Call your music init from initSounds() and start playback
   with SND_LOOP | SND_ASYNC flags.
   ---------------------------------------- */

// ============================================================================
// Wave synthesis (verbatim from original main.cpp lines 387-419)
// Soft sine tones are synthesised into in-memory WAV buffers at startup and
// played asynchronously with PlaySound. Each tone has a short fade-in and a
// smooth fade-out, so nothing clicks or beeps harshly. Only one sound plays at
// a time (a new sound replaces the old), so effects never pile up.
// ============================================================================
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

// ============================================================================
// Sound initialisation (verbatim from original main.cpp lines 421-428)
// ============================================================================
void initSounds() {
    sndJump  = synthWav({ { 520.f, 720.f, 0.11f, 0.00f, 0.28f } });
    sndDuck  = synthWav({ { 320.f, 240.f, 0.07f, 0.00f, 0.18f } });
    sndScore = synthWav({ { 660.f, 660.f, 0.14f, 0.00f, 0.22f },
                          { 880.f, 880.f, 0.24f, 0.11f, 0.22f } });
    sndOver  = synthWav({ { 440.f, 330.f, 0.25f, 0.00f, 0.26f },
                          { 330.f, 247.f, 0.40f, 0.20f, 0.24f } });
}

// ============================================================================
// Sound playback (verbatim from original main.cpp lines 430-433)
// ============================================================================
void playSnd(const std::vector<char>& w) {
    if (soundOn && !w.empty())
        PlaySoundA(w.data(), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}
