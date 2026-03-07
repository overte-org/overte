//
//  Framebuffer.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 4/12/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Framebuffer_h
#define hifi_gpu_Framebuffer_h

#include "Texture.h"
#include "ResourceSwapChain.h"
#include <memory>

class Transform; // Texcood transform util

namespace gpu {

typedef Element Format;

/**
 * API-independent class representing a framebuffer.
 */
class Framebuffer {
public:
    /**
     * DOCTODO
     */
    enum BufferMask {
        BUFFER_COLOR0 = 1,
        BUFFER_COLOR1 = 2,
        BUFFER_COLOR2 = 4,
        BUFFER_COLOR3 = 8,
        BUFFER_COLOR4 = 16,
        BUFFER_COLOR5 = 32,
        BUFFER_COLOR6 = 64,
        BUFFER_COLOR7 = 128,
        BUFFER_COLORS = 0x000000FF,

        BUFFER_DEPTH = 0x40000000,
        BUFFER_STENCIL = 0x80000000,
        BUFFER_DEPTHSTENCIL = 0xC0000000,
    };
    typedef uint32 Masks;

    ~Framebuffer();

    /**
     * @brief Creates a framebuffer with a given name. Does not create any render buffers.
     * @param name Name of the framebuffer.
     * @return Pointer to the framebuffer object.
     */
    static Framebuffer* create(const std::string& name);

    /**
     * @brief Creates a framebuffer with a color render buffer.
     *
     * @param name Name of the framebuffer.
     * @param colorBufferFormat Format of the color render buffer to be created.
     * @param width With of the framebuffer.
     * @param height Height of the framebuffer.
     * @return Pointer to the framebuffer object.
     */
    static Framebuffer* create(const std::string& name, const Format& colorBufferFormat, uint16 width, uint16 height);

    /**
     * @brief Creates a framebuffer with a color and depth stencil render buffers.
     *
     * @param name Name of the framebuffer.
     * @param colorBufferFormat Format of the color render buffer to be created.
     * @param depthStencilBufferFormat Format of the depth stencil render buffer to be created.
     * @param width With of the framebuffer.
     * @param height Height of the framebuffer.
     * @return Pointer to the framebuffer object.
     */
    static Framebuffer* create(const std::string& name, const Format& colorBufferFormat, const Format& depthStencilBufferFormat, uint16 width, uint16 height);

    // Render buffers
    /**
     * @brief Removes all render buffers from the current framebuffer.
     */
    void removeRenderBuffers();

    /**
     * @return Number of render buffers in the framebuffer.
     */
    uint32 getNumRenderBuffers() const;

    /**
     * @return Render buffers as TextureView objects.
     */
    const TextureViews& getRenderBuffers() const { return _renderBuffers; }

    /**
     * @brief Sets a given render buffer slot in the framebuffer.
     *
     * @param slot Index of the slot to set.
     * @param texture Texture to set it to.
     * @param subresource Subresource index for when texture has several subresources. Used for shadow cascades and SSAO.
     * @return
     */
    int32 setRenderBuffer(uint32 slot, const TexturePointer& texture, uint32 subresource = 0);

    /**
     * @brief Returns render buffer from particular slot.
     *
     * @param slot Slot index.
     * @return Pointer to the render buffer.
     */
    const TexturePointer& getRenderBuffer(uint32 slot) const;

    /**
     * @brief Get subresource index for a give slot.
     * @param slot Slot index.
     * @return Subresource index.
     */
    uint32 getRenderBufferSubresource(uint32 slot) const;

    /**
     * @brief Sets depth render buffer.
     *
     * @param texture Texture to be set as render buffer.
     * @param format Depth format for the render buffer.
     * @param subresource Subresource index, used when texture has several subresources, for example for shadow cascades.
     * @return `true` if render buffer was set successfully.
     */
    bool setDepthBuffer(const TexturePointer& texture, const Format& format, uint32 subresource = 0);

    /**
     * @brief Sets depth render buffer.
     *
     * @param texture Texture to be set as render buffer.
     * @param format Stencil format for the render buffer.
     * @param subresource Subresource index, used when texture has several subresources.
     * @return `true` if render buffer was set successfully.
     */
    bool setStencilBuffer(const TexturePointer& texture, const Format& format, uint32 subresource = 0);

    /**
     * @brief Sets the combined depth stencil render buffer.
     *
     * @param texture Texture to be set as render buffer.
     * @param format Depth stencil format for the render buffer.
     * @param subresource Subresource index, used when texture has several subresources.
     * @return `true` if render buffer was set successfully.
     */
    bool setDepthStencilBuffer(const TexturePointer& texture, const Format& format, uint32 subresource = 0);

    /**
     * @return Shared pointer to a depth stencil buffer. It can be `nullptr` if the render buffer does not exist.
     */
    const TexturePointer& getDepthStencilBuffer() const;

    /**
     * @return Subresource index of the current depth stencil buffer. Multiple subresources on depth buffers are used for shadow cascades.
     */
    uint32 getDepthStencilBufferSubresource() const;

    /**
     * @return Depth stencil buffer format.
     */
    Format getDepthStencilBufferFormat() const;


    // Properties
    /**
     * Buffer mask is used for checking what types of render buffers are populated in the particular framebuffer.
     *
     * @return Buffer mask.
     */
    Masks getBufferMask() const { return _bufferMask; }

    /**
     * @return `true` if there's no render buffers in the current framebuffer.
     */
    bool isEmpty() const { return (_bufferMask == 0); }

    /**
     * @return `true` if the framebuffer has one or more color render buffers.
     */
    bool hasColor() const { return (getBufferMask() & BUFFER_COLORS); }

    /**
     * @return `true` if render buffer has a depth stencil render buffer.
     */
    bool hasDepthStencil() const { return (getBufferMask() & BUFFER_DEPTHSTENCIL); }

    /**
     * @return `true` if render buffer has a depth render buffer.
     */
    bool hasDepth() const { return (getBufferMask() & BUFFER_DEPTH); }

    /**
     * @return `true` if render buffer has a stencil render buffer.
     */
    bool hasStencil() const { return (getBufferMask() & BUFFER_STENCIL); }

    /**
     * @brief Checks if a texture is compatible with being a render attachment.
     *
     * Checks if texture size and number of samples is the same as for the framebuffer.
     *
     * @param texture Texture to check.
     * @param subresource Index of the subresource to check. Currently not used.
     * @return `true` if texture passed the check.
     */
    bool validateTargetCompatibility(const Texture& texture, uint32 subresource = 0) const;

    /**
     * @return Framebuffer size in pixels.
     */
    Vec2u getSize() const { return Vec2u(getWidth(), getHeight()); }

    /**
     * @return Framebuffer width in pixels.
     */
    uint16 getWidth() const;

    /**
     * @return Framebuffer height in pixels.
     */
    uint16 getHeight() const;

    /**
     * @return Number of samples. It's greater than one for MSAA.
     */
    uint16 getNumSamples() const;

    /**
     * @return Framebuffer name.
     */
    const std::string& getName() const { return _name; }

    /**
     * @brief Sets the framebuffer name.
     *
     * @param name New name.
     */
    void setName(const std::string& name) { _name = name; }

    /**
     * @return Framebuffer width divided by height.
     */
    float getAspectRatio() const { return getWidth() / (float) getHeight() ; }

#if !defined(Q_OS_ANDROID)
    static const uint32 MAX_NUM_RENDER_BUFFERS = 8;
#else    
    static const uint32 MAX_NUM_RENDER_BUFFERS = 4;
#endif
    /**
     * @return Returns maximum number of render buffers. The result is platform-dependent.
     */
    static uint32 getMaxNumRenderBuffers() { return MAX_NUM_RENDER_BUFFERS; }

    /**
     * Graphics API-specific object representing this framebuffer on the backend side.
     */
    const GPUObjectPointer gpuObject {};

    /**
     * Stamp is a counter that is incremented on changes and then compared with the stamp on backend-specific object
     * side to determine if backend-specific object needs to be updated.
     * @return Current stamp for depth render buffer.
     */
    Stamp getDepthStamp() const { return _depthStamp; }

    /**
     * Stamp is a counter that is incremented on changes and then compared with the stamp on backend-specific object
     * side to determine if backend-specific object needs to be updated.
     * @return Stamps for color buffers.
     */
    const std::vector<Stamp>& getColorStamps() const { return _colorStamps; }

    /**
     * @brief Used to get coefficients for mapping one surface to an area on the other surface.
     *
     * @param sourceSurface Size of the source surface in pixels.
     * @param destRegionSize Size of the destination region in pixels.
     * @param destRegionOffset Start position of the destination region in pixels.
     * @return `vec4` where x - x start coefficient, y - y start coefficient, z - width coefficient, w - height coefficient.
     */
    static glm::vec4 evalSubregionTexcoordTransformCoefficients(const glm::ivec2& sourceSurface, const glm::ivec2& destRegionSize, const glm::ivec2& destRegionOffset = glm::ivec2(0));

    /**
     * @brief Used to get coefficients for mapping one surface to an area on the other surface.
     *
     * @param sourceSurface Size of the source surface in pixels.
     * @param destViewport x,y - destination region offset, z, w - destination region size.
     * @return `vec4` where x - x start coefficient, y - y start coefficient, z - width coefficient, w - height coefficient.
     */
    static glm::vec4 evalSubregionTexcoordTransformCoefficients(const glm::ivec2& sourceSurface, const glm::ivec4& destViewport);

    /**
     * @brief Used to get transform for mapping one surface to an area on the other surface.
     *
     * @param sourceSurface Size of the source surface in pixels.
     * @param destRegionSize Size of the destination region in pixels.
     * @param destRegionOffset Start position of the destination region in pixels.
     * @return Transform from source to destination.
     */
    static Transform evalSubregionTexcoordTransform(const glm::ivec2& sourceSurface, const glm::ivec2& destRegionSize, const glm::ivec2& destRegionOffset = glm::ivec2(0));

    /**
     * @brief Used to get transform for mapping one surface to an area on the other surface.
     *
     * @param sourceSurface Size of the source surface in pixels.
     * @param destViewport x,y - destination region offset, z, w - destination region size.
     * @return Transform from source to destination.
     */
    static Transform evalSubregionTexcoordTransform(const glm::ivec2& sourceSurface, const glm::ivec4& destViewport);

protected:
    /// Framebuffer name.
    std::string _name;

    /// A counter that is incremented on changes and then compared with the stamp on backend-specific object
    /// to determine if backend-specific object for depth render buffer needs to be updated.
    Stamp _depthStamp { 0 };

    /// Counters that are incremented on changes and then compared with the stamps on backend-specific objects
    /// to determine if backend-specific objects for color render buffers needs to be updated.
    std::vector<Stamp> _colorStamps;

    /// Color render buffers
    TextureViews _renderBuffers;

    /// Depth, stencil, or combined depth stencil rneder buffer.
    TextureView _depthStencilBuffer;

    /// Bit sum of `BufferMask` enums. Used to check which buffers are populated.
    Masks _bufferMask = 0;

    /// Framebuffer width in pixels.
    uint16 _width = 0;

    /// Framebuffer height in pixels.
    uint16 _height = 0;

    /// Number of samples per pixel. Greater than 1 when MSAA is used.
    uint16 _numSamples = 0;

    /**
     * @brief Copy width, height and sample count from a texture.
     *
     * If `texture` is `nullptr`, then width, height and sample count are set to 0.
     * @param texture Shared pointer to a texture to copy width, height and sample count from. Can be `nullptr`.
     */
    void updateSize(const TexturePointer& texture);

    /**
     * @brief Sets depth stencil buffer.
     *
     * @param texture Shared pointer to a texture. Can be `nullptr`.
     * @param format Depth stencil format.
     * @param subresource Subresource index. Used when texture has multiple subresources, for example with shadow cascades.
     * @return `true` if depth stencil render buffer was assigned successfully.
     */
    bool assignDepthStencilBuffer(const TexturePointer& texture, const Format& format, uint32 subresource);

    friend class Serializer;
    friend class Deserializer;
    // Non exposed
    Framebuffer(const Framebuffer& framebuffer) = delete;
    Framebuffer() {}
};
typedef std::shared_ptr<Framebuffer> FramebufferPointer;
typedef ResourceSwapChain<Framebuffer> FramebufferSwapChain;
typedef std::shared_ptr<FramebufferSwapChain> FramebufferSwapChainPointer;

}

#endif
