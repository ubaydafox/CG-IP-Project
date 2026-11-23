#pragma once
#include <GL/glut.h>

// ============================================================================
// Colour helpers
// ============================================================================
struct ColorRGB {
    float r, g, b;
    ColorRGB(float r = 0.f, float g = 0.f, float b = 0.f) : r(r), g(g), b(b) {}
};

ColorRGB lerp(const ColorRGB& a, const ColorRGB& b, float t);
float smooth01(float t);
void setColor(const ColorRGB& c);
