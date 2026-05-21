//
// Created by ksuprynowicz on 18.05.2026.
//
//
//  Created by Bradley Austin Davis on 2016/05/15
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "VKTexture.h"

#include <QObject>
#include <QtCore/QThread>
#include <NumericalConstants.h>
#include <ThreadHelpers.h>

#include "VKBackend.h"
#include "VKTextureTransfer.h"
#include "VKShared.h"

#define OVERSUBSCRIBED_PRESSURE_VALUE 0.95f
#define UNDERSUBSCRIBED_PRESSURE_VALUE 0.85f
#define DEFAULT_ALLOWED_TEXTURE_MEMORY_MB ((size_t)2048)
#define MAX_RESOURCE_TEXTURES_PER_FRAME 2
#define NO_BUFFER_WORK_SLEEP_TIME_MS 2
#define THREADED_TEXTURE_BUFFERING 1
#define MAX_AUTO_FRACTION_OF_TOTAL_MEMORY 0.8f
#define AUTO_RESERVE_TEXTURE_MEMORY MB_TO_BYTES(64)

static constexpr size_t DEFAULT_ALLOWED_TEXTURE_MEMORY = MB_TO_BYTES(DEFAULT_ALLOWED_TEXTURE_MEMORY_MB);

static constexpr size_t MAX_PIXEL_BYTE_SIZE{ 4 };
static constexpr size_t MAX_TRANSFER_DIMENSION{ 1024 };



namespace gpu { namespace vk {

enum class MemoryPressureState
{
    Idle,
    Transfer,
    Undersubscribed,
};

static MemoryPressureState _memoryPressureState{ MemoryPressureState::Idle };

using ImmediateQueuePair = std::pair<TexturePointer, float>;
// Contains a priority sorted list of textures on which work is to be done in the current frame
using ImmediateWorkQueue = std::priority_queue<ImmediateQueuePair, std::vector<ImmediateQueuePair>, LessPairSecond<ImmediateQueuePair>>;

}}  // namespace gpu::gl

using namespace gpu;
using namespace gpu::vk;

void VKBackend::initTextureManagementStage() {
    _textureManagement._transferEngine = std::make_shared<VKTextureTransferEngine>();
}

void VKBackend::killTextureManagementStage() {
    _textureManagement._transferEngine->shutdown();
    _textureManagement._transferEngine.reset();
}

void VKTextureTransferEngine::TextureBufferThread::run() {
    while (!_parent._shutdown) {
        if (!_parent.processActiveBufferQueue()) {
            QThread::msleep(NO_BUFFER_WORK_SLEEP_TIME_MS);
        }
    }
}

std::vector<TexturePointer> VKTextureTransferEngine::getAllTextures() {
    auto expiredBegin = std::remove_if(_registeredTextures.begin(), _registeredTextures.end(), [&](const std::weak_ptr<Texture>& weak) -> bool {
        return weak.expired();
    });
    _registeredTextures.erase(expiredBegin, _registeredTextures.end());

    std::vector<TexturePointer> result;
    result.reserve(_registeredTextures.size());
    for (const auto& weak : _registeredTextures) {
        auto strong = weak.lock();
        if (!strong) {
            continue;
        }
        result.push_back(strong);
    }
    return result;
}

void VKTextureTransferEngine::addMemoryManagedTexture(const TexturePointer& texturePointer) {
    ++_frameTexturesCreated;
    _registeredTextures.push_back(texturePointer);
}

void VKTextureTransferEngine::shutdown() {
    _shutdown = true;
#if THREADED_TEXTURE_BUFFERING
    if (_transferThread) {
        _transferThread->wait();
        delete _transferThread;
        _transferThread = nullptr;
    }
#endif
}

void VKTextureTransferEngine::manageMemory() {
    PROFILE_RANGE(gpu_vk, __FUNCTION__);
    // reset the count used to limit the number of textures created per frame
    resetFrameTextureCreated();
    // Determine the current memory management state.  It will be either idle (no work to do),
    // undersubscribed (need to do more allocation) or transfer (need to upload content from the
    // backing store to the GPU
    updateMemoryPressure();
    if (MemoryPressureState::Undersubscribed == _memoryPressureState) {
        // If we're undersubscribed, we need to process some of the textures that can have additional allocation
        processPromotes();
    } else if (MemoryPressureState::Transfer == _memoryPressureState) {
        // If we're in transfer mode we need to manage the buffering and upload queues
        processTransferQueues();
    }
}

// Each frame we will check if our memory pressure state has changed.
void VKTextureTransferEngine::updateMemoryPressure() {
    PROFILE_RANGE(gpu_vk, __FUNCTION__);

    bool useAvailableGlMemory = false;
    size_t allowedMemoryAllocation = gpu::Texture::getAllowedGPUMemoryUsage();

    if (0 == allowedMemoryAllocation) {
        // Automatic allocation

        if (VKBackend::availableMemoryKnown()) {
            // If we know how much is free, then we use that
            useAvailableGlMemory = true;
        } else {
            // We don't know how much is free, so leave some reasonable spare room
            // and hope it works.
            allowedMemoryAllocation = VKBackend::getTotalMemory() * MAX_AUTO_FRACTION_OF_TOTAL_MEMORY;

            if (0 == allowedMemoryAllocation) {
                // Last resort, if we failed to detect
                allowedMemoryAllocation = DEFAULT_ALLOWED_TEXTURE_MEMORY;
            }
        }
    }

    // Clear any defunct textures (weak pointers that no longer have a valid texture)
    auto strongTextures = getAllTextures();

    size_t totalVariableMemoryAllocation = 0;
    size_t idealMemoryAllocation = 0;
    bool canDemote = false;
    bool canPromote = false;
    bool hasTransfers = false;
    for (const auto& texture : strongTextures) {
        VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texture);
        VKVariableAllocationSupport* varTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
        varTexture->sanityCheck();

        // Track how much the texture thinks it should be using
        idealMemoryAllocation += texture->evalTotalSize();
        // Track how much we're actually using
        totalVariableMemoryAllocation += vkTexture->size();
        if (!vkTexture->_gpuObject.getImportant() && varTexture->canDemote()) {
            canDemote |= true;
        }
        if (varTexture->canPromote()) {
            canPromote |= true;
        }
        if (varTexture->hasPendingTransfers()) {
            hasTransfers |= true;
        }
    }

    Backend::textureResourceIdealGPUMemSize.set(idealMemoryAllocation);
    size_t unallocated = idealMemoryAllocation - totalVariableMemoryAllocation;
    float pressure = 0;

    if (useAvailableGlMemory) {
        size_t totalMem = VKBackend::getTotalMemory();
        size_t availMem = VKBackend::getAvailableMemory();

        if (availMem >= AUTO_RESERVE_TEXTURE_MEMORY) {
            availMem -= AUTO_RESERVE_TEXTURE_MEMORY;
        } else {
            availMem = 0;
        }

        pressure = ((float)totalMem - (float)availMem) / (float)totalMem;
    } else {
        pressure = (float)totalVariableMemoryAllocation / (float)allowedMemoryAllocation;
    }

    // If we're oversubscribed we need to demote textures IMMEDIATELY
    if (pressure > OVERSUBSCRIBED_PRESSURE_VALUE && canDemote) {
        auto overPressure = pressure - OVERSUBSCRIBED_PRESSURE_VALUE;
        size_t relief = (size_t)(overPressure * totalVariableMemoryAllocation);
        processDemotes(relief, strongTextures);
        return;
    }

    auto newState = MemoryPressureState::Idle;
    if (pressure < UNDERSUBSCRIBED_PRESSURE_VALUE && (unallocated != 0 && canPromote)) {
        newState = MemoryPressureState::Undersubscribed;
    } else if (hasTransfers) {
        newState = MemoryPressureState::Transfer;
    } else {
        Lock lock(_bufferMutex);
        if (!_activeBufferQueue.empty() || !_activeTransferQueue.empty() || !_pendingTransfersMap.empty()) {
            newState = MemoryPressureState::Transfer;
        }
    }

    // If we've changed state then we have to populate the appropriate structure with the work to be done
    if (newState != _memoryPressureState) {
        _memoryPressureState = newState;
        _promoteQueue = WorkQueue();
        _pendingTransfersMap.clear();

        if (MemoryPressureState::Idle == _memoryPressureState) {
            return;
        }

        // For each texture, if it's eligible for work in the current state, put it into the appropriate structure
        for (const auto& texture : strongTextures) {
            VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texture);
            VKVariableAllocationSupport* varVkTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
            if (MemoryPressureState::Undersubscribed == _memoryPressureState && varVkTexture->canPromote()) {
                // Promote smallest first
                _promoteQueue.push({ texture, 1.0f / (float)vkTexture->size() });
            } else if (MemoryPressureState::Transfer == _memoryPressureState && varVkTexture->hasPendingTransfers()) {
                populateTransferQueue(texture);
            }
        }
    }
}

// Manage the _activeBufferQueue and _activeTransferQueue queues
void VKTextureTransferEngine::processTransferQueues() {
#if THREADED_TEXTURE_BUFFERING
    if (!_transferThread) {
        _transferThread = new TextureBufferThread(*this);
        QString name = "TextureBufferThread";
        _transferThread->setObjectName(name);
        QObject::connect(_transferThread, &QThread::started, [name] { setThreadName(name.toStdString()); });
        _transferThread->start();
    }
#endif

    // From the pendingTransferMap, queue jobs into the _activeBufferQueue
    // Doing so will lock the weak texture pointer so that it can't be destroyed
    // while the background thread is working.
    //
    // This will queue jobs until _queuedBufferSize can't be increased without exceeding
    // VKVariableAllocationTexture::MAX_BUFFER_SIZE or there is no more work to be done
    populateActiveBufferQueue();
#if !THREADED_TEXTURE_BUFFERING
    processActiveBufferQueue();
#endif

    // Take any tasks which have completed buffering and process them, uploading the buffered
    // data to the GPU.  Drains the _activeTransferQueue
    {
        ActiveTransferQueue activeTransferQueue;
        {
            Lock lock(_bufferMutex);
            activeTransferQueue.swap(_activeTransferQueue);
        }

        while (!activeTransferQueue.empty()) {
            const auto& activeTransferJob = activeTransferQueue.front();
            const auto& texturePointer = activeTransferJob.first;
            VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texturePointer);
            VKVariableAllocationSupport* varVkTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
            const auto& tranferJob = activeTransferJob.second;
            if (tranferJob->sourceMip() < varVkTexture->populatedMip()) {
                tranferJob->transfer(texturePointer);
            }
            // The pop_front MUST be the last call since all of these variables in scope are
            // references that will be invalid after the pop
            activeTransferQueue.pop_front();
        }
    }

    // If we have no more work in any of the structures, reset the memory state to idle to
    // force reconstruction of the _pendingTransfersMap if necessary
    {
        Lock lock(_bufferMutex);
        if (_activeTransferQueue.empty() && _activeBufferQueue.empty() && _pendingTransfersMap.empty()) {
            _memoryPressureState = MemoryPressureState::Idle;
        }
    }
}

void VKTextureTransferEngine::populateActiveBufferQueue() {
    size_t queuedBufferSize = _queuedBufferSize;
    static const auto& MAX_BUFFER_SIZE = VKVariableAllocationSupport::MAX_BUFFER_SIZE;
    Q_ASSERT(queuedBufferSize <= MAX_BUFFER_SIZE);
    size_t availableBufferSize = MAX_BUFFER_SIZE - queuedBufferSize;

    // Queue up buffering jobs
    ActiveTransferQueue newBufferJobs;
    size_t newTransferSize{ 0 };

    for (auto itr = _pendingTransfersMap.begin(); itr != _pendingTransfersMap.end();) {
        const auto& weakTexture = itr->first;
        const auto texture = weakTexture.lock();

        // Texture no longer exists, remove from the transfer map and move on
        if (!texture) {
            itr = _pendingTransfersMap.erase(itr);
            continue;
        }

        VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texture);
        VKVariableAllocationSupport* varVkTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);

        auto& textureTransferQueue = itr->second;
        // Can't find any pending transfers, so move on
        if (textureTransferQueue.empty()) {
            if (varVkTexture->hasPendingTransfers()) {
                // qWarning(gpugllogging) << "Texture " << gltexture->_id << "(" << texture->source().c_str() << ") has no transfer jobs, but has pending transfers" ;
            }
            itr = _pendingTransfersMap.erase(itr);
            continue;
        }

        const auto& transferJob = textureTransferQueue.front();
        const auto& transferSize = transferJob->size();
        // If there's not enough space for the buffering, then break out of the loop
        if (transferSize > availableBufferSize) {
            break;
        }
        availableBufferSize -= transferSize;
        Q_ASSERT(availableBufferSize <= MAX_BUFFER_SIZE);
        Q_ASSERT(newTransferSize <= MAX_BUFFER_SIZE);
        newTransferSize += transferSize;
        Q_ASSERT(newTransferSize <= MAX_BUFFER_SIZE);
        newBufferJobs.emplace_back(texture, transferJob);
        textureTransferQueue.pop();
        ++itr;
    }

    {
        Lock lock(_bufferMutex);
        _activeBufferQueue.splice(_activeBufferQueue.end(), newBufferJobs);
        Q_ASSERT(_queuedBufferSize <= MAX_BUFFER_SIZE);
        _queuedBufferSize += newTransferSize;
        Q_ASSERT(_queuedBufferSize <= MAX_BUFFER_SIZE);
    }
}

bool VKTextureTransferEngine::processActiveBufferQueue() {
    ActiveTransferQueue activeBufferQueue;
    {
        Lock lock(_bufferMutex);
        _activeBufferQueue.swap(activeBufferQueue);
    }

    if (activeBufferQueue.empty()) {
        return false;
    }

    for (const auto& activeJob : activeBufferQueue) {
        const auto& texture = activeJob.first;
        const auto& transferJob = activeJob.second;
        // Some jobs don't require buffering, but they pass through this queue to ensure that we don't re-order
        // the buffering & transfer operations.  All jobs in the queue must be processed in order.
        if (!transferJob->bufferingRequired()) {
            continue;
        }
        const auto& transferSize = transferJob->size();
        transferJob->buffer(texture);
        Q_ASSERT(_queuedBufferSize >= transferSize);
        _queuedBufferSize -= transferSize;
    }

    {
        Lock lock(_bufferMutex);
        _activeTransferQueue.splice(_activeTransferQueue.end(), activeBufferQueue);
    }

    Texture::KtxStorage::releaseOpenKtxFiles();
    return true;
}

void VKTextureTransferEngine::populateTransferQueue(const TexturePointer& texturePointer) {
    TextureWeakPointer weakTexture = texturePointer;
    VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texturePointer);
    VKVariableAllocationSupport* varVkTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
    TransferJob::Queue pendingTransfers;
    PROFILE_RANGE(gpu_vk, __FUNCTION__);
    varVkTexture->populateTransferQueue(pendingTransfers);
    if (!pendingTransfers.empty()) {
        _pendingTransfersMap[weakTexture] = pendingTransfers;
    }
}

// From the queue of textures to be promoted
void VKTextureTransferEngine::processPromotes() {
    // FIXME use max allocated memory per frame instead of promotion count
    static const size_t MAX_ALLOCATED_BYTES_PER_FRAME = VKVariableAllocationSupport::MAX_BUFFER_SIZE;
    static const size_t MAX_ALLOCATIONS_PER_FRAME = 8;
    size_t allocatedBytes{ 0 };
    size_t allocations{ 0 };

    while (!_promoteQueue.empty()) {
        // Grab the first item off the demote queue
        PROFILE_RANGE(gpu_vk, __FUNCTION__);
        auto entry = _promoteQueue.top();
        _promoteQueue.pop();
        auto texture = entry.first.lock();
        if (!texture) {
            continue;
        }

        VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texture);
        VKVariableAllocationSupport* varTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
        auto originalSize = vkTexture->size();
        varTexture->promote();
        auto allocationDelta = vkTexture->size() - originalSize;
        if (varTexture->canPromote()) {
            // Promote smallest first
            _promoteQueue.push({ texture, 1.0f / (float)vkTexture->size() });
        }
        allocatedBytes += allocationDelta;
        if (++allocations >= MAX_ALLOCATIONS_PER_FRAME) {
            break;
        }
        if (allocatedBytes >= MAX_ALLOCATED_BYTES_PER_FRAME) {
            break;
        }
    }

    // Get the front of the work queue to perform work
    if (_promoteQueue.empty()) {
        // Force rebuild of work queue
        _memoryPressureState = MemoryPressureState::Idle;
    }
}

void VKTextureTransferEngine::processDemotes(size_t reliefRequired, const std::vector<TexturePointer>& strongTextures) {
    // Demote largest first
    ImmediateWorkQueue demoteQueue;
    for (const auto& texture : strongTextures) {
        VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texture);
        VKVariableAllocationSupport* varVkTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
        if (!vkTexture->_gpuObject.getImportant() && varVkTexture->canDemote()) {
            demoteQueue.push({ texture, (float)vkTexture->size() });
        }
    }

    size_t relieved = 0;
    while (!demoteQueue.empty() && relieved < reliefRequired) {
        {
            const auto& target = demoteQueue.top();
            const auto& texture = target.first;
            VKTexture* vkTexture = Backend::getGPUObject<VKTexture>(*texture);
            auto oldSize = vkTexture->size();
            VKVariableAllocationSupport* varVkTexture = dynamic_cast<VKVariableAllocationSupport*>(vkTexture);
            varVkTexture->demote();
            auto newSize = vkTexture->size();
            relieved += (oldSize - newSize);
        }
        demoteQueue.pop();
    }
}

const uvec3 VKVariableAllocationSupport::MAX_TRANSFER_DIMENSIONS{ MAX_TRANSFER_DIMENSION, MAX_TRANSFER_DIMENSION, 1 };
const uvec3 VKVariableAllocationSupport::INITIAL_MIP_TRANSFER_DIMENSIONS{ 64, 64, 1 };
const size_t VKVariableAllocationSupport::MAX_TRANSFER_SIZE = MAX_TRANSFER_DIMENSION * MAX_TRANSFER_DIMENSION * MAX_PIXEL_BYTE_SIZE;
const size_t VKVariableAllocationSupport::MAX_BUFFER_SIZE = MAX_TRANSFER_SIZE;

VKVariableAllocationSupport::VKVariableAllocationSupport() {
    Backend::textureResourceCount.increment();
}

VKVariableAllocationSupport::~VKVariableAllocationSupport() {
    Backend::textureResourceCount.decrement();
    Backend::textureResourceGPUMemSize.update(_size, 0);
    Backend::textureResourcePopulatedGPUMemSize.update(_populatedSize, 0);
}

void VKVariableAllocationSupport::incrementPopulatedSize(Size delta) const {
    _populatedSize += delta;
    // Keep the 2 code paths to be able to debug
    if (_size < _populatedSize) {
        Backend::textureResourcePopulatedGPUMemSize.update(0, delta);
    } else  {
        Backend::textureResourcePopulatedGPUMemSize.update(0, delta);
    }
}

void VKVariableAllocationSupport::decrementPopulatedSize(Size delta) const {
    _populatedSize -= delta;
    // Keep the 2 code paths to be able to debug
    if (_size < _populatedSize) {
        Backend::textureResourcePopulatedGPUMemSize.update(delta, 0);
    } else  {
        Backend::textureResourcePopulatedGPUMemSize.update(delta, 0);
    }
}

void VKVariableAllocationSupport::sanityCheck() const {
    if (_populatedMip < _allocatedMip) {
        qCWarning(gpu_vk_logging) << "Invalid mip levels";
    }
}

// FIXME hack for stats display
QString getTextureMemoryPressureModeStringVK() {
    switch (_memoryPressureState) {
        case MemoryPressureState::Undersubscribed:
            return "Undersubscribed";

        case MemoryPressureState::Transfer:
            return "Transfer";

        case MemoryPressureState::Idle:
            return "Idle";
    }
    Q_UNREACHABLE();
    return "Unknown";
}
