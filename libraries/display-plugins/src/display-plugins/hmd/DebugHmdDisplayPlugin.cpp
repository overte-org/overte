//
//  Created by Bradley Austin Davis on 2016/07/31
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "DebugHmdDisplayPlugin.h"

#include <ui-plugins/PluginContainer.h>

#include <QtCore/QProcessEnvironment>

#include <ViewFrustum.h>
#include <gpu/Frame.h>

const QString DebugHmdDisplayPlugin::NAME("HMD Simulator");

static const QString DEBUG_FLAG("HIFI_DEBUG_HMD");
static bool enableDebugHmd = QProcessEnvironment::systemEnvironment().contains(DEBUG_FLAG);


bool DebugHmdDisplayPlugin::isSupported() const {
    return false;
    //return enableDebugHmd;
}

void DebugHmdDisplayPlugin::resetSensors() {
    _currentRenderFrameInfo.renderPose = glm::mat4(); // identity
}

bool DebugHmdDisplayPlugin::beginFrameRender(uint32_t frameIndex) {
    _currentRenderFrameInfo = FrameInfo();
    _currentRenderFrameInfo.sensorSampleTime = secTimestampNow();
    _currentRenderFrameInfo.predictedDisplayTime = _currentRenderFrameInfo.sensorSampleTime;
    // FIXME simulate head movement
    //_currentRenderFrameInfo.renderPose = ;
    //_currentRenderFrameInfo.presentPose = _currentRenderFrameInfo.renderPose;

    withNonPresentThreadLock([&] {
        _frameInfos[frameIndex] = _currentRenderFrameInfo;
    });
    return Parent::beginFrameRender(frameIndex);
}

bool DebugHmdDisplayPlugin::internalActivate() {
    _isAutoRotateEnabled = _container->getBoolSetting("autoRotate", true);
    _container->addMenuItem(PluginType::DISPLAY_PLUGIN, MENU_PATH(), tr("Auto Rotate"),
                            [this](bool clicked) {
        _isAutoRotateEnabled = clicked;
        _container->setBoolSetting("autoRotate", _isAutoRotateEnabled);
    }, true, _isAutoRotateEnabled);

    _ipd = 0.0327499993f * 2.0f;
    // Quest 
    _eyeProjections[0][0] = vec4{ 0.91729, 0.0, -0.17407, 0.0  };
    _eyeProjections[0][1] = vec4{ 0.0, 0.083354, -0.106141, 0.0 };
    _eyeProjections[0][2] = vec4{ 0.0, 0.0, -1.0, -0.2 };
    _eyeProjections[0][3] = vec4{ 0.0, 0.0, -1.0, 0.0 };
    _eyeProjections[1][0] = vec4{ 0.91729, 0.0, 0.17407, 0.0 };
    _eyeProjections[1][1] = vec4{ 0.0, 0.083354, -0.106141, 0.0 };
    _eyeProjections[1][2] = vec4{ 0.0, 0.0, -1.0, -0.2 };
    _eyeProjections[1][3] = vec4{ 0.0, 0.0, -1.0, 0.0 };
    // No need to do so here as this will done in Parent::internalActivate
    //_eyeInverseProjections[0] = glm::inverse(_eyeProjections[0]);
    //_eyeInverseProjections[1] = glm::inverse(_eyeProjections[1]);
    _eyeOffsets[0][3] = vec4{ -0.0327499993, 0.0, 0.0149999997, 1.0 };
    _eyeOffsets[1][3] = vec4{ 0.0327499993, 0.0, 0.0149999997, 1.0 };
    _eyeInverseProjections[0] = glm::inverse(_eyeProjections[0]);
    _eyeInverseProjections[1] = glm::inverse(_eyeProjections[1]);
    _eyeOffsets[0][3] = vec4{ -0.0327499993, 0.0, -0.0149999997, 1.0 };
    _eyeOffsets[1][3] = vec4{ 0.0327499993, 0.0, -0.0149999997, 1.0 };
    // Test HMD per-eye resolution
    _renderTargetSize = uvec2{ 1214 * 2 , 1344 };
    // uncomment to capture a quarter size frame
    //_renderTargetSize /= 2;
    _cullingProjection = _eyeProjections[0];
    // This must come after the initialization, so that the values calculated
    // above are available during the customizeContext call (when not running
    // in threaded present mode)
    return Parent::internalActivate();
}

void DebugHmdDisplayPlugin::updatePresentPose() {
    Parent::updatePresentPose();
    if (_isAutoRotateEnabled) {
        float yaw = sinf(secTimestampNow()) * 0.25f;
        float pitch = cosf(secTimestampNow()) * 0.25f;
        // Simulates head pose latency correction
        _currentPresentFrameInfo.presentPose =
            glm::mat4_cast(glm::angleAxis(yaw, Vectors::UP)) *
            glm::mat4_cast(glm::angleAxis(pitch, Vectors::RIGHT)) ;
    }
}
