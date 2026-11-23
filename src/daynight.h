#pragma once
#include "colors.h"

// ============================================================================
// Day-night cycle functions
// ============================================================================
void updatePalette();
ColorRGB tone(float k);

extern ColorRGB skyTopCol;          // colour at the top of the sky gradient
ColorRGB skyAt(float y);            // sky colour at height y (horizon -> top)
void skipToNextPhase();             // demo key [N]

