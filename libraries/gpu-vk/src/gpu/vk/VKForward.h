//
//  Created by Bradley Austin Davis on 2016/09/22
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
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



