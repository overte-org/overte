//
//  Shader.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 2/27/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Shader_h
#define hifi_gpu_Shader_h

#include "Resource.h"
#include <string>
#include <memory>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <functional>
#include <shaders/Shaders.h>
#include <QUrl>

namespace gpu {

class Shader {
public:
    // unique identifier of a shader
    using ID = uint32_t;

    enum Type
    {
        VERTEX = 0,
        PIXEL,
        FRAGMENT = PIXEL,
        GEOMETRY,
        NUM_DOMAINS,

        PROGRAM,
    };

    typedef std::shared_ptr<Shader> Pointer;
    typedef std::vector<Pointer> Shaders;

    using Source = shader::Source;
    using Reflection = shader::Reflection;
    using Dialect = shader::Dialect;
    using Variant = shader::Variant;

    /// Helper class containing compilation log and variable stating if compilation was successful.
    struct CompilationLog {
        /// Log from shader compilation.
        std::string message;

        /// `true` if compilation was successful.
        bool compiled{ false };

        CompilationLog() {}
        CompilationLog(const CompilationLog& src) : message(src.message), compiled(src.compiled) {}
    };
    using CompilationLogs = std::vector<CompilationLog>;

    /**
     * @brief Return an object containing sources and reflections of the shader.
     *
     * @param id Index of the shader, as defined in ShaderEnums.h.
     * @return Object containing sources and reflections of the shader.
     */
    static const Source& getShaderSource(uint32_t id);

    /**
     * @brief Return an object containing sources and reflections of the shader.
     *
     * TODO: this is misleading, it can return either vertex of fragment source, depending on which index you use.
     *  It would be best to add an assert to check for this.
     * @param id Index of the shader, as defined in ShaderEnums.h. Needs to be one of shader::render_utils::vertex.
     * @return Object containing sources and reflections of the shader.
     */
    static const Source& getVertexShaderSource(uint32_t id) { return getShaderSource(id); }

    /**
     * @brief Return an object containing sources and reflections of the shader.
     *
     * TODO: this is misleading, it can return either vertex of fragment source, depending on which index you use.
     *  It would be best to add an assert to check for this.
     * @param id Index of the shader, as defined in ShaderEnums.h. Needs to be one of shader::render_utils::fragment.
     * @return Object containing sources and reflections of the shader.
     */
    static const Source& getFragmentShaderSource(uint32_t id) { return getShaderSource(id); }

    /**
     * @brief Creates a vertex shader from given source object.
     *
     * Used for procedural shaders.
     * @param source Object containing shader's source and other data. Look in `Procedural::prepare()` for details.
     * @return Shared pointer to the shader.
     */
    static Pointer createVertex(const Source& source);

    /**
    * @brief Creates a fragment shader from given source object.
     *
     * Used for procedural shaders.
     * @param source Object containing shader's source and other data. Look in `Procedural::prepare()` for details.
     * @return Shared pointer to the shader.
     */
    static Pointer createPixel(const Source& source);

    /**
     * @brief Creates or reuses vertex shader with a given index.
     *
     * If shader was already created, then a pointer to the existing one will be returned.
     * @param shaderId One of either `shader::render_utils::vertex` or `shader::gpu::vertex` enums.
     * @return Shared pointer to the vertex shader.
     */
    static Pointer createVertex(uint32_t shaderId);

    /**
    * @brief Creates or reuses fragment shader with a given index.
     *
     * If shader was already created, then a pointer to the existing one will be returned.
     * @param shaderId One of either `shader::render_utils::fragment` or `shader::gpu::fragment` enums.
     * @return Shared pointer to the fragment shader.
     */
    static Pointer createPixel(uint32_t shaderId);

    /**
    * @brief Creates or reuses a program with a given index.
     *
     * Program contains both a vertex shader and a fragment shader.
     * If shader was already created, then a pointer to the existing one will be returned.
     * @param programId One of either `shader::render_utils::program` or `shader::gpu::program` enums.
     * @return Shared pointer to the program.
     */
    static Pointer createProgram(uint32_t programId);

    /**
     * @brief Creates a program from given vertex and fragment shader objects.
     *
     * @param vertexShader Vertex shader created by `createVertex`.
     * @param pixelShader Fragment shader created by `createPixel`.
     * @return Shared pointer to the resulting shader.
     */
    static Pointer createProgram(const Pointer& vertexShader, const Pointer& pixelShader);

    ~Shader();

    /**
     * @brief Get index of the shader.
     *
     * For shader objects that contain both fragment and vertex shader it returns combined ID of these.
     * Shader index enums can be found in ShaderEnums.h, which is generated during build time.
     * @return Index of the shader.
     */
    ID getID() const;

    /**
     * @brief Get the type of the current shader.
     *
     * PROGRAM type means that shader object contains both vertex and fragment shader.
     * @return Type of the shader.
     */
    Type getType() const { return _type; }

    /**
     * @return `true` if the shader object contains both vertex and fragment shader.
     */
    bool isProgram() const { return getType() > NUM_DOMAINS; }

    /**
     * @return `true` if the shader object contains only one type such as vertex or fragment shader, `false` if it contains both vertex and fragment shaders.
     */
    bool isDomain() const { return getType() < NUM_DOMAINS; }

    /**
     * @return Object containing shader sources and reflection.
     */
    const Source& getSource() const { return _source; }

    /**
     * @brief For a shader object that contains both vertex and fragment shaders it returns reference to a vector of shader objects.
     *
     * @return Reference to a vector of shader objects.
     */
    const Shaders& getShaders() const { return _shaders; }

    /**
     * @brief Gets the shader reflection for specific dialect and variant.
     *
     * @param dialect Version specific to currently used GPU API, for example OpenGL ES or  OpenGL 4.5.
     * @param variant Mono or stereo version of te shader.
     * @return Shader reflection.
     */
    Reflection getReflection(shader::Dialect dialect, shader::Variant variant) const;

    /**
     * @brief Get the default version of the reflection.
     *
     * Used when specific reflections are not needed and generic one will do, for example to check for resource
     * locations in `ShapePlumber::addPipeline`.
     * @return Shader reflection.
     */
    Reflection getReflection() const;


    /**
     * Compilation Handler can be passed while compiling a shader (in the makeProgram call) to be able to give the hand to
     * the caller thread if the compilation fails and to provide a different version of the source for it.
     * @param0 The Shader object that just failed to compile.
     * @param1 The original source code as submitted to the compiler.
     * @param2 The compilation log containing the error message.
     * @param3 A new string ready to be filled with the new version of the source that could be proposed from the handler functor.
     * @return Boolean true if the backend should keep trying to compile the shader with the new source returned or false to stop and fail that shader compilation.
     */
    using CompilationHandler = std::function<bool(const Shader&, const std::string&, CompilationLog&, std::string&)>;

    /**
     * @return `true` if the compilation finished and shader failed to compile, otherwise false.
     */
    bool compilationHasFailed() const { return _compilationHasFailed; }

    /**
     * @return Object containing compilation log and boolean stating if compilation was successful.
     */
    const CompilationLogs& getCompilationLogs() const { return _compilationLogs; }

    /**
     * @brief Render backend sets this when shader compilation finishes.
     *
     * Set Compilation logs can only be called by the Backend layers
     * @param compilationHasFailed
     */
    void setCompilationHasFailed(bool compilationHasFailed) { _compilationHasFailed = compilationHasFailed; }

    /**
     * @brief Sets compilation log objects.
     *
     * Compilation log object contains log output and boolean variable telling if compilation was successful.
     * @param logs
     */
    void setCompilationLogs(const CompilationLogs& logs) const;

    // Object representing shader on the API-specific backend side.
    const GPUObjectPointer gpuObject{};

protected:
    /**
     * @brief Create shader from source object.
     *
     * @param type Type of the shader.
     * @param source Shader source object.
     * @param dynamic `true` for procedural materials.
     */
    Shader(Type type, const Source& source, bool dynamic);

    /**
     * @brief Create shader object containing both vertex and fragment shader.
     *
     * @param type Type of the shader. Should be `PROGRAM`.
     * @param vertex Shared pointer to the vertex shader.
     * @param geometry Currently not implemented.
     * @param pixel Shared pointer to the pixel shader.
     */
    Shader(Type type, const Pointer& vertex, const Pointer& geometry, const Pointer& pixel);

    /// Source contains the actual source code or nothing if the shader is a program.
    const Source _source;

    /// If shader is composed of sub shaders, here they are.
    const Shaders _shaders;

     
    /// The type of the shader, the primary key.
    const Type _type;

    /// Compilation logs (one for each versions generated).
    mutable CompilationLogs _compilationLogs;

    /// `true` if the shader compilation failed.
    bool _compilationHasFailed{ false };

    /**
     * @brief Creates a shader or retrieves it from cache if it was already created.
     *
     * "Domain shader" means a shader for the specific part of the pipeline.
     * @param type Shader type. Can be `VERTEX` or `FRAGMENT`.
     * @param sourceId One of the ids in ShaderEnums.h.
     * @return Pointer to the shader that was created or retrieved from cache.
     */
    static ShaderPointer createOrReuseDomainShader(Type type, uint32_t sourceId);

    //. The IDs of the shaders in a program make its key.
    using ProgramMapKey = glm::uvec3;

    /// Used for sorting program keys in the map.
    class ProgramKeyLess {
    public:
        bool operator()(const ProgramMapKey& l, const ProgramMapKey& r) const {
            if (l.x == r.x) {
                if (l.y == r.y) {
                    return (l.z < r.z);
                } else {
                    return (l.y < r.y);
                }
            } else {
                return (l.x < r.x);
            }
        }
    };
    using ProgramMap = std::map<ProgramMapKey, std::weak_ptr<Shader>, ProgramKeyLess>;

    /// Used for caching and reusing already created shaders.
    static ProgramMap _programMap;

    /**
     * @brief Creates a shader or retrieves it from cache if it was already created.
     *
     * Program shader means a shader that contains shaders for all needed pipeline stages.
     * @param type Must be `PROGRAM`.
     * @param vertexShader Shared pointer to vertex shader.
     * @param geometryShader Not implemented.
     * @param pixelShader Shared pointer to fragment shader.
     * @return Pointer to the shader object that was created or retrieved from cache.
     */
    static ShaderPointer createOrReuseProgramShader(Type type,
                                                    const Pointer& vertexShader,
                                                    const Pointer& geometryShader,
                                                    const Pointer& pixelShader);
    friend class Serializer;
    friend class Deserializer;
};

typedef Shader::Pointer ShaderPointer;
typedef std::vector<ShaderPointer> Shaders;

};  // namespace gpu

#endif
