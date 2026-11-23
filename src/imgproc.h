#pragma once

enum FilterMode {
    FILTER_NONE = 0,
    FILTER_GRAY,
    FILTER_SEPIA,
    FILTER_INVERT,
    FILTER_BLUR,
    FILTER_SOBEL,
    FILTER_COUNT
};

extern FilterMode curFilter;

void cycleFilter();
void saveScreenshot();
void applyPostFilter();
