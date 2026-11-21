#pragma once
#include <string>
#include "colors.h"

void fillRect(float x, float y, float w, float h);
void outlineRect(float x, float y, float w, float h);
void fillCircle(float cx, float cy, float r, int seg = 20);
void drawStr(float x, float y, const std::string& s, void* f);
float strW(const std::string& s, void* f);
void drawStrC(float cx, float y, const std::string& s, void* f);
void drawSprite(const char* const* rows, int n, float x, float y);
void drawDinoSprite(const char* const* rows, int n, float x, float y, float pxSize,
                    const ColorRGB& colBody, const ColorRGB& colBelly, const ColorRGB& colEye);
