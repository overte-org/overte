//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#include "OpenXrDisplayPlugin.h"
#include <qloggingcategory.h>

#include "ViewFrustum.h"

#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <thread>
#include <sstream>

#if defined(Q_OS_WIN)
#undef near
#undef far
#endif

Q_DECLARE_LOGGING_CATEGORY(xr_display_cat)
Q_LOGGING_CATEGORY(xr_display_cat, "openxr.display")

constexpr GLint XR_PREFERRED_COLOR_FORMAT = GL_SRGB8_ALPHA8;

OpenXrDisplayPlugin::OpenXrDisplayPlugin(std::shared_ptr<OpenXrContext> c) {
    _context = c;
}

bool OpenXrDisplayPlugin::isSupported() const {
    return _context->_isValid && _context->_isSupported;
}

// Slightly differs from glm::ortho
inline static glm::mat4 fovToProjection(const XrFovf fov, const float near, const float far) {
    const float left = tanf(fov.angleLeft);
    const float right = tanf(fov.angleRight);
    const float down = tanf(fov.angleDown);
    const float up = tanf(fov.angleUp);

    const float width = right - left;
    const float height = up - down;

    const float m11 = 2 / width;
    const float m22 = 2 / height;
    const float m33 = -(far + near) / (far - near);

    const float m31 = (right + left) / width;
    const float m32 = (up + down) / height;
    const float m43 = -(far * (near + near)) / (far - near);

    // clang-format off
    const float mat[16] = {
        m11, 0  , 0  ,  0,
        0  , m22, 0  ,  0,
        m31, m32, m33, -1,
        0  , 0  , m43,  0,
    };
    // clang-format on

    return glm::make_mat4(mat);
}

glm::mat4 OpenXrDisplayPlugin::getEyeProjection(Eye eye, const glm::mat4& baseProjection) const {
    if (!_views.has_value()) {
        return baseProjection;
    }

    ViewFrustum frustum;
    frustum.setProjection(baseProjection);
    return fovToProjection(_views.value()[(eye == Left) ? 0 : 1].fov, frustum.getNearClip(), frustum.getFarClip());
}

// TODO: interface/src/Application_Graphics.cpp:535
glm::mat4 OpenXrDisplayPlugin::getCullingProjection(const glm::mat4& baseProjection) const {
    return getEyeProjection(Left, baseProjection);
}

float OpenXrDisplayPlugin::getTargetFrameRate() const {
    // predictedDisplayPeriod is delta nanoseconds, so convert it to frames per second
    return std::max(1.0f, 1.0f / (_lastFrameState.predictedDisplayPeriod / 1e9f));
}

bool OpenXrDisplayPlugin::initViews() {
    XrInstance instance = _context->_instance;
    XrSystemId systemId = _context->_systemId;

    XrResult result = xrEnumerateViewConfigurationViews(instance, systemId, XR_VIEW_CONFIG_TYPE, 0, &_viewCount, nullptr);
    if (!xrCheck(instance, result, "Failed to get view configuration view count!")) {
        qCCritical(xr_display_cat, "Failed to get view configuration view count!");
        return false;
    }

    assert(_viewCount != 0);

    _views = std::vector<XrView>();

    for (uint32_t i = 0; i < _viewCount; i++) {
        XrView view = { .type = XR_TYPE_VIEW };
        _views.value().push_back(view);

        XrViewConfigurationView viewConfig = { .type = XR_TYPE_VIEW_CONFIGURATION_VIEW };
        _viewConfigs.push_back(viewConfig);
    }

    _swapChains.resize(_viewCount);
    _swapChainLengths.resize(_viewCount);
    _swapChainIndices.resize(_viewCount);
    _images.resize(_viewCount);

    result = xrEnumerateViewConfigurationViews(instance, systemId, XR_VIEW_CONFIG_TYPE, _viewCount, &_viewCount,
                                               _viewConfigs.data());
    if (!xrCheck(instance, result, "Failed to enumerate view configuration views!")) {
        qCCritical(xr_display_cat, "Failed to enumerate view configuration views!");
        return false;
    }

    return true;
}

#define ENUM_TO_STR(r) \
    case r:            \
        return #r

static std::string glFormatStr(GLenum source) {
    switch (source) {
        ENUM_TO_STR(GL_RGBA16);
        ENUM_TO_STR(GL_RGBA16F);
        ENUM_TO_STR(GL_SRGB8_ALPHA8);
        ENUM_TO_STR(GL_RGB10_A2UI);
        default: {
            std::stringstream stream;
            stream << "0x" << std::uppercase << std::hex << source;
            return stream.str();
        }
    }
}

static int64_t chooseSwapChainFormat(XrInstance instance, XrSession session, int64_t preferred) {
    uint32_t formatCount;
    XrResult result = xrEnumerateSwapchainFormats(session, 0, &formatCount, nullptr);
    if (!xrCheck(instance, result, "Failed to get number of supported swapchain formats"))
        return -1;

    qCInfo(xr_display_cat, "Runtime supports %d swapchain formats", formatCount);
    std::vector<int64_t> formats(formatCount);

    result = xrEnumerateSwapchainFormats(session, formatCount, &formatCount, formats.data());
    if (!xrCheck(instance, result, "Failed to enumerate swapchain formats"))
        return -1;

    int64_t chosen = formats[0];

    for (uint32_t i = 0; i < formatCount; i++) {
        qCInfo(xr_display_cat, "Supported GL format: %s", glFormatStr(formats[i]).c_str());
        if (formats[i] == preferred) {
            chosen = formats[i];
            qCInfo(xr_display_cat, "Using preferred swapchain format %s", glFormatStr(chosen).c_str());
            break;
        }
    }
    if (chosen != preferred) {
        qCWarning(xr_display_cat, "Falling back to non preferred swapchain format %s", glFormatStr(chosen).c_str());
    }

    return chosen;
}

bool OpenXrDisplayPlugin::initSwapChains() {
    XrInstance instance = _context->_instance;
    XrSession session = _context->_session;

    int64_t format = chooseSwapChainFormat(instance, session, XR_PREFERRED_COLOR_FORMAT);

    for (uint32_t i = 0; i < _viewCount; i++) {
        _images[i].clear();

        XrSwapchainCreateInfo info = {
            .type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
            .createFlags = 0,
            .usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
            .format = format,
            .sampleCount = _viewConfigs[i].recommendedSwapchainSampleCount,
            .width = _viewConfigs[i].recommendedImageRectWidth,
            .height = _viewConfigs[i].recommendedImageRectHeight,
            .faceCount = 1,
            .arraySize = 1,
            .mipCount = 1,
        };

        XrResult result = xrCreateSwapchain(session, &info, &_swapChains[i]);
        if (!xrCheck(instance, result, "Failed to create swapchain!"))
            return false;

        result = xrEnumerateSwapchainImages(_swapChains[i], 0, &_swapChainLengths[i], nullptr);
        if (!xrCheck(instance, result, "Failed to enumerate swapchains"))
            return false;

        for (uint32_t j = 0; j < _swapChainLengths[i]; j++) {
            XrSwapchainImageOpenGLKHR image = { .type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR };
            _images[i].push_back(image);
        }
        result = xrEnumerateSwapchainImages(_swapChains[i], _swapChainLengths[i], &_swapChainLengths[i],
                                            (XrSwapchainImageBaseHeader*)_images[i].data());
        if (!xrCheck(instance, result, "Failed to enumerate swapchain images"))
            return false;
    }

    return true;
}

bool OpenXrDisplayPlugin::initLayers() {
    for (uint32_t i = 0; i < _viewCount; i++) {
        XrCompositionLayerProjectionView layer = {
            .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW,
            .subImage = {
                .swapchain = _swapChains[i],
                .imageRect = {
                    .offset = {
                        .x = 0,
                        .y = 0,
                    },
                    .extent = {
                        .width = (int32_t)_viewConfigs[i].recommendedImageRectWidth,
                        .height = (int32_t)_viewConfigs[i].recommendedImageRectHeight,
                    },
                },
                .imageArrayIndex = 0,
            },
        };
        _projectionLayerViews.push_back(layer);
    };

    return true;
}

void OpenXrDisplayPlugin::init() {
    Plugin::init();

    if (!initViews()) {
        qCCritical(xr_display_cat, "View init failed.");
        return;
    }

    for (const XrViewConfigurationView& view : _viewConfigs) {
        assert(view.recommendedImageRectWidth != 0);
        qCDebug(xr_display_cat, "Swapchain dimensions: %dx%d", view.recommendedImageRectWidth, view.recommendedImageRectHeight);
        // TODO: Don't render side-by-side but use multiview (texture arrays). This probably won't work with GL.
        _renderTargetSize.x = view.recommendedImageRectWidth * 2;
        _renderTargetSize.y = view.recommendedImageRectHeight;
    }

    emit deviceConnected(getName());
}

const QString OpenXrDisplayPlugin::getName() const {
    return QString("OpenXR: %1").arg(_context->_systemName);
}

bool OpenXrDisplayPlugin::internalActivate() {
    if (!_context->_isValid) { return false; }

    _context->reset();
    _context->_isDisplayActive = true;
    return HmdDisplayPlugin::internalActivate();
}

void OpenXrDisplayPlugin::internalDeactivate() {
    _context->_isDisplayActive = false;
    HmdDisplayPlugin::internalDeactivate();
}

void OpenXrDisplayPlugin::customizeContext() {
    gl::initModuleGl();
    HmdDisplayPlugin::customizeContext();

    if (!_context->initPostGraphics()) {
        qCCritical(xr_display_cat, "Post graphics init failed.");
        return;
    }

    if (!initSwapChains()) {
        qCCritical(xr_display_cat, "Swap chain init failed.");
        return;
    }

    if (!initLayers()) {
        qCCritical(xr_display_cat, "Layer init failed.");
        return;
    }

    // Create swap chain images for _compositeFramebuffer
    for (size_t i = 0; i < _swapChainLengths[0]; ++i) {
        gpu::TexturePointer texture =
            gpu::Texture::createRenderBuffer(gpu::Element::COLOR_SRGBA_32, _renderTargetSize.x, _renderTargetSize.y,
                                             gpu::Texture::SINGLE_MIP, gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_POINT));
        _compositeSwapChain.push_back(texture);
    }
}

void OpenXrDisplayPlugin::uncustomizeContext() {
    _compositeSwapChain.clear();
    _projectionLayerViews.clear();
    for (uint32_t i = 0; i < _viewCount; i++) {
        _images[i].clear();
    }
    HmdDisplayPlugin::uncustomizeContext();
}

void OpenXrDisplayPlugin::resetSensors() {
}

bool OpenXrDisplayPlugin::beginFrameRender(uint32_t frameIndex) {
    if (!_context->_isValid) {
        deactivate();
        return false;
    }

    _context->pollEvents();

    if (_context->_shouldQuit) {
        QMetaObject::invokeMethod(qApp, "quit");
        return false;
    }

    if (!_context->_shouldRunFrameCycle) {
        qCWarning(xr_display_cat, "beginFrameRender: Shouldn't run frame cycle. Skipping renderin frame %d", frameIndex);
        return true;
    }

    _currentRenderFrameInfo = FrameInfo();
    _currentRenderFrameInfo.predictedDisplayTime = _lastFrameState.predictedDisplayTime / 1e9;

    withNonPresentThreadLock([&] {
        _currentRenderFrameInfo.renderPose = _context->_lastHeadPose.getMatrix();
        _currentRenderFrameInfo.presentPose = _context->_lastHeadPose.getMatrix();
        _frameInfos[frameIndex] = _currentRenderFrameInfo;
    });

    return HmdDisplayPlugin::beginFrameRender(frameIndex);
}

void OpenXrDisplayPlugin::submitFrame(const gpu::FramePointer& newFrame) {
    OpenGLDisplayPlugin::submitFrame(newFrame);
}

void OpenXrDisplayPlugin::compositeLayers() {
    if (!_context->_shouldRunFrameCycle) {
        return;
    }

    if (_lastFrameState.shouldRender) {
        _compositeFramebuffer->setRenderBuffer(0, _compositeSwapChain[_swapChainIndices[0]]);
        HmdDisplayPlugin::compositeLayers();
    }
}

void OpenXrDisplayPlugin::hmdPresent() {
    if (!_context->_isValid) {
        deactivate();
        return;
    }

    if (!_context->_shouldRunFrameCycle) {
        qCWarning(xr_display_cat, "hmdPresent: Shouldn't run frame cycle. Skipping renderin frame %d",
                  _currentFrame->frameIndex);
        return;
    }

    _lastFrameState = { .type = XR_TYPE_FRAME_STATE };
    XrResult result = xrWaitFrame(_context->_session, nullptr, &_lastFrameState);

    if (!xrCheck(_context->_instance, result, "xrWaitFrame failed"))
        return;

    if (!_context->beginFrame())
        return;

    if (_lastFrameState.shouldRender) {
        // TODO: Use multiview swapchain
        for (uint32_t i = 0; i < 2; i++) {
            XrSwapchainImageAcquireInfo acquireInfo = { .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };

            XrResult result = xrAcquireSwapchainImage(_swapChains[i], &acquireInfo, &_swapChainIndices[i]);
            if (!xrCheck(_context->_instance, result, "failed to acquire swapchain image!"))
                return;

            XrSwapchainImageWaitInfo waitInfo = { .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, .timeout = 1000 };
            result = xrWaitSwapchainImage(_swapChains[i], &waitInfo);
            if (!xrCheck(_context->_instance, result, "failed to wait for swapchain image!"))
                return;
        }

        GLuint glTexId = getGLBackend()->getTextureID(_compositeFramebuffer->getRenderBuffer(0));

        glCopyImageSubData(glTexId, GL_TEXTURE_2D, 0, 0, 0, 0, _images[0][_swapChainIndices[0]].image, GL_TEXTURE_2D, 0, 0, 0,
                           0, _renderTargetSize.x / 2, _renderTargetSize.y, 1);

        glCopyImageSubData(glTexId, GL_TEXTURE_2D, 0, _renderTargetSize.x / 2, 0, 0, _images[1][_swapChainIndices[1]].image,
                           GL_TEXTURE_2D, 0, 0, 0, 0, _renderTargetSize.x / 2, _renderTargetSize.y, 1);

        for (uint32_t i = 0; i < 2; i++) {
            XrSwapchainImageReleaseInfo releaseInfo = { .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
            XrResult result = xrReleaseSwapchainImage(_swapChains[i], &releaseInfo);
            if (!xrCheck(_context->_instance, result, "failed to release swapchain image!")) {
                assert(false);
                return;
            }
        }
    }

    endFrame();

    _presentRate.increment();
}

bool OpenXrDisplayPlugin::endFrame() {
    XrCompositionLayerProjection projectionLayer = {
        .type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
        .layerFlags = 0,
        .space = _context->_stageSpace,
        .viewCount = _viewCount,
        .views = _projectionLayerViews.data(),
    };

    std::vector<const XrCompositionLayerBaseHeader*> layers = {
        (const XrCompositionLayerBaseHeader*)&projectionLayer,
    };

    XrFrameEndInfo info = {
        .type = XR_TYPE_FRAME_END_INFO,
        .displayTime = _lastFrameState.predictedDisplayTime,
        .environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
        .layerCount = (uint32_t)layers.size(),
        .layers = layers.data(),
    };

    if ((_lastViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) {
        info.layerCount = 0;
    }

    if (!_lastFrameState.shouldRender) {
        info.layerCount = 0;
    }

    XrResult result = xrEndFrame(_context->_session, &info);
    if (!xrCheck(_context->_instance, result, "failed to end frame!")) {
        return false;
    }

    return true;
}

void OpenXrDisplayPlugin::postPreview() {
}

bool OpenXrDisplayPlugin::isHmdMounted() const {
    return _context->_hmdMounted;
}

void OpenXrDisplayPlugin::updatePresentPose() {
    if (!_context->_isValid) {
        deactivate();
        return;
    }

    if (_lastFrameState.predictedDisplayTime == 0) { return; }

    _context->_lastPredictedDisplayTime = _lastFrameState.predictedDisplayTime;

    auto predictedDisplayTime = _lastFrameState.predictedDisplayTime;

    std::vector<XrView> eye_views(_viewCount);
    for (uint32_t i = 0; i < _viewCount; i++) {
        eye_views[i].type = XR_TYPE_VIEW;
    }

    // TODO: Probably shouldn't call xrLocateViews twice. Use only view space views?
    XrViewLocateInfo eyeViewLocateInfo = {
        .type = XR_TYPE_VIEW_LOCATE_INFO,
        .viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        .displayTime = predictedDisplayTime,
        .space = _context->_viewSpace,
    };

    XrViewState eyeViewState = { .type = XR_TYPE_VIEW_STATE };

    XrResult result = xrLocateViews(_context->_session, &eyeViewLocateInfo, &eyeViewState, _viewCount, &_viewCount, eye_views.data());
    if (!xrCheck(_context->_instance, result, "Could not locate views"))
        return;

    for (uint32_t i = 0; i < 2; i++) {
        vec3 eyePosition = xrVecToGlm(eye_views[i].pose.position);
        quat eyeOrientation = xrQuatToGlm(eye_views[i].pose.orientation);
        _eyeOffsets[i] = controller::Pose(eyePosition, eyeOrientation).getMatrix();
    }

    _lastViewState = { .type = XR_TYPE_VIEW_STATE };

    XrViewLocateInfo viewLocateInfo = {
        .type = XR_TYPE_VIEW_LOCATE_INFO,
        .viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        .displayTime = predictedDisplayTime,
        .space = _context->_stageSpace,
    };

    result = xrLocateViews(_context->_session, &viewLocateInfo, &_lastViewState, _viewCount, &_viewCount, _views.value().data());
    if (!xrCheck(_context->_instance, result, "Could not locate views"))
        return;

    for (uint32_t i = 0; i < _viewCount; i++) {
        _projectionLayerViews[i].pose = _views.value()[i].pose;
        _projectionLayerViews[i].fov = _views.value()[i].fov;
    }

    XrSpaceLocation headLocation = {
        .type = XR_TYPE_SPACE_LOCATION,
        .pose = XR_INDENTITY_POSE,
    };
    xrLocateSpace(_context->_viewSpace, _context->_stageSpace, predictedDisplayTime, &headLocation);

    glm::vec3 headPosition = xrVecToGlm(headLocation.pose.position);
    glm::quat headOrientation = xrQuatToGlm(headLocation.pose.orientation);
    _context->_lastHeadPose = controller::Pose(headPosition, headOrientation);

    _currentPresentFrameInfo.presentPose = _context->_lastHeadPose.getMatrix();
    _currentPresentFrameInfo.predictedDisplayTime = _lastFrameState.predictedDisplayTime / 1e9;
}

int OpenXrDisplayPlugin::getRequiredThreadCount() const {
    return HmdDisplayPlugin::getRequiredThreadCount();
}

QRectF OpenXrDisplayPlugin::getPlayAreaRect() {
    return QRectF(0, 0, 10, 10);
}

DisplayPlugin::StencilMaskMeshOperator OpenXrDisplayPlugin::getStencilMaskMeshOperator() {
    return nullptr;
}
