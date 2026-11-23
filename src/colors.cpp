#include "colors.h"

// Linear interpolation: C(t) = C1 + (C2 - C1) * t
ColorRGB lerp(const ColorRGB& a, const ColorRGB& b, float t) {
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return ColorRGB(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t);
}

float smooth01(float t) {
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return t * t * (3.f - 2.f * t);
}

void setColor(const ColorRGB& c) {
    glColor3f(c.r, c.g, c.b);
}
