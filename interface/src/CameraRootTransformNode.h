//
//  Created by HifiExperiments on 10/30/2024
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_CameraRootTransformNode_h
#define hifi_CameraRootTransformNode_h

#include "TransformNode.h"

class CameraRootTransformNode : public TransformNode {
public:
    CameraRootTransformNode() {}
    Transform getTransform() override;
    QVariantMap toVariantMap() const override;
};

#endif // hifi_CameraRootTransformNode_h
