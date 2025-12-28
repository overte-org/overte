//
//  Created by Bradley Austin Davis on 2016/09/22
//  Adapted for Vulkan in 2022-2025 by dr Karol Suprynowicz.
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2023-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//
#ifndef hifi_gpu_VKForward_h
#define hifi_gpu_VKForward_h

#include <vk/Config.h>
#include <gpu/Forward.h>
#include <gpu/Stream.h>

namespace gpu { namespace vk {

static const int MAX_NUM_ATTRIBUTES = Stream::NUM_INPUT_SLOTS;
// The drawcall Info attribute  channel is reserved and is the upper bound for the number of available Input buffers
static const int MAX_NUM_INPUT_BUFFERS = Stream::DRAW_CALL_INFO;

class VKBuffer;
class VKShader;
class VKTexture;
class VKBackend;
class VKFramebuffer;
class VKPipeline;
class VKQuery;

} }


#endif



