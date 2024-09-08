//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "VKFramebuffer.h"
#include "VKBackend.h"

void gpu::vulkan::VKFramebuffer::update() {
    // VKTODO
    /*gl::GLTexture* gltexture = nullptr;
    TexturePointer surface;
    if (_gpuObject.getColorStamps() != _colorStamps) {
        if (_gpuObject.hasColor()) {
            _colorBuffers.clear();
            static const GLenum colorAttachments[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
                GL_COLOR_ATTACHMENT4,
                GL_COLOR_ATTACHMENT5,
                GL_COLOR_ATTACHMENT6,
                GL_COLOR_ATTACHMENT7,
                GL_COLOR_ATTACHMENT8,
                GL_COLOR_ATTACHMENT9,
                GL_COLOR_ATTACHMENT10,
                GL_COLOR_ATTACHMENT11,
                GL_COLOR_ATTACHMENT12,
                GL_COLOR_ATTACHMENT13,
                GL_COLOR_ATTACHMENT14,
                GL_COLOR_ATTACHMENT15 };

            int unit = 0;
            auto backend = _backend.lock();
            for (auto& b : _gpuObject.getRenderBuffers()) {
                surface = b._texture;
                if (surface) {
                    Q_ASSERT(TextureUsageType::RENDERBUFFER == surface->getUsageType());
                    gltexture = backend->syncGPUObject(surface);
                } else {
                    gltexture = nullptr;
                }

                if (gltexture) {
                    if (gltexture->_target == GL_TEXTURE_2D) {
                        glNamedFramebufferTexture(_id, colorAttachments[unit], gltexture->_texture, 0);
                    } else if (gltexture->_target == GL_TEXTURE_2D_MULTISAMPLE) {
                        glNamedFramebufferTexture(_id, colorAttachments[unit], gltexture->_texture, 0);
                    } else {
                        glNamedFramebufferTextureLayer(_id, colorAttachments[unit], gltexture->_texture, 0, b._subresource);
                    }
                    _colorBuffers.push_back(colorAttachments[unit]);
                } else {
                    glNamedFramebufferTexture(_id, colorAttachments[unit], 0, 0);
                }
                unit++;
            }
        }
        _colorStamps = _gpuObject.getColorStamps();
    }

    GLenum attachement = GL_DEPTH_STENCIL_ATTACHMENT;
    if (!_gpuObject.hasStencil()) {
        attachement = GL_DEPTH_ATTACHMENT;
    } else if (!_gpuObject.hasDepth()) {
        attachement = GL_STENCIL_ATTACHMENT;
    }

    if (_gpuObject.getDepthStamp() != _depthStamp) {
        auto surface = _gpuObject.getDepthStencilBuffer();
        auto backend = _backend.lock();
        if (_gpuObject.hasDepthStencil() && surface) {
            Q_ASSERT(TextureUsageType::RENDERBUFFER == surface->getUsageType());
            gltexture = backend->syncGPUObject(surface);
        }

        if (gltexture) {
            if (gltexture->_target == GL_TEXTURE_2D) {
                glNamedFramebufferTexture(_id, attachement, gltexture->_texture, 0);
            }
            else if (gltexture->_target == GL_TEXTURE_2D_MULTISAMPLE) {
                glNamedFramebufferTexture(_id, attachement, gltexture->_texture, 0);
            } else {
                glNamedFramebufferTextureLayer(_id, attachement, gltexture->_texture, 0,
                                               _gpuObject.getDepthStencilBufferSubresource());
            }
        } else {
            glNamedFramebufferTexture(_id, attachement, 0, 0);
        }
        _depthStamp = _gpuObject.getDepthStamp();
    }

    // Last but not least, define where we draw
    if (!_colorBuffers.empty()) {
        glNamedFramebufferDrawBuffers(_id, (GLsizei)_colorBuffers.size(), _colorBuffers.data());
    } else {
        glNamedFramebufferDrawBuffer(_id, GL_NONE);
    }

    // Now check for completness
    _status = glCheckNamedFramebufferStatus(_id, GL_DRAW_FRAMEBUFFER);

    // restore the current framebuffer
    checkStatus();*/
}

bool gpu::vulkan::VKFramebuffer::checkStatus(gpu::vulkan::VKFramebuffer::FramebufferStatus target) const {
    // VKTODO
    /*switch (_status) {
        case GL_FRAMEBUFFER_COMPLETE:
            // Success !
            return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_UNSUPPORTED.";
            break;
#if !defined(USE_GLES)
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.";
            break;
#endif
        default:
            break;
    }
    return false;
*/
}

gpu::vulkan::VKFramebuffer::~VKFramebuffer() {
    //VKTODO
    /*if (_id) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseFramebuffer(_id);
        }
    }*/
}

#if 0

using namespace gpu;
using namespace gpu::gl;

VKFramebuffer::~VKFramebuffer() { 
    if (_id) { 
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseFramebuffer(_id);
        }
    } 
}

bool VKFramebuffer::checkStatus(VKenum target) const {
    bool result = false;
    switch (_status) {
    case VK_FRAMEBUFFER_COMPLETE:
        // Success !
        result = true;
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.";
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.";
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.";
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.";
        break;
    case VK_FRAMEBUFFER_UNSUPPORTED:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_UNSUPPORTED.";
        break;
    }
    return result;
}
#endif