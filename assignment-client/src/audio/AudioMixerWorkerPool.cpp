//
//  AudioMixerWorkerPool.cpp
//  assignment-client/src/audio
//
//  Created by Zach Pomerantz on 11/16/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AudioMixerWorkerPool.h"

#include <QObject>

#include <assert.h>
#include <algorithm>

#include <ThreadHelpers.h>

void AudioMixerWorkerThread::run() {
    while (true) {
        wait();

        // iterate over all available nodes
        SharedNodePointer node;
        while (try_pop(node)) {
            (this->*_function)(node);
        }

        bool stopping = _stop;
        notify(stopping);
        if (stopping) {
            return;
        }
    }
}

void AudioMixerWorkerThread::wait() {
    {
        Lock lock(_pool._mutex);
        _pool._workerCondition.wait(lock, [&] {
            assert(_pool._numStarted <= _pool._numThreads);
            return _pool._numStarted != _pool._numThreads;
        });
        ++_pool._numStarted;
    }

    if (_pool._configure) {
        _pool._configure(*this);
    }
    _function = _pool._function;
}

void AudioMixerWorkerThread::notify(bool stopping) {
    {
        Lock lock(_pool._mutex);
        assert(_pool._numFinished < _pool._numThreads);
        ++_pool._numFinished;
        if (stopping) {
            ++_pool._numStopped;
        }
    }
    _pool._poolCondition.notify_one();
}

bool AudioMixerWorkerThread::try_pop(SharedNodePointer& node) {
    return _pool._queue.try_pop(node);
}

void AudioMixerWorkerPool::processPackets(ConstIter begin, ConstIter end) {
    _function = &AudioMixerWorker::processPackets;
    _configure = [](AudioMixerWorker& worker) {};
    run(begin, end);
}

void AudioMixerWorkerPool::mix(ConstIter begin, ConstIter end, unsigned int frame, int numToRetain) {
    _function = &AudioMixerWorker::mix;
    _configure = [=](AudioMixerWorker& worker) {
        worker.configureMix(_begin, _end, frame, numToRetain);
    };

    run(begin, end);
}

void AudioMixerWorkerPool::run(ConstIter begin, ConstIter end) {
    _begin = begin;
    _end = end;

    // fill the queue
    std::for_each(_begin, _end, [&](const SharedNodePointer& node) {
        _queue.push(node);
    });

    {
        Lock lock(_mutex);

        // run
        _numStarted = _numFinished = 0;
        _workerCondition.notify_all();

        // wait
        _poolCondition.wait(lock, [&] {
            assert(_numFinished <= _numThreads);
            return _numFinished == _numThreads;
        });

        assert(_numStarted == _numThreads);
    }

    assert(_queue.empty());
}

void AudioMixerWorkerPool::each(std::function<void(AudioMixerWorker& worker)> functor) {
    for (auto& worker : _workers) {
        functor(*worker.get());
    }
}

#ifdef DEBUG_EVENT_QUEUE
void AudioMixerWorkerPool::queueStats(QJsonObject& stats) {
    unsigned i = 0;
    for (auto& worker : _workers) {
        int queueSize = ::hifi::qt::getEventQueueSize(worker.get());
        QString queueName = QString("audio_thread_event_queue_%1").arg(i);
        stats[queueName] = queueSize;

        i++;
    }
}
#endif // DEBUG_EVENT_QUEUE

void AudioMixerWorkerPool::setNumThreads(int numThreads) {
    // clamp to allowed size
    {
        int maxThreads = QThread::idealThreadCount();
        if (maxThreads == -1) {
            // idealThreadCount returns -1 if cores cannot be detected
            static const int MAX_THREADS_IF_UNKNOWN = 4;
            maxThreads = MAX_THREADS_IF_UNKNOWN;
        }

        int clampedThreads = std::min(std::max(1, numThreads), maxThreads);
        if (clampedThreads != numThreads) {
            qWarning("%s: clamped to %d (was %d)", __FUNCTION__, clampedThreads, numThreads);
            numThreads = clampedThreads;
        }
    }

    resize(numThreads);
}

void AudioMixerWorkerPool::resize(int numThreads) {
    assert(_numThreads == (int)_workers.size());

    qDebug("%s: set %d threads (was %d)", __FUNCTION__, numThreads, _numThreads);

    Lock lock(_mutex);

    if (numThreads > _numThreads) {
        // start new workers
        for (int i = 0; i < numThreads - _numThreads; ++i) {
            auto worker = new AudioMixerWorkerThread(*this, _workerSharedData);
            QObject::connect(worker, &QThread::started, [] { setThreadName("AudioMixerWorkerThread"); });
            worker->start();
            _workers.emplace_back(worker);
        }
    } else if (numThreads < _numThreads) {
        auto extraBegin = _workers.begin() + numThreads;

        // mark workers to stop...
        auto worker = extraBegin;
        while (worker != _workers.end()) {
            (*worker)->_stop = true;
            ++worker;
        }

        // ...cycle them until they do stop...
        _numStopped = 0;
        while (_numStopped != (_numThreads - numThreads)) {
            _numStarted = _numFinished = _numStopped;
            _workerCondition.notify_all();
            _poolCondition.wait(lock, [&] {
                assert(_numFinished <= _numThreads);
                return _numFinished == _numThreads;
            });
        }

        // ...wait for threads to finish...
        worker = extraBegin;
        while (worker != _workers.end()) {
            QThread* thread = reinterpret_cast<QThread*>(worker->get());
            static const int MAX_THREAD_WAIT_TIME = 10;
            thread->wait(MAX_THREAD_WAIT_TIME);
            ++worker;
        }

        // ...and erase them
        _workers.erase(extraBegin, _workers.end());
    }

    _numThreads = _numStarted = _numFinished = numThreads;
    assert(_numThreads == (int)_workers.size());
}
