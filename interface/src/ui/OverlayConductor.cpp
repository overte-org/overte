//
//  OverlayConductor.cpp
//  interface/src/ui
//
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "OverlayConductor.h"

#include <OffscreenUi.h>
#include <display-plugins/CompositorHelper.h>

#include "Application.h"
#include "avatar/AvatarManager.h"
#include "InterfaceLogging.h"

OverlayConductor::OverlayConductor() {
}

OverlayConductor::~OverlayConductor() {
}

bool OverlayConductor::headNotCenteredInOverlay() const {
    glm::mat4 hmdMat = qApp->getHMDSensorPose();
    glm::vec3 hmdPos = extractTranslation(hmdMat);
    glm::vec3 hmdForward = transformVectorFast(hmdMat, glm::vec3(0.0f, 0.0f, -1.0f));

    Transform uiTransform = qApp->getApplicationCompositor().getModelTransform();
    glm::vec3 uiPos = uiTransform.getTranslation();
    glm::vec3 uiForward = uiTransform.getRotation() * glm::vec3(0.0f, 0.0f, -1.0f);

    const float MAX_COMPOSITOR_DISTANCE = 0.33f;
    const float MAX_COMPOSITOR_ANGLE = 90.0f;
    if (glm::distance(uiPos, hmdPos) > MAX_COMPOSITOR_DISTANCE ||
        glm::dot(uiForward, hmdForward) < cosf(glm::radians(MAX_COMPOSITOR_ANGLE))) {
        return true;
    }
    return false;
}

void OverlayConductor::centerUI() {
    // place the overlay at the current hmd position in sensor space
    auto camMat = cancelOutRollAndPitch(qApp->getHMDSensorPose());
    // Set its radius.
    camMat = glm::scale(camMat, glm::vec3(HUD_RADIUS));
    qApp->getApplicationCompositor().setModelTransform(Transform(camMat));
}

void OverlayConductor::update(float dt) {
#if !defined(DISABLE_QML)
    auto offscreenUi = DependencyManager::get<OffscreenUi>();
    if (!offscreenUi) {
        return;
    }
    auto desktop = offscreenUi->getDesktop();
    if (!desktop) {
        return;
    }

    auto myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();
    // centerUI when hmd mode is first enabled and mounted
    if (qApp->isHMDMode() && qApp->getActiveDisplayPlugin()->isDisplayVisible()) {
        if (!_hmdMode) {
            _hmdMode = true;
            centerUI();
        }
    } else {
        _hmdMode = false;
    }

    bool initiateRecenter = false;
    if (_hmdMode && headNotCenteredInOverlay()) {
        initiateRecenter = true;
    }

    bool shouldRecenter = false;

    bool hasDriveInput = myAvatar->hasDriveInput();
    if (hasDriveInput && !_lastHasDriveInput && initiateRecenter) {
        shouldRecenter = true;
    }
    _lastHasDriveInput = hasDriveInput;

    if (shouldRecenter) {
        centerUI();
    }
#endif
}
