//
//  AvatarMixerWorkerPool.h
//  assignment-client/src/avatar
//
//  Created by Brad Hefta-Gaub on 2/14/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AvatarMixerWorkerPool_h
#define hifi_AvatarMixerWorkerPool_h

#include <condition_variable>
#include <mutex>
#include <vector>

#include <QThread>

#include <TBBHelpers.h>
#include <NodeList.h>
#include <shared/QtHelpers.h>

#include "AvatarMixerWorker.h"


class AvatarMixerWorkerPool;

class AvatarMixerWorkerThread : public QThread, public AvatarMixerWorker {
    Q_OBJECT
    using ConstIter = NodeList::const_iterator;
    using Mutex = std::mutex;
    using Lock = std::unique_lock<Mutex>;

public:
    AvatarMixerWorkerThread(AvatarMixerWorkerPool& pool, WorkerSharedData* workerSharedData) :
        AvatarMixerWorker(workerSharedData), _pool(pool) {};

    void run() override final;

private:
    friend class AvatarMixerWorkerPool;

    void wait();
    void notify(bool stopping);
    bool try_pop(SharedNodePointer& node);

    AvatarMixerWorkerPool& _pool;
    void (AvatarMixerWorker::*_function)(const SharedNodePointer& node) { nullptr };
    bool _stop { false };
};

// Worker pool for avatar mixers
//   AvatarMixerWorkerPool is not thread-safe! It should be instantiated and used from a single thread.
class AvatarMixerWorkerPool {
    using Queue = tbb::concurrent_queue<SharedNodePointer>;
    using Mutex = std::mutex;
    using Lock = std::unique_lock<Mutex>;
    using ConditionVariable = std::condition_variable;

public:
    using ConstIter = NodeList::const_iterator;

    AvatarMixerWorkerPool(WorkerSharedData* workerSharedData, int numThreads = QThread::idealThreadCount()) :
        _workerSharedData(workerSharedData) { setNumThreads(numThreads); }
    ~AvatarMixerWorkerPool() { resize(0); }

    // Jobs the worker pool can do...
    void processIncomingPackets(ConstIter begin, ConstIter end);
    void broadcastAvatarData(ConstIter begin, ConstIter end, 
                    p_high_resolution_clock::time_point lastFrameTimestamp, float maxKbpsPerNode, float throttlingRatio);

    // iterate over all workers
    void each(std::function<void(AvatarMixerWorker& worker)> functor);

#ifdef DEBUG_EVENT_QUEUE
    void queueStats(QJsonObject& stats);
#endif

    void setNumThreads(int numThreads);
    int numThreads() const { return _numThreads; }

    void setPriorityReservedFraction(float fraction) { _priorityReservedFraction = fraction; }
    float getPriorityReservedFraction() const { return  _priorityReservedFraction; }

private:
    void run(ConstIter begin, ConstIter end);
    void resize(int numThreads);

    std::vector<std::unique_ptr<AvatarMixerWorkerThread>> _workers;

    friend void AvatarMixerWorkerThread::wait();
    friend void AvatarMixerWorkerThread::notify(bool stopping);
    friend bool AvatarMixerWorkerThread::try_pop(SharedNodePointer& node);

    // synchronization state
    Mutex _mutex;
    ConditionVariable _workerCondition;
    ConditionVariable _poolCondition;
    void (AvatarMixerWorker::*_function)(const SharedNodePointer& node);
    std::function<void(AvatarMixerWorker&)> _configure;

    // Set from Domain Settings:
    float _priorityReservedFraction { 0.4f };
    int _numThreads { 0 };

    int _numStarted { 0 }; // guarded by _mutex
    int _numFinished { 0 }; // guarded by _mutex
    int _numStopped { 0 }; // guarded by _mutex

    // frame state
    Queue _queue;
    ConstIter _begin;
    ConstIter _end;

    WorkerSharedData* _workerSharedData;
};

#endif // hifi_AvatarMixerWorkerPool_h
