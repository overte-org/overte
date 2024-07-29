//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AmbientOcclusionTechnique.h"

const char* ambientOcclusionTechniqueNames[] = {
    "ssao",
    "hbao"
};

static const size_t AO_TECHNIQUE_NAMES = (sizeof(ambientOcclusionTechniqueNames) / sizeof(ambientOcclusionTechniqueNames[0]));

QString AmbientOcclusionTechniqueHelpers::getNameForAmbientOcclusionTechnique(AmbientOcclusionTechnique technique) {
    if (((int)technique <= 0) || ((int)technique >= (int)AO_TECHNIQUE_NAMES)) {
        technique = (AmbientOcclusionTechnique)0;
    }

    return ambientOcclusionTechniqueNames[(int)technique];
}
