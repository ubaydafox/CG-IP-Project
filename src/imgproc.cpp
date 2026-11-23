#define NOMINMAX
#include "imgproc.h"
#include "config.h"
#include "draw.h"
#include <GL/glut.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

using Img = std::vector<unsigned char>;

FilterMode curFilter = FILTER_NONE;
static bool shotRequested = false;

extern int gVpX, gVpY, gVpW, gVpH;

void cycleFilter() {
    curFilter = (FilterMode)((curFilter + 1) % FILTER_COUNT);
}

void saveScreenshot() {
    shotRequested = true;
}

// Grayscale filter: Luminance 0.299R + 0.587G + 0.114B
static void filterGray(Img& p, int w, int h) {
    (void)w; (void)h;
    for (size_t i = 0; i < p.size(); i += 3) {
        unsigned char g = (unsigned char)(0.299f * p[i] + 0.587f * p[i+1] + 0.114f * p[i+2]);
        p[i] = p[i+1] = p[i+2] = g;
    }
}

// Sepia filter: 3x3 color transform matrix
static void filterSepia(Img& p, int w, int h) {
    (void)w; (void)h;
    for (size_t i = 0; i < p.size(); i += 3) {
        float r = p[i], g = p[i+1], b = p[i+2];
        int tr = (int)(0.393f * r + 0.769f * g + 0.189f * b);
        int tg = (int)(0.349f * r + 0.686f * g + 0.168f * b);
        int tb = (int)(0.272f * r + 0.534f * g + 0.131f * b);
        p[i]   = (unsigned char)std::min(255, tr);
        p[i+1] = (unsigned char)std::min(255, tg);
        p[i+2] = (unsigned char)std::min(255, tb);
    }
}

// Invert filter: 255 - value per channel
static void filterInvert(Img& p, int w, int h) {
    (void)w; (void)h;
    for (size_t i = 0; i < p.size(); ++i) {
        p[i] = (unsigned char)(255 - p[i]);
    }
}

// Separable box blur: a 5-tap average horizontally, then vertically.
// Same result as a 5x5 box kernel, but about 2.5x fewer operations per pass.
static void filterBlur(Img& p, int w, int h, int passes) {
    Img tmp(p.size());
    for (int n = 0; n < passes; ++n) {
        // horizontal: p -> tmp
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int c = 0; c < 3; ++c) {
                    int sum = 0;
                    for (int d = -2; d <= 2; ++d) {
                        int xx = std::min(std::max(x + d, 0), w - 1);
                        sum += p[(y * w + xx) * 3 + c];
                    }
                    tmp[(y * w + x) * 3 + c] = (unsigned char)(sum / 5);
                }
        // vertical: tmp -> p
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int c = 0; c < 3; ++c) {
                    int sum = 0;
                    for (int d = -2; d <= 2; ++d) {
                        int yy = std::min(std::max(y + d, 0), h - 1);
                        sum += tmp[(yy * w + x) * 3 + c];
                    }
                    p[(y * w + x) * 3 + c] = (unsigned char)(sum / 5);
                }
    }
}

// Edge detection: Sobel operator, gradient magnitude sqrt(Gx^2 + Gy^2)
static void filterSobel(Img& p, int w, int h) {
    std::vector<int> gray(w * h);
    for (int i = 0; i < w * h; ++i) {
        gray[i] = (int)(0.299f * p[i*3] + 0.587f * p[i*3+1] + 0.114f * p[i*3+2]);
    }
    auto getG = [&](int x, int y) {
        x = std::min(std::max(x, 0), w - 1);
        y = std::min(std::max(y, 0), h - 1);
        return gray[y * w + x];
    };
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int gx = -getG(x-1, y-1) + getG(x+1, y-1)
                     -2*getG(x-1, y)   + 2*getG(x+1, y)
                     -getG(x-1, y+1) + getG(x+1, y+1);
            int gy = -getG(x-1, y-1) - 2*getG(x, y-1) - getG(x+1, y-1)
                     +getG(x-1, y+1) + 2*getG(x, y+1) + getG(x+1, y+1);
            int mag = (int)sqrtf((float)(gx * gx + gy * gy));
            unsigned char val = (unsigned char)std::min(255, mag);
            int idx = (y * w + x) * 3;
            p[idx] = p[idx+1] = p[idx+2] = val;
        }
    }
}

// 24-bit BMP writer (header + BGR rows)
static void writeBMP(const Img& px, int w, int h) {
    static int shotCounter = 1;
    char filename[64];
    while (true) {
        std::snprintf(filename, sizeof(filename), "chronorun_%d.bmp", shotCounter);
        FILE* test = std::fopen(filename, "r");
        if (!test) break;
        std::fclose(test);
        shotCounter++;
    }

    FILE* f = std::fopen(filename, "wb");
    if (!f) return;

    int rowSize = ((w * 3 + 3) / 4) * 4;
    int imageSize = rowSize * h;
    int fileSize = 54 + imageSize;

    unsigned char header[54] = {
        'B', 'M',
        (unsigned char)(fileSize), (unsigned char)(fileSize >> 8), (unsigned char)(fileSize >> 16), (unsigned char)(fileSize >> 24),
        0, 0, 0, 0,
        54, 0, 0, 0,
        40, 0, 0, 0,
        (unsigned char)(w), (unsigned char)(w >> 8), (unsigned char)(w >> 16), (unsigned char)(w >> 24),
        (unsigned char)(h), (unsigned char)(h >> 8), (unsigned char)(h >> 16), (unsigned char)(h >> 24),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        (unsigned char)(imageSize), (unsigned char)(imageSize >> 8), (unsigned char)(imageSize >> 16), (unsigned char)(imageSize >> 24),
        0x13, 0x0B, 0, 0, // 2835 ppm
        0x13, 0x0B, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    std::fwrite(header, 1, 54, f);

    std::vector<unsigned char> row(rowSize, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * 3;
            row[x * 3 + 0] = px[srcIdx + 2]; // B
            row[x * 3 + 1] = px[srcIdx + 1]; // G
            row[x * 3 + 2] = px[srcIdx + 0]; // R
        }
        std::fwrite(row.data(), 1, rowSize, f);
    }

    std::fclose(f);
    std::cout << "Screenshot saved: " << filename << std::endl;
}

void applyPostFilter() {
    if (curFilter == FILTER_NONE && !shotRequested) return;

    int w = gVpW, h = gVpH;
    if (w < 1 || h < 1) return;

    Img px(w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(gVpX, gVpY, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());

    if (curFilter != FILTER_NONE) {
        switch (curFilter) {
            case FILTER_GRAY:   filterGray(px, w, h); break;
            case FILTER_SEPIA:  filterSepia(px, w, h); break;
            case FILTER_INVERT: filterInvert(px, w, h); break;
            case FILTER_BLUR:   filterBlur(px, w, h, 2); break;
            case FILTER_SOBEL:  filterSobel(px, w, h); break;
            default: break;
        }

        glDisable(GL_BLEND);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glRasterPos2f(0.f, 0.f);
        glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
        glEnable(GL_BLEND);

        // Yellow label at top-left
        const char* names[] = { "", "GRAYSCALE", "SEPIA", "INVERT", "BLUR", "EDGE DETECT" };
        std::string lbl = std::string("FILTER: ") + names[curFilter];
        glColor3f(1.0f, 0.9f, 0.2f);
        drawStr(30.f, WIN_H - 60.f, lbl, GLUT_BITMAP_9_BY_15);
    }

    if (shotRequested) {
        shotRequested = false;
        writeBMP(px, w, h);
    }
}
