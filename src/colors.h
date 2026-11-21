#pragma once

#include <GL/glut.h>

// ============================================================================
// Colour helpers
// ============================================================================
struct ColorRGB {
    float r, g, b;
    ColorRGB(float r = 0.f, float g = 0.f, float b = 0.f) : r(r), g(g), b(b) {}
};

// Linear interpolation: C(t) = C1 + (C2 - C1) * t
inline ColorRGB lerp(const ColorRGB& a, const ColorRGB& b, float t) {
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return ColorRGB(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t);
}
inline float smooth01(float t) { if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f; return t * t * (3.f - 2.f * t); }
inline void  setColor(const ColorRGB& c) { glColor3f(c.r, c.g, c.b); }

// ============================================================================
// AABB
// ============================================================================
struct RectAABB { float x, y, w, h; };
inline bool aabbCollide(const RectAABB& a, const RectAABB& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}
