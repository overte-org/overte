//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#if 0
#include "VKShader.h"
#include "VKBackend.h"

using namespace gpu;
using namespace gpu::gl;

VKShader::VKShader(const std::weak_ptr<VKBackend>& backend) : _backend(backend) {
}

VKShader::~VKShader() {
    for (auto& so : _shaderObjects) {
        auto backend = _backend.lock();
        if (backend) {
            if (so.glshader != 0) {
                backend->releaseShader(so.glshader);
            }
            if (so.glprogram != 0) {
                backend->releaseProgram(so.glprogram);
            }
        }
    }
}

// VKSL version
static const std::string glslVersion {
    "#version 410 core"
};

// Shader domain
static const size_t NUM_SHADER_DOMAINS = 3;

// VK Shader type enums
// Must match the order of type specified in gpu::Shader::Type
static const std::array<VKenum, NUM_SHADER_DOMAINS> SHADER_DOMAINS { {
    VK_VERTEX_SHADER,
    VK_FRAGMENT_SHADER,
    VK_GEOMETRY_SHADER,
} };

// Domain specific defines
// Must match the order of type specified in gpu::Shader::Type
static const std::array<std::string, NUM_SHADER_DOMAINS> DOMAIN_DEFINES { {
    "#define GPU_VERTEX_SHADER",
    "#define GPU_PIXEL_SHADER",
    "#define GPU_GEOMETRY_SHADER",
} };

// Versions specific of the shader
static const std::array<std::string, VKShader::NumVersions> VERSION_DEFINES { {
    ""
} };

VKShader* compileBackendShader(VKBackend& backend, const Shader& shader) {
    // Any VKSLprogram ? normally yes...
    const std::string& shaderSource = shader.getSource().getCode();
    VKenum shaderDomain = SHADER_DOMAINS[shader.getType()];
    VKShader::ShaderObjects shaderObjects;

    for (int version = 0; version < VKShader::NumVersions; version++) {
        auto& shaderObject = shaderObjects[version];

        std::string shaderDefines = glslVersion + "\n" + DOMAIN_DEFINES[shader.getType()] + "\n" + VERSION_DEFINES[version];

        bool result = compileShader(shaderDomain, shaderSource, shaderDefines, shaderObject.glshader, shaderObject.glprogram);
        if (!result) {
            return nullptr;
        }
    }

    // So far so good, the shader is created successfully
    VKShader* object = new VKShader(backend.shared_from_this());
    object->_shaderObjects = shaderObjects;

    return object;
}

VKShader* compileBackendProgram(VKBackend& backend, const Shader& program) {
    if (!program.isProgram()) {
        return nullptr;
    }

    VKShader::ShaderObjects programObjects;

    for (int version = 0; version < VKShader::NumVersions; version++) {
        auto& programObject = programObjects[version];

        // Let's go through every shaders and make sure they are ready to go
        std::vector< VKuint > shaderGLObjects;
        for (auto subShader : program.getShaders()) {
            auto object = VKShader::sync(backend, *subShader);
            if (object) {
                shaderGLObjects.push_back(object->_shaderObjects[version].glshader);
            } else {
                qCDebug(gpu_vk_logging) << "VKShader::compileBackendProgram - One of the shaders of the program is not compiled?";
                return nullptr;
            }
        }

        VKuint glprogram = compileProgram(shaderGLObjects);
        if (glprogram == 0) {
            return nullptr;
        }

        programObject.glprogram = glprogram;

        makeProgramBindings(programObject);
    }

    // So far so good, the program versions have all been created successfully
    VKShader* object = new VKShader(backend.shared_from_this());
    object->_shaderObjects = programObjects;

    return object;
}

VKShader* VKShader::sync(VKBackend& backend, const Shader& shader) {
    VKShader* object = Backend::getGPUObject<VKShader>(shader);

    // If GPU object already created then good
    if (object) {
        return object;
    }
    // need to have a gpu object?
    if (shader.isProgram()) {
        VKShader* tempObject = compileBackendProgram(backend, shader);
        if (tempObject) {
            object = tempObject;
            Backend::setGPUObject(shader, object);
        }
    } else if (shader.isDomain()) {
        VKShader* tempObject = compileBackendShader(backend, shader);
        if (tempObject) {
            object = tempObject;
            Backend::setGPUObject(shader, object);
        }
    }

    glFinish();
    return object;
}

bool VKShader::makeProgram(VKBackend& backend, Shader& shader, const Shader::BindingSet& slotBindings) {

    // First make sure the Shader has been compiled
    VKShader* object = sync(backend, shader);
    if (!object) {
        return false;
    }

    // Apply bindings to all program versions and generate list of slots from default version
    for (int version = 0; version < VKShader::NumVersions; version++) {
        auto& shaderObject = object->_shaderObjects[version];
        if (shaderObject.glprogram) {
            Shader::SlotSet buffers;
            makeUniformBlockSlots(shaderObject.glprogram, slotBindings, buffers);

            Shader::SlotSet uniforms;
            Shader::SlotSet textures;
            Shader::SlotSet samplers;
            makeUniformSlots(shaderObject.glprogram, slotBindings, uniforms, textures, samplers);

            Shader::SlotSet inputs;
            makeInputSlots(shaderObject.glprogram, slotBindings, inputs);

            Shader::SlotSet outputs;
            makeOutputSlots(shaderObject.glprogram, slotBindings, outputs);

            // Define the public slots only from the default version
            if (version == 0) {
                shader.defineSlots(uniforms, buffers, textures, samplers, inputs, outputs);
            } else {
                VKShader::UniformMapping mapping;
                for (auto srcUniform : shader.getUniforms()) {
                    mapping[srcUniform._location] = uniforms.findLocation(srcUniform._name);
                }
                object->_uniformMappings.push_back(mapping);
            }
        }
    }

    return true;
}

#endif