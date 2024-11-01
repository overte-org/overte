//
//  Created by HifiExperiments on 10/30/2024
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "CameraRootTransformNode.h"

#include "Application.h"
#include "DependencyManager.h"
#include "avatar/AvatarManager.h"
#include "avatar/MyAvatar.h"

Transform CameraRootTransformNode::getTransform() {
    auto myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();

    glm::vec3 pos;
    glm::quat ori;

    CameraMode mode = qApp->getCamera().getMode();
    if (mode == CAMERA_MODE_FIRST_PERSON || mode == CAMERA_MODE_THIRD_PERSON) {
        pos = myAvatar->getDefaultEyePosition();
        ori = myAvatar->getHeadOrientation();
    } else if (mode == CAMERA_MODE_FIRST_PERSON_LOOK_AT) {
        pos = myAvatar->getCameraEyesPosition(0.0f);
        ori = myAvatar->getLookAtRotation();
    } else {
        ori = myAvatar->getLookAtRotation();
        pos = myAvatar->getLookAtPivotPoint();

        if (mode == CAMERA_MODE_SELFIE) {
            ori = ori * glm::angleAxis(PI, ori * Vectors::UP);
        }
    }

    ori = ori * glm::angleAxis(-PI / 2.0f, Vectors::RIGHT);

    glm::vec3 scale = glm::vec3(myAvatar->scaleForChildren());
    return Transform(ori, scale, pos);
}

QVariantMap CameraRootTransformNode::toVariantMap() const {
    QVariantMap map;
    map["joint"] = "CameraRoot";
    return map;
}
