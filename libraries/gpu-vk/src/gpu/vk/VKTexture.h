//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKTexture_h
#define hifi_gpu_vk_VKTexture_h

#include "VKShared.h"
#include "VKTextureTransfer.h"
#include "VKBackend.h"

namespace gpu { namespace vk {

struct VKFilterMode {
    vk::Filter minFilter;
    vk::Filter magFilter;
};

class VKTexture : public VKObject<Texture, vk::Image> {
public:
    static void initTextureTransferHelper();
    static std::shared_ptr<VKTextureTransferHelper> _textureTransferHelper;

    template <typename VKTextureType>
    static VKTextureType* sync(VKBackend& backend, const TexturePointer& texturePointer, bool needTransfer) {
        const Texture& texture = *texturePointer;
        if (!texture.isDefined()) {
            // NO texture definition yet so let's avoid thinking
            return nullptr;
        }

        // If the object hasn't been created, or the object definition is out of date, drop and re-create
        VKTextureType* object = Backend::getGPUObject<VKTextureType>(texture);

        // Create the texture if need be (force re-creation if the storage stamp changes
        // for easier use of immutable storage)
        if (!object || object->isInvalid()) {
            // This automatically any previous texture
            object = new VKTextureType(backend.shared_from_this(), texture, needTransfer);
            if (!object->_transferrable) {
                object->createTexture();
                object->_contentStamp = texture.getDataStamp();
                object->postTransfer();
            }
        }

        // Object maybe doens't neet to be tranasferred after creation
        if (!object->_transferrable) {
            return object;
        }

        // If we just did a transfer, return the object after doing post-transfer work
        if (VKSyncState::Transferred == object->getSyncState()) {
            object->postTransfer();
            return object;
        }

        if (object->isReady()) {
            // Do we need to reduce texture memory usage?
            if (object->isOverMaxMemory() && texturePointer->incremementMinMip()) {
                // WARNING, this code path will essentially `delete this`, 
                // so no dereferencing of this instance should be done past this point
                object = new VKTextureType(backend.shared_from_this(), texture, object);
                _textureTransferHelper->transferTexture(texturePointer);
            }
        } else if (object->isOutdated()) {
            // Object might be outdated, if so, start the transfer
            // (outdated objects that are already in transfer will have reported 'true' for ready()
            _textureTransferHelper->transferTexture(texturePointer);
        }

        return object;
    }

    template <typename VKTextureType> 
    static vk::Image getId(VKBackend& backend, const TexturePointer& texture, bool shouldSync) {
        if (!texture) {
            return 0;
        }
        VKTextureType* object { nullptr };
        if (shouldSync) {
            object = sync<VKTextureType>(backend, texture, shouldSync);
        } else {
            object = Backend::getGPUObject<VKTextureType>(*texture);
        }
        if (!object) {
            return 0;
        }

        VKuint result = object->_id;

        // Don't return textures that are in transfer state
        if (shouldSync) {
            if ((object->getSyncState() != VKSyncState::Idle) ||
                // Don't return transferrable textures that have never completed transfer
                (!object->_transferrable || 0 != object->_transferCount)) {
                // Will be either 0 or the original texture being downsampled.
                result = object->_downsampleSource._texture;
            }
        }
        
        return result;
    }

    // Used by derived classes and helpers to ensure the actual VK object exceeds the lifetime of `this`
    vk::Image takeOwnership() {
        vk::Image result = _id;
        const_cast<vk::Image&>(_id) = 0;
        return result;
    }

    ~VKTexture();

    const Stamp _storageStamp;
    const vk::ImageType _target;
    const uint16 _maxMip;
    const uint16 _minMip;
    const Size _virtualSize; // theoretical size as expected
    Stamp _contentStamp { 0 };
    const bool _transferrable;
    Size _transferCount { 0 };

    struct DownsampleSource {
        using Pointer = std::shared_ptr<DownsampleSource>;
        DownsampleSource(const std::weak_ptr<vk::VKBackend>& backend) : _backend(backend), _size(0), _texture(0), _minMip(0), _maxMip(0) {}
        DownsampleSource(const std::weak_ptr<vk::VKBackend>& backend, VKTexture* originalTexture);
        ~DownsampleSource();
        void reset() const { const_cast<VKuint&>(_texture) = 0; }
        const std::weak_ptr<vk::VKBackend>& _backend;
        const Size _size { 0 };
        const vk::Image _texture { 0 };
        const uint16 _minMip { 0 };
        const uint16 _maxMip { 0 };
    } _downsampleSource;

    VKuint size() const { return _size; }
    VKSyncState getSyncState() const { return _syncState; }

    // Is the storage out of date relative to the gpu texture?
    bool isInvalid() const;

    // Is the content out of date relative to the gpu texture?
    bool isOutdated() const;

    // Is the texture in a state where it can be rendered with no work?
    bool isReady() const;

    // Execute any post-move operations that must occur only on the main thread
    void postTransfer();

    bool isOverMaxMemory() const;

protected:
    static const size_t CUBE_NUM_FACES = 6;
    static const VKenum CUBE_FACE_LAYOUT[6];
    static const VKFilterMode FILTER_MODES[Sampler::NUM_FILTERS];
    static const VKenum WRAP_MODES[Sampler::NUM_WRAP_MODES];

    static const std::vector<VKenum>& getFaceTargets(VKenum textureType);

    static vk::ImageType getGLTextureType(const Texture& texture);
    // Return a floating point value indicating how much of the allowed 
    // texture memory we are currently consuming.  A value of 0 indicates 
    // no texture memory usage, while a value of 1 indicates all available / allowed memory
    // is consumed.  A value above 1 indicates that there is a problem.
    static float getMemoryPressure();


    const VKuint _size { 0 }; // true size as reported by the gl api
    std::atomic<VKSyncState> _syncState { VKSyncState::Idle };

    VKTexture(const std::weak_ptr<vk::VKBackend>& backend, const Texture& texture, VKuint id, bool transferrable);
    VKTexture(const std::weak_ptr<vk::VKBackend>& backend, const Texture& texture, VKuint id, VKTexture* originalTexture);

    void setSyncState(VKSyncState syncState) { _syncState = syncState; }
    uint16 usedMipLevels() const { return (_maxMip - _minMip) + 1; }

    void createTexture();
    
    virtual void allocateStorage() const = 0;
    virtual void updateSize() const = 0;
    virtual void transfer() const = 0;
    virtual void syncSampler() const = 0;
    virtual void generateMips() const = 0;
    virtual void withPreservedTexture(std::function<void()> f) const = 0;

protected:
    void setSize(VKuint size) const;

private:

    VKTexture(const std::weak_ptr<VKBackend>& backend, const gpu::Texture& gpuTexture, VKuint id, VKTexture* originalTexture, bool transferrable);

    friend class VKTextureTransferHelper;
    friend class VKBackend;
};

} }

#endif
