//
//  Created by Dr. Karol Suprynowicz on 18.05.2026.
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include "VKTextureTransfer.h"

#include <QtCore/QThreadPool>
//#include <QtConcurrent>

#include "VKShared.h"
#include "VKBackend.h"
//#include "gpu/gl/GLTexelFormat.h"
#include <thread>

namespace gpu { namespace vk {

template <typename T>
struct LessPairSecond {
    bool operator()(const T& a, const T& b) { return a.second < b.second; }
};


using QueuePair = std::pair<TextureWeakPointer, float>;

// Contains a priority sorted list of textures on which work is to be done over many frames
// Uses a weak pointer to the texture to avoid keeping it in scope if the client stops using it
using WorkQueue = std::priority_queue<QueuePair, std::vector<QueuePair>, LessPairSecond<QueuePair>>;

struct GLFilterMode {
    GLint minFilter;
    GLint magFilter;
};

/**
  A transfer job encapsulates an individual piece of work required to upload texture data to the GPU.
  The work can be broken down into two parts, expressed as lambdas.  The buffering lambda is repsonsible
  for putting the data to be uploaded into a CPU memory buffer.  The transfer lambda is repsonsible for
  uploading the data from the CPU memory buffer to the GPU using OpenGL calls.  Ideally the buffering lambda
  will be executed on a seprate thread from the OpenGL work to ensure that disk IO operations do not block
  OpenGL calls

  Additionally, a TransferJob can encapsulate some kind of post-upload work that changes the state of the
  GLTexture derived object wrapping the actual texture ID, such as changing the _populateMip value once
  a given mip level has been compeltely uploaded
 */
class TransferJob {
public:
    using Pointer = std::shared_ptr<TransferJob>;
    using Queue = std::queue<Pointer>;
    using Lambda = std::function<void(const TexturePointer&)>;
private:
    Texture::PixelsPointer _mipData;
    size_t _transferOffset{ 0 };
    size_t _transferSize{ 0 };
    uint16_t _sourceMip{ 0 };
    bool _bufferingRequired{ true };
    Lambda _transferLambda{ [](const TexturePointer&) {} };
    Lambda _bufferingLambda{ [](const TexturePointer&) {} };
public:
    TransferJob(const TransferJob& other) = delete;
    TransferJob(uint16_t sourceMip, const std::function<void()>& transferLambda);
    TransferJob(const Texture& texture, uint16_t sourceMip, uint16_t targetMip, uint8_t face, uint32_t lines = 0, uint32_t lineOffset = 0);
    ~TransferJob();
    const uint16_t& sourceMip() const { return _sourceMip; }
    const size_t& size() const { return _transferSize; }
    bool bufferingRequired() const { return _bufferingRequired; }
    void buffer(const TexturePointer& texture) { _bufferingLambda(texture); }
    void transfer(const TexturePointer& texture) { _transferLambda(texture); }
};

using TransferJobPointer = std::shared_ptr<TransferJob>;
using TransferQueue = std::queue<TransferJobPointer>;

// A map of weak texture pointers to queues of work to be done to transfer their data from the backing store to the GPU
using TransferMap = std::map<TextureWeakPointer, TransferQueue, std::owner_less<TextureWeakPointer>>;

class VKTextureTransferEngine {
public:
    using Pointer = std::shared_ptr<VKTextureTransferEngine>;

    virtual ~VKTextureTransferEngine() = default;

    /// Called once per frame by the VKBackend to perform any require memory management or transfer work
    /// Will deallocate textures if oversubscribed,
    void manageMemory();
    void shutdown();

    /// Called whenever a client wants to create a new texture.  This allows the transfer engine to
    /// potentially limit the number of GL textures created per frame
    bool allowCreate() const { return _frameTexturesCreated < MAX_RESOURCE_TEXTURES_PER_FRAME; }
    /// Called whenever a client creates a new resource texture that should use managed memory
    /// and incremental transfer
    void addMemoryManagedTexture(const TexturePointer& texturePointer);

protected:
    class TextureBufferThread : public QThread {
    public:
        TextureBufferThread(VKTextureTransferEngine& parent) : _parent(parent) {}

    protected:
        void run() override;

        VKTextureTransferEngine& _parent;
    };

    // Fetch all the currently active textures as strong pointers, while clearing the
    // empty weak pointers out of _registeredTextures
    std::vector<TexturePointer> getAllTextures();
    void resetFrameTextureCreated() { _frameTexturesCreated = 0;  }

    using ActiveTransferJob = std::pair<TexturePointer, TransferJobPointer>;
    using ActiveTransferQueue = std::list<ActiveTransferJob>;

    void populateActiveBufferQueue();
    bool processActiveBufferQueue();
    void processTransferQueues();
    void populateTransferQueue(const TexturePointer& texturePointer);
    //void addToWorkQueue(const TexturePointer& texturePointer);
    void updateMemoryPressure();

    void processDemotes(size_t relief, const std::vector<TexturePointer>& strongTextures);
    void processPromotes();

private:
    static const size_t MAX_RESOURCE_TEXTURES_PER_FRAME{ 2 };
    size_t _frameTexturesCreated{ 0 };
    std::list<TextureWeakPointer> _registeredTextures;

    std::atomic<bool> _shutdown{ false };
    // Contains a priority sorted list of weak texture pointers that have been determined to be eligible for additional allocation
    // While the memory state is 'undersubscribed', items will be removed from this list and processed, allocating additional memory
    // per frame
    WorkQueue _promoteQueue;
    // This queue contains jobs that will buffer data from the texture backing store (ideally a memory mapped KTX file)
    // to a CPU memory buffer.  This queue is populated on the main GPU thread, and drained on a dedicated thread.
    // When an item on the _activeBufferQueue is completed it is put into the _activeTransferQueue
    ActiveTransferQueue _activeBufferQueue;
    // This queue contains jobs that will upload data from a CPU buffer into a GPU.  This queue is populated on the background
    // thread that process the _activeBufferQueue and drained on the main GPU thread
    ActiveTransferQueue _activeTransferQueue;
    // Mutex protecting the _activeTransferQueue & _activeBufferQueue since they are each accessed both from the main GPU thread
    // and the buffering thread
    Mutex _bufferMutex;
    // The buffering thread which drains the _activeBufferQueue and populates the _activeTransferQueue
    TextureBufferThread* _transferThread{ nullptr };
    // The amount of buffering work currently represented by the _activeBufferQueue
    std::atomic<size_t> _queuedBufferSize{ 0 };
    // This contains a map of all textures to queues of pending transfer jobs.  While in the transfer state, this map is used to
    // populate the _activeBufferQueue up to the limit specified in GLVariableAllocationTexture::MAX_BUFFER_SIZE
    TransferMap _pendingTransfersMap;

};


class VKVariableAllocationSupport {
    friend class VKBackend;

public:
    VKVariableAllocationSupport();
    virtual ~VKVariableAllocationSupport() ;
    virtual void populateTransferQueue(TransferQueue& pendingTransfers) = 0;

    void sanityCheck() const;
    uint16 populatedMip() const { return _populatedMip; }
    bool canPromote() const { return _allocatedMip > _minAllocatedMip; }
    bool canDemote() const { return _allocatedMip < _maxAllocatedMip; }
    bool hasPendingTransfers() const { return _populatedMip > _allocatedMip; }

    virtual size_t promote() = 0;
    virtual size_t demote() = 0;

    static const uvec3 MAX_TRANSFER_DIMENSIONS;
    static const uvec3 INITIAL_MIP_TRANSFER_DIMENSIONS;
    static const size_t MAX_TRANSFER_SIZE;
    static const size_t MAX_BUFFER_SIZE;

protected:
    // THe amount of memory currently allocated
    Size _size { 0 };

    // The amount of memory currently populated
    void incrementPopulatedSize(Size delta) const;
    void decrementPopulatedSize(Size delta) const;
    mutable Size _populatedSize { 0 };

    // The allocated mip level, relative to the number of mips in the gpu::Texture object
    // The relationship between a given glMip to the original gpu::Texture mip is always
    // glMip + _allocatedMip
    uint16 _allocatedMip { 0 };
    // The populated mip level, relative to the number of mips in the gpu::Texture object
    // This must always be >= the allocated mip
    uint16 _populatedMip { 0 };
    // The highest (lowest resolution) mip that we will support, relative to the number
    // of mips in the gpu::Texture object
    uint16 _maxAllocatedMip { 0 };
    // The lowest (highest resolution) mip that we will support, relative to the number
    // of mips in the gpu::Texture object
    uint16 _minAllocatedMip { 0 };
};

} }
