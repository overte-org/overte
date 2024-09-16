//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "VKTexture.h"

#include <QtCore/QThread>
#include <NumericalConstants.h>

#include "VKBackend.h"

using namespace gpu;
using namespace gpu::vk;


/*const uint32_t VKTexture::CUBE_FACE_LAYOUT[VKTexture::TEXTURE_CUBE_NUM_FACES] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};*/

const uint32_t VKTexture::WRAP_MODES[Sampler::NUM_WRAP_MODES] = {
    VK_SAMPLER_ADDRESS_MODE_REPEAT,                         // WRAP_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,                // WRAP_MIRROR,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,                  // WRAP_CLAMP,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,                // WRAP_BORDER,
    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE        // WRAP_MIRROR_ONCE,
};

const VKFilterMode VKTexture::FILTER_MODES[Sampler::NUM_FILTERS] = {
    { VK_FILTER_NEAREST, VK_FILTER_NEAREST },  //FILTER_MIN_MAG_POINT,
    { VK_FILTER_NEAREST, VK_FILTER_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR,
    { VK_FILTER_LINEAR, VK_FILTER_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR },  //FILTER_MIN_MAG_LINEAR,

    // VKTODO: VkSamplerMipmapMode has some related setting too
    // VKTODO: This is not really accurate, but hopefully good enough for now
    { VK_FILTER_NEAREST, VK_FILTER_NEAREST },  //FILTER_MIN_MAG_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_NEAREST },  //FILTER_MIN_MAG_POINT_MIP_LINEAR,
    { VK_FILTER_NEAREST, VK_FILTER_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR },  //FILTER_MIN_POINT_MAG_MIP_LINEAR,
    { VK_FILTER_NEAREST, VK_FILTER_NEAREST },  //FILTER_MIN_LINEAR_MAG_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    { VK_FILTER_NEAREST, VK_FILTER_LINEAR },  //FILTER_MIN_MAG_LINEAR_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR },  //FILTER_MIN_MAG_MIP_LINEAR,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR }  //FILTER_ANISOTROPIC,
};

static constexpr size_t MAX_PIXEL_BYTE_SIZE{ 4 };
static constexpr size_t MAX_TRANSFER_DIMENSION{ 1024 };

/*const uvec3 GLVariableAllocationSupport::MAX_TRANSFER_DIMENSIONS{ MAX_TRANSFER_DIMENSION, MAX_TRANSFER_DIMENSION, 1 };
const uvec3 GLVariableAllocationSupport::INITIAL_MIP_TRANSFER_DIMENSIONS{ 64, 64, 1 };
const size_t GLVariableAllocationSupport::MAX_TRANSFER_SIZE = MAX_TRANSFER_DIMENSION * MAX_TRANSFER_DIMENSION * MAX_PIXEL_BYTE_SIZE;
const size_t GLVariableAllocationSupport::MAX_BUFFER_SIZE = MAX_TRANSFER_SIZE;*/

/*GLenum VKTexture::getVKTextureType(const Texture& texture) {
    switch (texture.getType()) {
        case Texture::TEX_2D:
            if (!texture.isArray()) {
                if (!texture.isMultisample()) {
                    return GL_TEXTURE_2D;
                } else {
                    return GL_TEXTURE_2D_MULTISAMPLE;
                }
            } else {
                if (!texture.isMultisample()) {
                    return GL_TEXTURE_2D_ARRAY;
                } else {
                    return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
                }
            }
            break;

        case Texture::TEX_CUBE:
            return GL_TEXTURE_CUBE_MAP;
            break;

        default:
            qFatal("Unsupported texture type");
    }
    Q_UNREACHABLE();
    return GL_TEXTURE_2D;
}


uint8_t VKTexture::getFaceCount(GLenum target) {
    switch (target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            return TEXTURE_2D_NUM_FACES;
        case GL_TEXTURE_CUBE_MAP:
            return TEXTURE_CUBE_NUM_FACES;
        default:
            Q_UNREACHABLE();
            break;
    }
}
const std::vector<GLenum>& VKTexture::getFaceTargets(GLenum target) {
    static const std::vector<GLenum> cubeFaceTargets {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    static const std::vector<GLenum> face2DTargets {
        GL_TEXTURE_2D
    };
    static const std::vector<GLenum> face2DMSTargets{
        GL_TEXTURE_2D_MULTISAMPLE
    };
    static const std::vector<GLenum> arrayFaceTargets{
        GL_TEXTURE_2D_ARRAY
    };
    switch (target) {
        case GL_TEXTURE_2D:
            return face2DTargets;
        case GL_TEXTURE_2D_MULTISAMPLE:
            return face2DMSTargets;
        case GL_TEXTURE_2D_ARRAY:
            return arrayFaceTargets;
        case GL_TEXTURE_CUBE_MAP:
            return cubeFaceTargets;
        default:
            Q_UNREACHABLE();
            break;
    }
    Q_UNREACHABLE();
    return face2DTargets;
}*/

VKTexture::VKTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture, bool isTransferable) :
    VKObject(*backend.lock(), texture),
    _target(getVKTextureType(texture)),
    _transferable(isTransferable),
    _downsampleSource(backend),
    _vkTexelFormat(evalTexelFormatInternal(texture.getTexelFormat()))
{
    Backend::setGPUObject(texture, this);
}

VKTexture::~VKTexture() {
    auto backend = _backend.lock();
    if (backend && _texture == VK_NULL_HANDLE) {
        // VKTODO
        // backend->releaseTexture(_id, 0);
    }
}

VkImageViewType VKTexture::getVKTextureType(const Texture& texture) {
    switch (texture.getType()) {
        case Texture::TEX_2D:
            if (!texture.isArray()) {
                if (!texture.isMultisample()) {
                    return VK_IMAGE_VIEW_TYPE_2D;
                } else {
                    // VKTODO: multisample?
                    return VK_IMAGE_VIEW_TYPE_2D;
                }
            } else {
                if (!texture.isMultisample()) {
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                } else {
                    // VKTODO multisample?
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                }
            }
            break;

        case Texture::TEX_CUBE:
            return VK_IMAGE_VIEW_TYPE_CUBE;
            break;

        default:
            qFatal("Unsupported texture type");
    }
    Q_UNREACHABLE();
    return VK_IMAGE_VIEW_TYPE_2D;
}

// From VKS
void VKAttachmentTexture::createTexture() {
    VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    imageCI.extent.width = _gpuObject.getWidth();
    imageCI.extent.height = _gpuObject.getHeight();
    imageCI.extent.depth = 1;
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = _gpuObject.isArray() ? _gpuObject.getNumSlices() : 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    if (_gpuObject.isColorRenderTarget()) {
        imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    } else if (_gpuObject.isDepthStencilRenderTarget()) {
        imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    } else {
        Q_ASSERT(false);
    }

    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    auto device = _backend.lock()->getContext().device->logicalDevice;

    // Create image for this attachment
    VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &vkImage));
    vkGetImageMemoryRequirements(device, vkImage, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = _backend.lock()->getContext().device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // VKTODO: this may need to be changed to VMA
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &vkDeviceMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, vkImage, vkDeviceMemory, 0));

    /*attachment.subresourceRange = {};
    attachment.subresourceRange.aspectMask = aspectMask;
    attachment.subresourceRange.levelCount = 1;
    attachment.subresourceRange.layerCount = createinfo.layerCount;*/

    /*VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
    imageView.viewType = (_gpuObject.isArray()) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    imageView.format = imageCI.format;
    // VKTODO: what is subresource?
    imageView.subresourceRange = attachment.subresourceRange;
    imageView.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
    imageView.image = attachment.imageCI;
    VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr, &attachment.view));*/

}

/*Size VKTexture::copyMipFaceFromTexture(uint16_t sourceMip, uint16_t targetMip, uint8_t face) const {
    if (!_gpuObject.isStoredMipFaceAvailable(sourceMip)) {
        return 0;
    }
    auto dim = _gpuObject.evalMipDimensions(sourceMip);
    auto mipData = _gpuObject.accessStoredMipFace(sourceMip, face);
    auto mipSize = _gpuObject.getStoredMipFaceSize(sourceMip, face);
    if (mipData) {
        GLTexelFormat texelFormat = GLTexelFormat::evalGLTexelFormat(_gpuObject.getTexelFormat(), _gpuObject.getStoredMipFormat());
        return copyMipFaceLinesFromTexture(targetMip, face, dim, 0, texelFormat.internalFormat, texelFormat.format, texelFormat.type, mipSize, mipData->readData());
    } else {
        qCDebug(gpugllogging) << "Missing mipData level=" << sourceMip << " face=" << (int)face << " for texture " << _gpuObject.source().c_str();
    }
    return 0;
}*/

//VKTODO:
/*GLExternalTexture::GLExternalTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture, GLuint id)
    : Parent(backend, texture, id) {
    Backend::textureExternalCount.increment();
}

GLExternalTexture::~GLExternalTexture() {
    auto backend = _backend.lock();
    if (backend) {
        auto recycler = _gpuObject.getExternalRecycler();
        if (recycler) {
            backend->releaseExternalTexture(_id, recycler);
        } else {
            qCWarning(gpugllogging) << "No recycler available for texture " << _id << " possible leak";
        }
        const_cast<GLuint&>(_id) = 0;
    }
    Backend::textureExternalCount.decrement();
}

GLVariableAllocationSupport::GLVariableAllocationSupport() {
    Backend::textureResourceCount.increment();
}

GLVariableAllocationSupport::~GLVariableAllocationSupport() {
    Backend::textureResourceCount.decrement();
    Backend::textureResourceGPUMemSize.update(_size, 0);
    Backend::textureResourcePopulatedGPUMemSize.update(_populatedSize, 0);
}

void GLVariableAllocationSupport::incrementPopulatedSize(Size delta) const {
    _populatedSize += delta;
    // Keep the 2 code paths to be able to debug
    if (_size < _populatedSize) {
        Backend::textureResourcePopulatedGPUMemSize.update(0, delta);
    } else  {
        Backend::textureResourcePopulatedGPUMemSize.update(0, delta);
    }
}

void GLVariableAllocationSupport::decrementPopulatedSize(Size delta) const {
    _populatedSize -= delta;
    // Keep the 2 code paths to be able to debug
    if (_size < _populatedSize) {
        Backend::textureResourcePopulatedGPUMemSize.update(delta, 0);
    } else  {
        Backend::textureResourcePopulatedGPUMemSize.update(delta, 0);
    }
}

void GLVariableAllocationSupport::sanityCheck() const {
    if (_populatedMip < _allocatedMip) {
        qCWarning(gpugllogging) << "Invalid mip levels";
    }
}*/

//VKTODO:
/*TransferJob::TransferJob(const Texture& texture,
                         uint16_t sourceMip,
                         uint16_t targetMip,
                         uint8_t face,
                         uint32_t lines,
                         uint32_t lineOffset) {
    auto transferDimensions = texture.evalMipDimensions(sourceMip);
    GLenum format;
    GLenum internalFormat;
    GLenum type;
    GLTexelFormat texelFormat = GLTexelFormat::evalGLTexelFormat(texture.getTexelFormat(), texture.getStoredMipFormat());
    format = texelFormat.format;
    internalFormat = texelFormat.internalFormat;
    type = texelFormat.type;
    _transferSize = texture.getStoredMipFaceSize(sourceMip, face);

    // If we're copying a subsection of the mip, do additional calculations to find the size and offset of the segment
    if (0 != lines) {
        transferDimensions.y = lines;
        auto dimensions = texture.evalMipDimensions(sourceMip);
        auto bytesPerLine = (uint32_t)_transferSize / dimensions.y;
        _transferOffset = bytesPerLine * lineOffset;
        _transferSize = bytesPerLine * lines;
    }

    Backend::texturePendingGPUTransferMemSize.update(0, _transferSize);

    if (_transferSize > GLVariableAllocationSupport::MAX_TRANSFER_SIZE) {
        qCWarning(gpugllogging) << "Transfer size of " << _transferSize << " exceeds theoretical maximum transfer size";
    }

    // Buffering can invoke disk IO, so it should be off of the main and render threads
    _bufferingLambda = [=](const TexturePointer& texture) {
        auto mipStorage = texture->accessStoredMipFace(sourceMip, face);
        if (mipStorage) {
            _mipData = mipStorage->createView(_transferSize, _transferOffset);
        } else {
            qCWarning(gpugllogging) << "Buffering failed because mip could not be retrieved from texture "
                                    << texture->source().c_str();
        }
    };

    _transferLambda = [=](const TexturePointer& texture) {
        if (_mipData) {
            auto gltexture = Backend::getGPUObject<VKTexture>(*texture);
            gltexture->copyMipFaceLinesFromTexture(targetMip, face, transferDimensions, lineOffset, internalFormat, format,
                                                   type, _mipData->size(), _mipData->readData());
            _mipData.reset();
        } else {
            qCWarning(gpugllogging) << "Transfer failed because mip could not be retrieved from texture "
                                    << texture->source().c_str();
        }
    };
}

TransferJob::TransferJob(uint16_t sourceMip, const std::function<void()>& transferLambda) :
    _sourceMip(sourceMip), _bufferingRequired(false), _transferLambda([=](const TexturePointer&) { transferLambda(); }) {}

TransferJob::~TransferJob() {
    Backend::texturePendingGPUTransferMemSize.update(_transferSize, 0);
}*/

#if 0
#include "VKTexture.h"

#include <NumericalConstants.h>

#include "VKTextureTransfer.h"
#include "VKBackend.h"

using namespace gpu;
using namespace gpu::gl;

std::shared_ptr<VKTextureTransferHelper> VKTexture::_textureTransferHelper;
static std::map<uint16, size_t> _textureCountByMips;
static uint16 _currentMaxMipCount { 0 };

// FIXME placeholder for texture memory over-use
#define DEFAULT_MAX_MEMORY_MB 256

const VKenum VKTexture::CUBE_FACE_LAYOUT[6] = {
    VK_TEXTURE_CUBE_MAP_POSITIVE_X, VK_TEXTURE_CUBE_MAP_NEGATIVE_X,
    VK_TEXTURE_CUBE_MAP_POSITIVE_Y, VK_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    VK_TEXTURE_CUBE_MAP_POSITIVE_Z, VK_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

const VKenum VKTexture::WRAP_MODES[Sampler::NUM_WRAP_MODES] = {
    VK_REPEAT,                         // WRAP_REPEAT,
    VK_MIRRORED_REPEAT,                // WRAP_MIRROR,
    VK_CLAMP_TO_EDGE,                  // WRAP_CLAMP,
    VK_CLAMP_TO_BORDER,                // WRAP_BORDER,
    VK_MIRROR_CLAMP_TO_EDGE_EXT        // WRAP_MIRROR_ONCE,
};     

const VKFilterMode VKTexture::FILTER_MODES[Sampler::NUM_FILTERS] = {
    { VK_NEAREST, VK_NEAREST },  //FILTER_MIN_MAG_POINT,
    { VK_NEAREST, VK_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR,
    { VK_LINEAR, VK_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT,
    { VK_LINEAR, VK_LINEAR },  //FILTER_MIN_MAG_LINEAR,

    { VK_NEAREST_MIPMAP_NEAREST, VK_NEAREST },  //FILTER_MIN_MAG_MIP_POINT,
    { VK_NEAREST_MIPMAP_LINEAR, VK_NEAREST },  //FILTER_MIN_MAG_POINT_MIP_LINEAR,
    { VK_NEAREST_MIPMAP_NEAREST, VK_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    { VK_NEAREST_MIPMAP_LINEAR, VK_LINEAR },  //FILTER_MIN_POINT_MAG_MIP_LINEAR,
    { VK_LINEAR_MIPMAP_NEAREST, VK_NEAREST },  //FILTER_MIN_LINEAR_MAG_MIP_POINT,
    { VK_LINEAR_MIPMAP_LINEAR, VK_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    { VK_LINEAR_MIPMAP_NEAREST, VK_LINEAR },  //FILTER_MIN_MAG_LINEAR_MIP_POINT,
    { VK_LINEAR_MIPMAP_LINEAR, VK_LINEAR },  //FILTER_MIN_MAG_MIP_LINEAR,
    { VK_LINEAR_MIPMAP_LINEAR, VK_LINEAR }  //FILTER_ANISOTROPIC,
};

VKenum VKTexture::getGLTextureType(const Texture& texture) {
    switch (texture.getType()) {
    case Texture::TEX_2D:
        return VK_TEXTURE_2D;
        break;

    case Texture::TEX_CUBE:
        return VK_TEXTURE_CUBE_MAP;
        break;

    default:
        qFatal("Unsupported texture type");
    }
    Q_UNREACHABLE();
    return VK_TEXTURE_2D;
}


const std::vector<VKenum>& VKTexture::getFaceTargets(VKenum target) {
    static std::vector<VKenum> cubeFaceTargets {
        VK_TEXTURE_CUBE_MAP_POSITIVE_X, VK_TEXTURE_CUBE_MAP_NEGATIVE_X,
        VK_TEXTURE_CUBE_MAP_POSITIVE_Y, VK_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        VK_TEXTURE_CUBE_MAP_POSITIVE_Z, VK_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    static std::vector<VKenum> faceTargets {
        VK_TEXTURE_2D
    };
    switch (target) {
    case VK_TEXTURE_2D:
        return faceTargets;
    case VK_TEXTURE_CUBE_MAP:
        return cubeFaceTargets;
    default:
        Q_UNREACHABLE();
        break;
    }
    Q_UNREACHABLE();
    return faceTargets;
}

float VKTexture::getMemoryPressure() {
    // Check for an explicit memory limit
    auto availableTextureMemory = Texture::getAllowedGPUMemoryUsage();

    // If no memory limit has been set, use a percentage of the total dedicated memory
    if (!availableTextureMemory) {
        auto totalGpuMemory = gpu::vk::getDedicatedMemory();

        // If no limit has been explicitly set, and the dedicated memory can't be determined, 
        // just use a fallback fixed value of 256 MB
        if (!totalGpuMemory) {
            totalGpuMemory = MB_TO_BYTES(DEFAULT_MAX_MEMORY_MB);
        }

        // Allow 75% of all available GPU memory to be consumed by textures
        // FIXME overly conservative?
        availableTextureMemory = (totalGpuMemory >> 2) * 3;
    }

    // Return the consumed texture memory divided by the available texture memory.
    auto consumedGpuMemory = Context::getTextureGPUMemoryUsage();
    return (float)consumedGpuMemory / (float)availableTextureMemory;
}

VKTexture::DownsampleSource::DownsampleSource(const std::weak_ptr<vk::VKBackend>& backend, VKTexture* oldTexture) :
    _backend(backend),
    _size(oldTexture ? oldTexture->_size : 0),
    _texture(oldTexture ? oldTexture->takeOwnership() : 0),
    _minMip(oldTexture ? oldTexture->_minMip : 0),
    _maxMip(oldTexture ? oldTexture->_maxMip : 0)
{
}

VKTexture::DownsampleSource::~DownsampleSource() {
    if (_texture) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseTexture(_texture, _size);
        }
    }
}

VKTexture::VKTexture(const std::weak_ptr<VKBackend>& backend, const gpu::Texture& texture, VKuint id, VKTexture* originalTexture, bool transferrable) :
    VKObject(backend, texture, id),
    _storageStamp(texture.getStamp()),
    _target(getGLTextureType(texture)),
    _maxMip(texture.maxMip()),
    _minMip(texture.minMip()),
    _virtualSize(texture.evalTotalSize()),
    _transferrable(transferrable),
    _downsampleSource(backend, originalTexture)
{
    if (_transferrable) {
        uint16 mipCount = usedMipLevels();
        _currentMaxMipCount = std::max(_currentMaxMipCount, mipCount);
        if (!_textureCountByMips.count(mipCount)) {
            _textureCountByMips[mipCount] = 1;
        } else {
            ++_textureCountByMips[mipCount];
        }
    } 
    Backend::incrementTextureGPUCount();
    Backend::updateTextureGPUVirtualMemoryUsage(0, _virtualSize);
}


// Create the texture and allocate storage
VKTexture::VKTexture(const std::weak_ptr<vk::VKBackend>& backend, const Texture& texture, VKuint id, bool transferrable) :
    VKTexture(backend, texture, id, nullptr, transferrable)
{
    // FIXME, do during allocation
    //Backend::updateTextureGPUMemoryUsage(0, _size);
    Backend::setGPUObject(texture, this);
}

// Create the texture and copy from the original higher resolution version
VKTexture::VKTexture(const std::weak_ptr<vk::VKBackend>& backend, const gpu::Texture& texture, VKuint id, VKTexture* originalTexture) :
    VKTexture(backend, texture, id, originalTexture, originalTexture->_transferrable)
{
    Q_ASSERT(_minMip >= originalTexture->_minMip);
    // Set the GPU object last because that implicitly destroys the originalTexture object
    Backend::setGPUObject(texture, this);
}

VKTexture::~VKTexture() {
    if (_transferrable) {
        uint16 mipCount = usedMipLevels();
        Q_ASSERT(_textureCountByMips.count(mipCount));
        auto& numTexturesForMipCount = _textureCountByMips[mipCount];
        --numTexturesForMipCount;
        if (0 == numTexturesForMipCount) {
            _textureCountByMips.erase(mipCount);
            if (mipCount == _currentMaxMipCount) {
                _currentMaxMipCount = (_textureCountByMips.empty() ? 0 : _textureCountByMips.rbegin()->first);
            }
        }
    }

    if (_id) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseTexture(_id, _size);
        }
    }
    Backend::updateTextureGPUVirtualMemoryUsage(_virtualSize, 0);
}

void VKTexture::createTexture() {
    withPreservedTexture([&] {
        allocateStorage();
        (void)CHECK_VK_ERROR();
        syncSampler();
        (void)CHECK_VK_ERROR();
    });
}

void VKTexture::setSize(VKuint size) const {
    Backend::updateTextureGPUMemoryUsage(_size, size);
    const_cast<VKuint&>(_size) = size;
}

bool VKTexture::isInvalid() const {
    return _storageStamp < _gpuObject.getStamp();
}

bool VKTexture::isOutdated() const {
    return VKSyncState::Idle == _syncState && _contentStamp < _gpuObject.getDataStamp();
}

bool VKTexture::isOverMaxMemory() const {
    // FIXME switch to using the max mip count used from the previous frame
    if (usedMipLevels() < _currentMaxMipCount) {
        return false;
    }
    Q_ASSERT(usedMipLevels() == _currentMaxMipCount);

    if (getMemoryPressure() < 1.0f) {
        return false;
    }

    return true;
}

bool VKTexture::isReady() const {
    // If we have an invalid texture, we're never ready
    if (isInvalid()) {
        return false;
    }

    // If we're out of date, but the transfer is in progress, report ready
    // as a special case
    auto syncState = _syncState.load();

    if (isOutdated()) {
        return Idle != syncState;
    }

    if (Idle != syncState) {
        return false;
    }

    return true;
}


// Do any post-transfer operations that might be required on the main context / rendering thread
void VKTexture::postTransfer() {
    setSyncState(VKSyncState::Idle);
    ++_transferCount;

    //// The public gltexture becaomes available
    //_id = _privateTexture;

    _downsampleSource.reset();

    // At this point the mip pixels have been loaded, we can notify the gpu texture to abandon it's memory
    switch (_gpuObject.getType()) {
        case Texture::TEX_2D:
            for (uint16_t i = 0; i < Sampler::MAX_MIP_LEVEL; ++i) {
                if (_gpuObject.isStoredMipFaceAvailable(i)) {
                    _gpuObject.notifyMipFaceGPULoaded(i);
                }
            }
            break;

        case Texture::TEX_CUBE:
            // transfer pixels from each faces
            for (uint8_t f = 0; f < CUBE_NUM_FACES; f++) {
                for (uint16_t i = 0; i < Sampler::MAX_MIP_LEVEL; ++i) {
                    if (_gpuObject.isStoredMipFaceAvailable(i, f)) {
                        _gpuObject.notifyMipFaceGPULoaded(i, f);
                    }
                    }
                }
            break;

        default:
            qCWarning(gpu_vk_logging) << __FUNCTION__ << " case for Texture Type " << _gpuObject.getType() << " not supported";
            break;
    }
}

void VKTexture::initTextureTransferHelper() {
    _textureTransferHelper = std::make_shared<VKTextureTransferHelper>();
}
#endif