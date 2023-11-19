//
//  Created by HifiExperiments on 3/14/22.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "MirrorMode.h"

const char* MirrorModeNames[] = {
    "none",
    "mirror",
    "portal"
};

static const size_t MIRROR_MODE_NAMES = (sizeof(MirrorModeNames) / sizeof(MirrorModeNames[0]));
std::function<uint32_t(ViewFrustum&, const glm::vec3&, const glm::quat&, MirrorMode, const QUuid&)> MirrorModeHelpers::_computeMirrorViewOperator =
    [](ViewFrustum&, const glm::vec3&, const glm::quat&, MirrorMode, const QUuid&) { return 0; };

QString MirrorModeHelpers::getNameForMirrorMode(MirrorMode mode) {
    if (((int)mode <= 0) || ((int)mode >= (int)MIRROR_MODE_NAMES)) {
        mode = (MirrorMode)0;
    }

    return MirrorModeNames[(int)mode];
}

void MirrorModeHelpers::setComputeMirrorViewOperator(std::function<uint32_t(ViewFrustum&, const glm::vec3&, const glm::quat&, MirrorMode, const QUuid&)> computeMirrorViewOperator) {
    _computeMirrorViewOperator = computeMirrorViewOperator;
}

uint32_t MirrorModeHelpers::computeMirrorView(ViewFrustum& viewFrustum, const glm::vec3& inPropertiesPosition, const glm::quat& inPropertiesRotation,
                                              MirrorMode mirrorMode, const QUuid& portalExitID) {
    return _computeMirrorViewOperator(viewFrustum, inPropertiesPosition, inPropertiesRotation, mirrorMode, portalExitID);
}