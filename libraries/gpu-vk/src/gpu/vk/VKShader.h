//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKShader_h
#define hifi_gpu_vk_VKShader_h

#include "VKShared.h"

namespace gpu { namespace vk {

class VKShader : public GPUObject {
public:
    static VKShader* sync(VKBackend& backend, const Shader& shader);
    static bool makeProgram(VKBackend& backend, Shader& shader, const Shader::BindingSet& slotBindings);

    enum Version {
        Mono = 0,
        NumVersions
    };

    using ShaderObject = gpu::vk::ShaderObject;
    using ShaderObjects = std::array< ShaderObject, NumVersions >;

    using UniformMapping = std::map<VKint, VKint>;
    using UniformMappingVersions = std::vector<UniformMapping>;

    VKShader(const std::weak_ptr<VKBackend>& backend);
    ~VKShader();

    ShaderObjects _shaderObjects;
    UniformMappingVersions _uniformMappings;

    VKuint getProgram(Version version = Mono) const {
        return _shaderObjects[version].glprogram;
    }

    VKint getUniformLocation(VKint srcLoc, Version version = Mono) {
        // THIS will be used in the future PR as we grow the number of versions
        // return _uniformMappings[version][srcLoc];
        return srcLoc;
    }

    const std::weak_ptr<VKBackend> _backend;
};

} }


#endif
