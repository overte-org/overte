//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
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