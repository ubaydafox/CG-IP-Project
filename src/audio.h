#pragma once

#include <vector>

// ============================================================================
// Audio — Tone descriptor
// ============================================================================
// Soft sine tones are synthesised into in-memory WAV buffers at startup and
// played asynchronously with PlaySound. Each tone has a short fade-in and a
// smooth fade-out, so nothing clicks or beeps harshly. Only one sound plays at
// a time (a new sound replaces the old), so effects never pile up.
struct Tone { float f0, f1, dur, delay, vol; };

// ============================================================================
// Sound buffer extern declarations (definitions in audio.cpp)
// ============================================================================
extern std::vector<char> sndJump, sndDuck, sndScore, sndOver;
extern bool soundOn;

// ============================================================================
// Audio function declarations
// ============================================================================
std::vector<char> synthWav(const std::vector<Tone>& tones);
void initSounds();
void playSnd(const std::vector<char>& w);
