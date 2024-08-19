//
//  Created by HifiExperiments on 2/9/21
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TextVerticalAlignment.h"

const char* textVerticalAlignmentNames[] = {
    "top",
    "bottom",
    "center"
};

static const size_t TEXT_VERTICAL_ALIGNMENT_NAMES = (sizeof(textVerticalAlignmentNames) / sizeof(textVerticalAlignmentNames[0]));

QString TextVerticalAlignmentHelpers::getNameForTextVerticalAlignment(TextVerticalAlignment verticalAlignment) {
    if (((int)verticalAlignment <= 0) || ((int)verticalAlignment >= (int)TEXT_VERTICAL_ALIGNMENT_NAMES)) {
        verticalAlignment = (TextVerticalAlignment)0;
    }

    return textVerticalAlignmentNames[(int)verticalAlignment];
}