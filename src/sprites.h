#pragma once

#include "colors.h"
#include "state.h"

// ============================================================================
// Sprite array extern declarations
// ============================================================================
extern const char* DINO_RUN_A[37];
extern const char* DINO_RUN_B[37];
extern const char* DINO_JUMP[37];
extern const char* DINO_CRASH[37];
extern const char* DINO_DUCK_A[19];
extern const char* DINO_DUCK_B[19];

// ============================================================================
// Theme data extern declarations
// ============================================================================
extern const ThemeColors THEMES[];
extern const int THEME_COUNT;
extern int currentTheme;

// Dynamic lighting colors for the current runner theme
extern ColorRGB curBodyCol, curBellyCol, curEyeCol;
