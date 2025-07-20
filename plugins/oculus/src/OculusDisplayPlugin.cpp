//
//  Created by Bradley Austin Davis on 2014/04/13.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "OculusDisplayPlugin.h"

// Odd ordering of header is required to avoid 'macro redinition warnings'
#include <AudioClient.h>

#include <OVR_CAPI_Audio.h>

#include <shared/NsightHelpers.h>
#include <gpu/Frame.h>
#include <gpu/Context.h>
#include <gpu/gl/GLBackend.h>

#include "OculusHelpers.h"

using namespace hifi;

const char* OculusDisplayPlugin::NAME { "Oculus Rift" };
static ovrPerfHudMode currentDebugMode = ovrPerfHud_Off;


OculusDisplayPlugin::OculusDisplayPlugin() {
    _appDroppedFrames.store(0);
    _compositorDroppedFrames.store(0);
}

float OculusDisplayPlugin::getTargetFrameRate() const {
    if (_aswActive) {
        return _hmdDesc.DisplayRefreshRate / 2.0f;
    }
    return _hmdDesc.DisplayRefreshRate;
}

bool OculusDisplayPlugin::internalActivate() {
    bool result = Parent::internalActivate();
    _longSubmits = 0;
    _longRenders = 0;
    _longFrames = 0;

    currentDebugMode = ovrPerfHud_Off;
    if (result && _session) {
        ovr_SetInt(_session, OVR_PERF_HUD_MODE, currentDebugMode);
    }
    return result;
}

void OculusDisplayPlugin::init() {
    Plugin::init();

    // Different HMDs end up showing the squeezed-vision egg as different sizes.  These values
    // attempt to make them appear the same.
    _visionSqueezeDeviceLowX = 0.7f;
    _visionSqueezeDeviceHighX = 0.98f;
    _visionSqueezeDeviceLowY = 0.7f;
    _visionSqueezeDeviceHighY = 0.9f;

    emit deviceConnected(getName());
}

void OculusDisplayPlugin::cycleDebugOutput() {
    if (_session) {
        currentDebugMode = static_cast<ovrPerfHudMode>((currentDebugMode + 1) % ovrPerfHud_Count);
        ovr_SetInt(_session, OVR_PERF_HUD_MODE, currentDebugMode);
    }
}

void OculusDisplayPlugin::customizeContext() {
    Parent::customizeContext();
    _outputFramebuffer.reset(gpu::Framebuffer::create("OculusOutput", gpu::Element::COLOR_SRGBA_32, _renderTargetSize.x, _renderTargetSize.y));
    ovrTextureSwapChainDesc desc = { };
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = _renderTargetSize.x;
    desc.Height = _renderTargetSize.y;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_textureSwapChain);
    if (!OVR_SUCCESS(result)) {
        qCritical(oculusLog) << "Failed to create swap textures" << ovr::getError();
        return;
    }

    int length = 0;
    result = ovr_GetTextureSwapChainLength(_session, _textureSwapChain, &length);
    if (!OVR_SUCCESS(result) || !length) {
        qCritical(oculusLog) << "Unable to count swap chain textures" << ovr::getError();
        return;
    }
    for (int i = 0; i < length; ++i) {
        GLuint chainTexId;
        ovr_GetTextureSwapChainBufferGL(_session, _textureSwapChain, i, &chainTexId);
        glBindTexture(GL_TEXTURE_2D, chainTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // We're rendering both eyes to the same texture, so only one of the 
    // pointers is populated
    _sceneLayer.ColorTexture[0] = _textureSwapChain;
    // not needed since the structure was zeroed on init, but explicit
    _sceneLayer.ColorTexture[1] = nullptr;
    _customized = true;
}

void OculusDisplayPlugin::uncustomizeContext() {
#if 0
    // Present a final black frame to the HMD
    _compositeFramebuffer->Bound(FramebufferTarget::Draw, [] {
        Context::ClearColor(0, 0, 0, 1);
        Context::Clear().ColorBuffer();
    });
    hmdPresent();
#endif

    ovr_DestroyTextureSwapChain(_session, _textureSwapChain);
    _textureSwapChain = nullptr;
    _outputFramebuffer.reset();
    _customized = false;
    Parent::uncustomizeContext();
}

static const uint64_t FRAME_BUDGET = (11 * USECS_PER_MSEC);
static const uint64_t FRAME_OVER_BUDGET = (15 * USECS_PER_MSEC);

void OculusDisplayPlugin::hmdPresent() {
    static uint64_t lastSubmitEnd = 0;

    if (!_customized) {
        return;
    }

    if (!_visible) {
        return;
    }

    if (!_currentFrame) {
        return;
    }

    PROFILE_RANGE_EX(render, __FUNCTION__, 0xff00ff00, (uint64_t)_currentFrame->frameIndex)

    {
        PROFILE_RANGE_EX(render, "Oculus Blit", 0xff00ff00, (uint64_t)_currentFrame->frameIndex)
        int curIndex;
        ovr_GetTextureSwapChainCurrentIndex(_session, _textureSwapChain, &curIndex);
        GLuint curTexId;
        ovr_GetTextureSwapChainBufferGL(_session, _textureSwapChain, curIndex, &curTexId);

        _visionSqueezeParametersBuffer.edit<VisionSqueezeParameters>()._leftProjection = _eyeProjections[0];
        _visionSqueezeParametersBuffer.edit<VisionSqueezeParameters>()._rightProjection = _eyeProjections[1];

        // Manually bind the texture to the FBO
        // FIXME we should have a way of wrapping raw GL ids in GPU objects without
        // taking ownership of the object
        auto fbo = getGLBackend()->getFramebufferID(_outputFramebuffer);
        glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, curTexId, 0);
        render([&](gpu::Batch& batch) {
            batch.enableStereo(false);
            batch.setFramebuffer(_outputFramebuffer);
            batch.setViewportTransform(ivec4(uvec2(), _outputFramebuffer->getSize()));
            batch.setStateScissorRect(ivec4(uvec2(), _outputFramebuffer->getSize()));
            batch.resetViewTransform();
            batch.setProjectionTransform(mat4());
            batch.setPipeline(_drawTexturePipeline);
            batch.setResourceTexture(0, _compositeFramebuffer->getRenderBuffer(0));
            batch.draw(gpu::TRIANGLE_STRIP, 4);
        });
        glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
    }

    {
        auto result = ovr_CommitTextureSwapChain(_session, _textureSwapChain);
        Q_ASSERT(OVR_SUCCESS(result));
        _sceneLayer.SensorSampleTime = _currentPresentFrameInfo.sensorSampleTime;
        _sceneLayer.RenderPose[ovrEyeType::ovrEye_Left] = ovr::poseFromGlm(_currentPresentFrameInfo.renderPose);
        _sceneLayer.RenderPose[ovrEyeType::ovrEye_Right] = ovr::poseFromGlm(_currentPresentFrameInfo.renderPose);

        auto submitStart = usecTimestampNow();
        uint64_t nonSubmitInterval = 0;
        if (lastSubmitEnd != 0) {
            nonSubmitInterval = submitStart - lastSubmitEnd;
            if (nonSubmitInterval > FRAME_BUDGET) {
                ++_longRenders;
            }
        }
        ovrLayerHeader* layers = &_sceneLayer.Header;
        {
            PROFILE_RANGE_EX(render, "Oculus Submit", 0xff00ff00, (uint64_t)_currentFrame->frameIndex)
            result = ovr_SubmitFrame(_session, _currentFrame->frameIndex, &_viewScaleDesc, &layers, 1);
        }
        lastSubmitEnd = usecTimestampNow();
        if (nonSubmitInterval != 0) {
            auto submitInterval = lastSubmitEnd - submitStart;
            if (nonSubmitInterval < FRAME_BUDGET && submitInterval > FRAME_BUDGET) {
                ++_longSubmits;
            }
            if ((nonSubmitInterval + submitInterval) > FRAME_OVER_BUDGET) {
                ++_longFrames;
            }
        }

        if (!OVR_SUCCESS(result)) {
            qWarning(oculusLog) << "Failed to present" << ovr::getError();
            if (result == ovrError_DisplayLost) {
                qWarning(oculusLog) << "Display lost, shutting down";
                return;
            }
        }

        static int compositorDroppedFrames = 0;
        static int appDroppedFrames = 0;
        ovrPerfStats perfStats;
        ovr_GetPerfStats(_session, &perfStats);
        bool shouldResetPresentRate = false;
        for (int i = 0; i < perfStats.FrameStatsCount; ++i) {
            const auto& frameStats = perfStats.FrameStats[i];
            int delta = frameStats.CompositorDroppedFrameCount - compositorDroppedFrames;
            _stutterRate.increment(delta);
            compositorDroppedFrames = frameStats.CompositorDroppedFrameCount;
            appDroppedFrames = frameStats.AppDroppedFrameCount;
            bool newAswState = ovrTrue == frameStats.AswIsActive;
            if (_aswActive.exchange(newAswState) != newAswState) {
                shouldResetPresentRate = true;
            }
        }
        if (shouldResetPresentRate) {
            resetPresentRate();
        }
        _appDroppedFrames.store(appDroppedFrames);
        _compositorDroppedFrames.store(compositorDroppedFrames);
    }

    _presentRate.increment();
}


QJsonObject OculusDisplayPlugin::getHardwareStats() const {
    QJsonObject hardwareStats;
    hardwareStats["asw_active"] = _aswActive.load();
    hardwareStats["app_dropped_frame_count"] = _appDroppedFrames.load();
    hardwareStats["compositor_dropped_frame_count"] = _compositorDroppedFrames.load();
    hardwareStats["long_render_count"] = _longRenders.load();
    hardwareStats["long_submit_count"] = _longSubmits.load();
    hardwareStats["long_frame_count"] = _longFrames.load();
    return hardwareStats;
}

bool OculusDisplayPlugin::isHmdMounted() const {
    return ovr::hmdMounted();
}

QString OculusDisplayPlugin::getPreferredAudioInDevice() const {
    WCHAR buffer[OVR_AUDIO_MAX_DEVICE_STR_SIZE];
    if (!OVR_SUCCESS(ovr_GetAudioDeviceInGuidStr(buffer))) {
        return QString();
    }
    return AudioClient::getWinDeviceName(buffer);
}

QString OculusDisplayPlugin::getPreferredAudioOutDevice() const {
    WCHAR buffer[OVR_AUDIO_MAX_DEVICE_STR_SIZE];
    if (!OVR_SUCCESS(ovr_GetAudioDeviceOutGuidStr(buffer))) {
        return QString();
    }
    return AudioClient::getWinDeviceName(buffer);
}

OculusDisplayPlugin::~OculusDisplayPlugin() {
}
