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

#include <gl/Config.h>
#include <gl/GLHelpers.h>

#include "VKBackend.h"


namespace gpu { namespace vk {

struct VKFilterMode {
    VkFilter minFilter;
    VkFilter magFilter;
};

class VKTexture : public VKObject<Texture> {
public:
    // VKTODO
    //static void initTextureTransferHelper();
    //static std::shared_ptr<VKTextureTransferHelper> _textureTransferHelper;

    /*template <typename VKTextureType>
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
            if (!object->_transferable) {
                object->createTexture(backend);
                object->_contentStamp = texture.getDataStamp();
                object->postTransfer(backend);
            }
        }

        // Object maybe doesn't need to be transferred after creation
        if (!object->_transferable) {
            return object;
        }

        // If we just did a transfer, return the object after doing post-transfer work
        if (VKSyncState::Transferred == object->getSyncState()) {
            object->postTransfer(backend);
            return object;
        }

        if (object->isReady()) {
            // Do we need to reduce texture memory usage?
            if (object->isOverMaxMemory() && texturePointer->incremementMinMip()) {
                // WARNING, this code path will essentially `delete this`, 
                // so no dereferencing of this instance should be done past this point
                object = new VKTextureType(backend.shared_from_this(), texture, object);
                // VKTODO
                // _textureTransferHelper->transferTexture(texturePointer);
            }
        } else if (object->isOutdated()) {
            // Object might be outdated, if so, start the transfer
            // (outdated objects that are already in transfer will have reported 'true' for ready()
            // VKTODO
            // _textureTransferHelper->transferTexture(texturePointer);
        }

        return object;
    }*/

    /*template <typename VKTextureType>
    static VkDescriptorImageInfo getDescriptorImageInfoFromTexture(VKBackend& backend, const TexturePointer& texture, bool shouldSync) {
        if (!texture) {
            Q_ASSERT(false);
            return backend.getDefaultTextureDescriptorInfo();
        }
        VKTextureType* object { nullptr };
        if (shouldSync) {
            object = sync<VKTextureType>(backend, texture, shouldSync);
        } else {
            object = Backend::getGPUObject<VKTextureType>(*texture);
        }
        if (!object) {
            Q_ASSERT(false);
            return backend.getDefaultTextureDescriptorInfo();
        }

        VkDescriptorImageInfo result {};
        //= object->_;

        // VKTODO
        // Don't return textures that are in transfer state
        /*if (shouldSync) {
            if ((object->getSyncState() != VKSyncState::Idle) ||
                // Don't return transferable textures that have never completed transfer
                (!object->_transferable || 0 != object->_transferCount)) {
                // Will be either 0 or the original texture being downsampled.
                Q_ASSERT(false);
                result = object->_downsampleSource._texture;
            }
        }*
        
        return object->getDescriptorImageInfo();
    }*/

    // Used by derived classes and helpers to ensure the actual VK object exceeds the lifetime of `this`
    /*VkImage takeOwnership() {
        VkImage result = _id;
        const_cast<VkImage&>(_id) = 0;
        return result;
    }*/

    virtual ~VKTexture();

    VkImage _vkImage{ VK_NULL_HANDLE };
    const Stamp _storageStamp;
    const VkImageViewType _target;
    //const uint16 _maxMip;
    //const uint16 _minMip;
    //const Size _virtualSize; // theoretical size as expected
    Stamp _contentStamp { 0 };
    const bool _transferable;
    Size _transferCount { 0 };

    struct DownsampleSource {
        using Pointer = std::shared_ptr<DownsampleSource>;
        DownsampleSource(const std::weak_ptr<VKBackend>& backend) : _backend(backend), _size(0), _texture(VK_NULL_HANDLE), _minMip(0), _maxMip(0) {}
        DownsampleSource(const std::weak_ptr<VKBackend>& backend, VKTexture* originalTexture)  : _backend(backend), _size(0), _texture(0), _minMip(0), _maxMip(0) { Q_ASSERT(false); }; // VKTODO
        ~DownsampleSource() {}; // VKTODO
        void reset() { _texture = VK_NULL_HANDLE; }
        const std::weak_ptr<vk::VKBackend>& _backend;
        const Size _size { 0 };
        // TODO: Shouldn't that be texture instead?
        VkImage _texture { VK_NULL_HANDLE };
        const uint16 _minMip { 0 };
        const uint16 _maxMip { 0 };
    } _downsampleSource;

    virtual size_t size() const { return _size; }
    VKSyncState getSyncState() const { return _syncState; }

    // Is the storage out of date relative to the gpu texture?
    // VKTODO
    bool isInvalid() const { return false; };

    // Is the content out of date relative to the gpu texture?
    // VKTODO
    bool isOutdated() const { return false; };

    // Is the texture in a state where it can be rendered with no work?
    bool isReady() const {
        // VKTODO: Is this right?
        return _syncState == VKSyncState::Transferred;
    };

    // Execute any post-move operations that must occur only on the main thread
    virtual void postTransfer(VKBackend &backend) = 0;

    // VKTODO: can be done later
    bool isOverMaxMemory() const { return false; };

protected:
    static const size_t CUBE_NUM_FACES = 6;
    // VKTODO
    //static const uint8_t TEXTURE_CUBE_NUM_FACES = 6;
    //static const uint32_t CUBE_FACE_LAYOUT[TEXTURE_CUBE_NUM_FACES];
    static const VKFilterMode FILTER_MODES[Sampler::NUM_FILTERS];
    static const uint32_t WRAP_MODES[Sampler::NUM_WRAP_MODES];

    //static const std::vector<VKenum>& getFaceTargets(VKenum textureType);

    static VkImageViewType getVKTextureType(const Texture& texture);
    // Return a floating point value indicating how much of the allowed 
    // texture memory we are currently consuming.  A value of 0 indicates 
    // no texture memory usage, while a value of 1 indicates all available / allowed memory
    // is consumed.  A value above 1 indicates that there is a problem.
    // VKTODO: this can wait for now
    static float getMemoryPressure() { return 0; };


    const size_t _size { 0 }; // true size as reported by the Vulkan api
    std::atomic<VKSyncState> _syncState { VKSyncState::Idle };
    VkFormat _vkTexelFormat { VK_FORMAT_UNDEFINED };

    VKTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture, bool isTransferable);
    //VKTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture, VkImage image, bool transferrable);
    //VKTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture, VkImage image, VKTexture* originalTexture);

    void setSyncState(VKSyncState syncState) { _syncState = syncState; }
    //uint16 usedMipLevels() const { return (_maxMip - _minMip) + 1; }

    virtual void createTexture(VKBackend &backend) = 0;
    virtual VkDescriptorImageInfo getDescriptorImageInfo() = 0;

    //virtual void allocateStorage() const = 0;
    //virtual void updateSize() const = 0;
    struct TransferData {
        uint16_t mipLevels;
        uint16_t width;
        uint16_t height;
        size_t buffer_size;
        struct Mip {
            size_t offset;
            size_t size;
            uint32_t width;
            uint32_t height;
            bool needsAddingAlpha;
            bool needsBGRToRGB;
            std::shared_ptr<const storage::Storage> data;
        };
        std::vector<std::vector<Mip>> mips;
    };
    TransferData _transferData{};
    virtual void transfer(VKBackend &backend) = 0;
    //virtual void syncSampler() const = 0;
    // VKTODO
    //virtual void generateMips() const = 0;
    // VKTODO what does this mean?
    //virtual void withPreservedTexture(std::function<void()> f) const = 0;

protected:
    void setSize(size_t size) const;

private:

    VKTexture(const std::weak_ptr<VKBackend>& backend, const gpu::Texture& gpuTexture, VkImage image, VKTexture* originalTexture, bool transferrable);

    friend class VKTextureTransferHelper;
    friend class VKBackend;
};

class VKFixedAllocationTexture : public VKTexture {
    using Parent = VKTexture;
    friend class VKBackend;

public:
    VKFixedAllocationTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture, bool isTransferable) :
        VKTexture(backend, texture, isTransferable) {}; // VKTODO
    virtual ~VKFixedAllocationTexture() {};

protected:
    Size size() const override { return _size; }

    //void allocateStorage();
    // VKTODO
    //void syncSampler() const override;
    // VKTODO
    //void updateSize() const override {};
    VmaAllocation _vmaAllocation;
    const Size _size{ 0 };
};

class VKAttachmentTexture : public VKFixedAllocationTexture {
    friend class VKBackend;

public:
    VkImageLayout getVkImageLayout() { return _vkImageLayout; };

protected:
    VKAttachmentTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture) :
        VKFixedAllocationTexture(backend, texture, false) {
            VKAttachmentTexture::createTexture(*backend.lock());
        };
    virtual ~VKAttachmentTexture(); // VKTODO: delete image and image view, release memory
    void createTexture(VKBackend &backend) override;
    void transfer(VKBackend &backend) override {}; // VKTODO
    void postTransfer(VKBackend &backend) override {}; // VKTODO

    VkDescriptorImageInfo getDescriptorImageInfo() override; // VKTODO

    VkImageView _vkImageView { VK_NULL_HANDLE };
    VkImageLayout _vkImageLayout {}; // VKTODO: this needs to be updated on blits and other image writes
    VkSampler _vkSampler { VK_NULL_HANDLE };
    //VkImage _vkImage { VK_NULL_HANDLE };
    //VkDeviceMemory _vkDeviceMemory{ VK_NULL_HANDLE };
};

class VKStrictResourceTexture: public VKFixedAllocationTexture {
    friend class VKBackend;

protected:
    // VKTODO: how to handle mipmaps?
    VKStrictResourceTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture) :
        VKFixedAllocationTexture(backend, texture, true) {
        VKBackend& vkBackend = *backend.lock();
        VKStrictResourceTexture::createTexture(vkBackend);
        // VKTODO: transfer on transfer tread
        VKStrictResourceTexture::transfer(vkBackend);
        VKStrictResourceTexture::postTransfer(vkBackend);
    };
    ~VKStrictResourceTexture() override; // VKTODO: delete image and image view, release memory
    void createTexture(VKBackend &backend) override;
    void transfer(VKBackend &backend) override;
    void postTransfer(VKBackend &backend) override;
    VkDescriptorImageInfo getDescriptorImageInfo() override;
    //VkImage _vkImage { VK_NULL_HANDLE };
    VkImageView _vkImageView { VK_NULL_HANDLE };
    VkImageLayout _vkImageLayout {};
    VkSampler _vkSampler { VK_NULL_HANDLE };
    // This need to be moved to VKFixedAllocationTexture and allocated in allocateStorage()
    //VkDeviceMemory _vkDeviceMemory{ VK_NULL_HANDLE };
};

class VKExternalTexture: public VKTexture {
    friend class VKBackend;

public:
    VKExternalTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture) :
            VKTexture(backend, texture, false) {
            VKBackend& vkBackend = *backend.lock();
            VKExternalTexture::createTexture(vkBackend);
            // VKTODO: maybe this needs to be done on OpenGL thread?
            VKExternalTexture::initGL(vkBackend);
            //VKExternalTexture::transferGL(vkBackend);
            VKExternalTexture::postTransfer(vkBackend);
        }; // VKTODO
    ~VKExternalTexture() override; // VKTODO: add proper cleanup for both Vulkan and OpenGL
    void setSource(GLuint source) { _openGLSourceId = source; };
    void transferGL(VKBackend &backend);

protected:
    Size size() const override { return _size; }

    //VmaAllocation _vmaAllocation;
    VkDeviceMemory _sharedMemory;
    size_t _sharedMemorySize;
    const Size _size{ 0 }; // VKTODO: how is this used?

    void createTexture(VKBackend &backend) override;
    void initGL(VKBackend &backend);
    void transfer(VKBackend &backend) override {};
    void postTransfer(VKBackend &backend) override;
    VkDescriptorImageInfo getDescriptorImageInfo() override;
    VkImageView _vkImageView { VK_NULL_HANDLE };
    VkImageLayout _vkImageLayout {};
    VkSampler _vkSampler { VK_NULL_HANDLE };

    // Shared texture properties
#ifdef WIN32
    HANDLE _sharedHandle { nullptr };  // Windows handle for shared texture memory.
#else
    int _sharedFd { -1 }; // File descriptor is used for sharing memory between Vulkan and OpenGL on Linux.
#endif
    GLuint _openGLMemoryObject = 0;
    GLuint _openGLId = 0;
    GLuint _openGLSourceId = 0;
};

} }

#endif
