//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TonemappingCurve.h"

const char* tonemappingCurveNames[] = {
    "rgb",
    "srgb",
    "reinhard",
    "filmic"
};

static const size_t TONEMAPPING_CURVE_NAMES = (sizeof(tonemappingCurveNames) / sizeof(tonemappingCurveNames[0]));

QString TonemappingCurveHelpers::getNameForTonemappingCurve(TonemappingCurve curve) {
    if (((int)curve <= 0) || ((int)curve >= (int)TONEMAPPING_CURVE_NAMES)) {
        curve = (TonemappingCurve)0;
    }

    return tonemappingCurveNames[(int)curve];
}
