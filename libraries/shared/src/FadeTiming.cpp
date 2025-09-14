//
//  Created by HifiExperiments on 6/15/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FadeTiming.h"

const char* fadeTimingNames[] = {
    "linear",
    "easeIn",
    "easeOut",
    "easeInOut"
};

static const size_t FADE_TIMING_NAMES = (sizeof(fadeTimingNames) / sizeof(fadeTimingNames[0]));

QString FadeTimingHelpers::getNameForFadeTiming(FadeTiming timing) {
    if (((int)timing <= 0) || ((int)timing >= (int)FADE_TIMING_NAMES)) {
        timing = (FadeTiming)0;
    }

    return fadeTimingNames[(int)timing];
}
