//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <graphics/Geometry.h>
#include <display-plugins/hmd/HmdDisplayPlugin.h>

#include "OpenXrContext.h"

class OpenXrDisplayPlugin : public HmdDisplayPlugin {
public:
    OpenXrDisplayPlugin(std::shared_ptr<OpenXrContext> c);
    bool isSupported() const override;
    const QString getName() const override;
    bool getSupportsAutoSwitch() override final { return true; }

    glm::mat4 getEyeProjection(Eye eye, const glm::mat4& baseProjection) const override;
    glm::mat4 getCullingProjection(const glm::mat4& baseProjection) const override;

    void init() override;

    float getTargetFrameRate() const override;
    bool hasAsyncReprojection() const override { return true; }

    void customizeContext() override;
    void uncustomizeContext() override;

    void resetSensors() override;
    bool beginFrameRender(uint32_t frameIndex) override;
    void submitFrame(const gpu::FramePointer& newFrame) override;
    void cycleDebugOutput() override { _lockCurrentTexture = !_lockCurrentTexture; }

    int getRequiredThreadCount() const override;

    QRectF getPlayAreaRect() override;

    virtual StencilMaskMode getStencilMaskMode() const override { return StencilMaskMode::MESH; }
    virtual StencilMaskMeshOperator getStencilMaskMeshOperator() override;

    glm::mat4 getSensorResetMatrix() const { return glm::mat4(1.0f); }

protected:
    bool internalActivate() override;
    void internalDeactivate() override;
    void updatePresentPose() override;

    void compositeLayers() override;
    void hmdPresent() override;
    bool isHmdMounted() const override;
    void postPreview() override;

private:
    std::vector<gpu::TexturePointer> _compositeSwapChain;

    XrViewState _lastViewState;

    std::shared_ptr<OpenXrContext> _context;

    uint32_t _viewCount = 0;
    std::vector<XrCompositionLayerProjectionView> _projectionLayerViews;

    std::optional<std::vector<XrView>> _views;

    std::vector<XrViewConfigurationView> _viewConfigs;

    std::vector<XrSwapchain> _swapChains;
    std::vector<uint32_t> _swapChainLengths;
    std::vector<uint32_t> _swapChainIndices;
    std::vector<std::vector<XrSwapchainImageOpenGLKHR>> _images;

    XrFrameState _lastFrameState;

    bool initViews();
    bool initSwapChains();
    bool initLayers();
    bool endFrame();

    bool _haveFrameToSubmit = false;
    std::mutex _haveFrameMutex;
};
