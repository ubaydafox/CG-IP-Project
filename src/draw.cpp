#include "config.h"
#include "colors.h"
#include "state.h"
#include "sprites.h"
#include "draw.h"
#include <GL/glut.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <cmath>

// ============================================================================
// Drawing primitives
// ============================================================================
void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}
void outlineRect(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}
void fillCircle(float cx, float cy, float r, int seg) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; ++i) {
        float a = i * 2.f * (float)M_PI / seg;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}
void drawStr(float x, float y, const std::string& s, void* f) {
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(f, c);
}
float strW(const std::string& s, void* f) {
    return (float)glutBitmapLength(f, (const unsigned char*)s.c_str());
}
void drawStrC(float cx, float y, const std::string& s, void* f) { drawStr(cx - strW(s, f) * 0.5f, y, s, f); }

// Raster sprite -> quads. (x, y) = bottom-left corner.
void drawSprite(const char* const* rows, int n, float x, float y) {
    glBegin(GL_QUADS);
    for (int r = 0; r < n; ++r) {
        const char* row = rows[r];
        int len = (int)strlen(row);
        float yy = y + (n - 1 - r) * PX;
        for (int c = 0; c < len; ++c) {
            if (row[c] != '#') continue;
            float xx = x + c * PX;
            glVertex2f(xx, yy); glVertex2f(xx + PX, yy);
            glVertex2f(xx + PX, yy + PX); glVertex2f(xx, yy + PX);
        }
    }
    glEnd();
}

// Dinosaur multi-colored sprite renderer with dynamic theme lighting
void drawDinoSprite(const char* const* rows, int n, float x, float y, float pxSize,
                    const ColorRGB& colBody, const ColorRGB& colBelly, const ColorRGB& colEye) {
    glBegin(GL_QUADS);
    for (int r = 0; r < n; ++r) {
        const char* row = rows[r];
        int len = (int)strlen(row);
        float yy = y + (n - 1 - r) * pxSize;
        for (int c = 0; c < len; ++c) {
            char ch = row[c];
            if (ch == '.') continue;
            if (ch == 'B') glColor3f(colBody.r, colBody.g, colBody.b);
            else if (ch == 'Y') glColor3f(colBelly.r, colBelly.g, colBelly.b);
            else if (ch == 'K') glColor3f(colEye.r, colEye.g, colEye.b);
            else if (ch == 'W') glColor3f(1.f, 1.f, 1.f);
            else glColor3f(colBody.r, colBody.g, colBody.b);

            float xx = x + c * pxSize;
            glVertex2f(xx, yy); glVertex2f(xx + pxSize, yy);
            glVertex2f(xx + pxSize, yy + pxSize); glVertex2f(xx, yy + pxSize);
        }
    }
    glEnd();
}
