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
#include "VKBackend.h"

Q_DECLARE_LOGGING_CATEGORY(gpu_vk_logging)
Q_DECLARE_LOGGING_CATEGORY(trace_gpu_vk)
Q_DECLARE_LOGGING_CATEGORY(trace_gpu_vk_detail)

namespace gpu { namespace vk {

gpu::Size getDedicatedMemory();
ComparisonFunction comparisonFuncFromGL(VkCompareOp func);
State::StencilOp stencilOpFromGL(VkStencilOp stencilOp);
State::BlendOp blendOpFromGL(VkBlendOp blendOp);
State::BlendArg blendArgFromGL(VkBlendFactor blendArg);

struct ShaderObject {
    VkShaderModule glshader{ nullptr };
    int32_t transformCameraSlot { -1 };
    int32_t transformObjectSlot { -1 };
};

VkFormat evalTexelFormatInternal(const Element& dstFormat);

bool isDepthStencilFormat(VkFormat format);

static const VkBlendOp BLEND_OPS_TO_VK[State::NUM_BLEND_OPS] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_REVERSE_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX
};

static const VkBlendFactor BLEND_ARGS_TO_VK[State::NUM_BLEND_ARGS] = {
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    VK_BLEND_FACTOR_CONSTANT_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    VK_BLEND_FACTOR_CONSTANT_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
};

static const VkCompareOp COMPARISON_TO_VK[gpu::NUM_COMPARISON_FUNCS] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS
};

static const VkPrimitiveTopology PRIMITIVE_TO_VK[gpu::NUM_PRIMITIVES] = {
    VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
};

enum VKSyncState {
    // The object is currently undergoing no processing, although it's content
    // may be out of date, or it's storage may be invalid relative to the
    // owning GPU object
    Idle,
    // The object has been queued for transfer to the GPU
    Pending,
    // The object has been transferred to the GPU, but is awaiting
    // any post transfer operations that may need to occur on the
    // primary rendering thread
    Transferred,
};

// VKTODO is it needed?
/*static const enum ELEMENT_TYPE_TO_VK[gpu::NUM_TYPES] = {
    VK_FLOAT,
    VK_INT,
    VK_UNSIGNED_INT,
    VK_HALF_FLOAT,
    VK_SHORT,
    VK_UNSIGNED_SHORT,
    VK_BYTE,
    VK_UNSIGNED_BYTE,
    // Normalized values
    VK_INT,
    VK_UNSIGNED_INT,
    VK_SHORT,
    VK_UNSIGNED_SHORT,
    VK_BYTE,
    VK_UNSIGNED_BYTE
};*/

class VKBackend;

template <typename GPUType>
struct VKObject : public GPUObject {
public:
    VKObject(VKBackend& backend, const GPUType& gpuObject) : _gpuObject(gpuObject), _backend(backend.shared_from_this()) {}

    virtual ~VKObject() { }

    const GPUType& _gpuObject;
protected:
    // VKTODO: Maybe replace with regular pointer for efficiency
    const std::weak_ptr<VKBackend> _backend;
};

} } // namespace gpu::vulkan

#endif



