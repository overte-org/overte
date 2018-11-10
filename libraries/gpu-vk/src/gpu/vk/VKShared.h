//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_VKShared_h
#define hifi_gpu_VKShared_h

#include <vk/Config.h>
#include <gpu/Forward.h>
#include <gpu/Format.h>
#include <gpu/Context.h>

Q_DECLARE_LOGGING_CATEGORY(gpu_vk_logging)
Q_DECLARE_LOGGING_CATEGORY(trace_gpu_vk)
Q_DECLARE_LOGGING_CATEGORY(trace_gpu_vk_detail)

namespace gpu { namespace vulkan {

gpu::Size getDedicatedMemory();
ComparisonFunction comparisonFuncFromGL(vk::CompareOp func);
State::StencilOp stencilOpFromGL(vk::StencilOp stencilOp);
State::BlendOp blendOpFromGL(vk::BlendOp blendOp);
State::BlendArg blendArgFromGL(vk::BlendFactor blendArg);

struct ShaderObject {
    vk::ShaderModule glshader{ nullptr };
    int32_t transformCameraSlot { -1 };
    int32_t transformObjectSlot { -1 };
};

vk::Format evalTexelFormatInternal(const Element& dstFormat);

bool isDepthStencilFormat(vk::Format format);

static const vk::BlendOp BLEND_OPS_TO_VK[State::NUM_BLEND_OPS] = {
    vk::BlendOp::eAdd,
    vk::BlendOp::eSubtract,
    vk::BlendOp::eReverseSubtract,
    vk::BlendOp::eMin,
    vk::BlendOp::eMax
};

static const vk::BlendFactor BLEND_ARGS_TO_VK[State::NUM_BLEND_ARGS] = {
    vk::BlendFactor::eZero,
    vk::BlendFactor::eOne,
    vk::BlendFactor::eSrcColor,
    vk::BlendFactor::eOneMinusSrcColor,
    vk::BlendFactor::eSrcAlpha,
    vk::BlendFactor::eOneMinusSrcAlpha,
    vk::BlendFactor::eDstAlpha,
    vk::BlendFactor::eOneMinusDstAlpha,
    vk::BlendFactor::eDstColor,
    vk::BlendFactor::eOneMinusDstColor,
    vk::BlendFactor::eSrcAlphaSaturate,
    vk::BlendFactor::eConstantColor,
    vk::BlendFactor::eOneMinusConstantColor,
    vk::BlendFactor::eConstantAlpha,
    vk::BlendFactor::eOneMinusConstantAlpha,
};

static const vk::CompareOp COMPARISON_TO_VK[gpu::NUM_COMPARISON_FUNCS] = {
    vk::CompareOp::eNever,
    vk::CompareOp::eLess,
    vk::CompareOp::eEqual,
    vk::CompareOp::eLessOrEqual,
    vk::CompareOp::eGreater,
    vk::CompareOp::eNotEqual,
    vk::CompareOp::eGreaterOrEqual,
    vk::CompareOp::eAlways
};

static const vk::PrimitiveTopology PRIMITIVE_TO_VK[gpu::NUM_PRIMITIVES] = {
    vk::PrimitiveTopology::ePointList,
    vk::PrimitiveTopology::eLineList,
    vk::PrimitiveTopology::eLineStrip,
    vk::PrimitiveTopology::eTriangleList,
    vk::PrimitiveTopology::eTriangleStrip,
    vk::PrimitiveTopology::eTriangleFan,
};

//static const VKenum ELEMENT_TYPE_TO_VK[gpu::NUM_TYPES] = {
//    VK_FLOAT,
//    VK_INT,
//    VK_UNSIGNED_INT,
//    VK_HALF_FLOAT,
//    VK_SHORT,
//    VK_UNSIGNED_SHORT,
//    VK_BYTE,
//    VK_UNSIGNED_BYTE,
//    // Normalized values
//    VK_INT,
//    VK_UNSIGNED_INT,
//    VK_SHORT,
//    VK_UNSIGNED_SHORT,
//    VK_BYTE,
//    VK_UNSIGNED_BYTE
//};

bool checkGLError(const char* name = nullptr);
bool checkGLErrorDebug(const char* name = nullptr);

class VKBackend;

template <typename GPUType>
struct VKObject : public GPUObject {
public:
    VKObject(VKBackend& backend, const GPUType& gpuObject) : _gpuObject(gpuObject), _backend(backend.shared_from_this()) {}

    virtual ~VKObject() { }

    const GPUType& _gpuObject;
protected:
    const std::weak_ptr<VKBackend> _backend;
};

} } // namespace gpu::gl 

#define CHECK_VK_ERROR() gpu::vk::checkGLErrorDebug(__FUNCTION__)

#endif



