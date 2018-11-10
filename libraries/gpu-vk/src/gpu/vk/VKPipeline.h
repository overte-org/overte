//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKPipeline_h
#define hifi_gpu_vk_VKPipeline_h

#include "VKShared.h"

namespace gpu { namespace vk {

class VKPipeline : public GPUObject {
public:
    static VKPipeline* sync(VKBackend& backend, const Pipeline& pipeline);

    VKShader* _program { nullptr };
    VKState* _state { nullptr };
    // Bit of a hack, any pipeline can need the camera correction buffer at execution time, so 
    // we store whether a given pipeline has declared the uniform buffer for it.
    int32 _cameraCorrection { -1 };
};

} }


#endif
