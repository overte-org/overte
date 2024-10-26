//
//  AudioMixerWorkerPool.h
//  assignment-client/src/audio
//
//  Created by Zach Pomerantz on 11/16/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AudioMixerWorkerPool_h
#define hifi_AudioMixerWorkerPool_h

#include <condition_variable>
#include <mutex>
#include <vector>

#include <QThread>
#include <shared/QtHelpers.h>
#include <TBBHelpers.h>

#include "AudioMixerWorker.h"

class AudioMixerWorkerPool;

class AudioMixerWorkerThread : public QThread, public AudioMixerWorker {
    Q_OBJECT
    using ConstIter = NodeList::const_iterator;
    using Mutex = std::mutex;
    using Lock = std::unique_lock<Mutex>;

public:
    AudioMixerWorkerThread(AudioMixerWorkerPool& pool, AudioMixerWorker::SharedData& sharedData)
        : AudioMixerWorker(sharedData), _pool(pool) {}

    void run() override final;

private:
    friend class AudioMixerWorkerPool;

    void wait();
    void notify(bool stopping);
    bool try_pop(SharedNodePointer& node);

    AudioMixerWorkerPool& _pool;
    void (AudioMixerWorker::*_function)(const SharedNodePointer& node) { nullptr };
    bool _stop { false };
};

// Worker pool for audio mixers
//   AudioMixerWorkerPool is not thread-safe! It should be instantiated and used from a single thread.
class AudioMixerWorkerPool {
    using Queue = tbb::concurrent_queue<SharedNodePointer>;
    using Mutex = std::mutex;
    using Lock = std::unique_lock<Mutex>;
    using ConditionVariable = std::condition_variable;

public:
    using ConstIter = NodeList::const_iterator;

    AudioMixerWorkerPool(AudioMixerWorker::SharedData& sharedData, int numThreads = QThread::idealThreadCount())
        : _workerSharedData(sharedData) { setNumThreads(numThreads); }
    ~AudioMixerWorkerPool() { resize(0); }

    // process packets on worker threads
    void processPackets(ConstIter begin, ConstIter end);

    // mix on worker threads
    void mix(ConstIter begin, ConstIter end, unsigned int frame, int numToRetain);

    // iterate over all workers
    void each(std::function<void(AudioMixerWorker& worker)> functor);

#ifdef DEBUG_EVENT_QUEUE
    void queueStats(QJsonObject& stats);
#endif

    void setNumThreads(int numThreads);
    int numThreads() { return _numThreads; }

private:
    void run(ConstIter begin, ConstIter end);
    void resize(int numThreads);

    std::vector<std::unique_ptr<AudioMixerWorkerThread>> _workers;

    friend void AudioMixerWorkerThread::wait();
    friend void AudioMixerWorkerThread::notify(bool stopping);
    friend bool AudioMixerWorkerThread::try_pop(SharedNodePointer& node);

    // synchronization state
    Mutex _mutex;
    ConditionVariable _workerCondition;
    ConditionVariable _poolCondition;
    void (AudioMixerWorker::*_function)(const SharedNodePointer& node);
    std::function<void(AudioMixerWorker&)> _configure;
    int _numThreads { 0 };
    int _numStarted { 0 }; // guarded by _mutex
    int _numFinished { 0 }; // guarded by _mutex
    int _numStopped { 0 }; // guarded by _mutex

    // frame state
    Queue _queue;
    ConstIter _begin;
    ConstIter _end;

    AudioMixerWorker::SharedData& _workerSharedData;
};

#endif // hifi_AudioMixerWorkerPool_h
