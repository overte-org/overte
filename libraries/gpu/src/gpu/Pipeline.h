//
//  Pipeline.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 3/8/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Pipeline_h
#define hifi_gpu_Pipeline_h

#include "Resource.h"
#include <memory>
#include <set>

#include "Shader.h"
#include "State.h"
 
namespace gpu {
/**
  * Contains render pipeline setup and pointer to the shaders.
  */
class Pipeline {
public:
    using Pointer = std::shared_ptr< Pipeline >;

    /**
     * @brief Creates new pipeline.
     *
     * @param program Shared pointer to the Shader object (contains both vertex and fragment shader).
     * @param state Render pipeline state.
     * @return Shared pointer to the newly created pipeline.
     */
    static Pointer create(const ShaderPointer& program, const StatePointer& state);
    ~Pipeline();

    /**
     * @return Object containing shaders.
     */
    const ShaderPointer& getProgram() const { return _program; }

    /**
     * @return Render pipeline state.
     */
    const StatePointer& getState() const { return _state; }

    /// Graphics API-specific object representing the pipeline.
    const GPUObjectPointer gpuObject {};
    
protected:
    /// Vertex and fragment shader.
    ShaderPointer _program;

    /// Pipeline setup.
    StatePointer _state;

    Pipeline();
};

typedef Pipeline::Pointer PipelinePointer;
typedef std::vector< PipelinePointer > Pipelines;

};


#endif
