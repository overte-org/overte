//
//  DeadlockWatchdog.h
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_DeadlockWatchdog_h
#define hifi_DeadlockWatchdog_h

#include <QThread>

#include <NumericalConstants.h>
#include <SharedUtil.h>
#include <crash-handler/CrashHandler.h>

#include "InterfaceLogging.h"
#include "SimpleMovingAverage.h"

class DeadlockWatchdogThread : public QThread {
public:
    static const unsigned long HEARTBEAT_UPDATE_INTERVAL_SECS = 1;
    static const unsigned long MAX_HEARTBEAT_AGE_USECS = 120 * USECS_PER_SECOND; // 2 mins with no checkin probably a deadlock
    static const int WARNING_ELAPSED_HEARTBEAT = 500 * USECS_PER_MSEC; // warn if elapsed heartbeat average is large
    static const int HEARTBEAT_SAMPLES = 100000; // ~5 seconds worth of samples

    // Set the heartbeat on launch
    DeadlockWatchdogThread() {
        setObjectName("Deadlock Watchdog");
        // Give the heartbeat an initial value
        _heartbeat = usecTimestampNow();
        _paused = false;
        connect(qApp, &QCoreApplication::aboutToQuit, [this] {
            _quit = true;
        });
    }

    void setMainThreadID(Qt::HANDLE threadID) {
        _mainThreadID = threadID;
    }

    static void updateHeartbeat() {
        auto now = usecTimestampNow();
        auto elapsed = now - _heartbeat;
        _movingAverage.addSample(elapsed);
        _heartbeat = now;
    }

    void deadlockDetectionCrash() {
        auto &ch = CrashHandler::getInstance();

        ch.setAnnotation("_mod_faulting_tid", std::to_string((uint64_t)_mainThreadID));
        ch.setAnnotation("deadlock", "1");
        uint32_t* crashTrigger = nullptr;
        *crashTrigger = 0xDEAD10CC;
    }

    static void withPause(const std::function<void()>& lambda) {
        pause();
        lambda();
        resume();
    }
    static void pause() {
        _paused = true;
    }

    static void resume() {
        // Update the heartbeat BEFORE resuming the checks
        updateHeartbeat();
        _paused = false;
    }

    void run() override {
        while (!_quit) {
            QThread::sleep(HEARTBEAT_UPDATE_INTERVAL_SECS);
            // Don't do heartbeat detection under nsight
            if (_paused) {
                continue;
            }
            uint64_t lastHeartbeat = _heartbeat; // sample atomic _heartbeat, because we could context switch away and have it updated on us
            uint64_t now = usecTimestampNow();
            auto lastHeartbeatAge = (now > lastHeartbeat) ? now - lastHeartbeat : 0;
            auto elapsedMovingAverage = _movingAverage.getAverage();

            if (elapsedMovingAverage > _maxElapsedAverage * 1.1f) {
#if !defined(NDEBUG)
                qCDebug(interfaceapp_deadlock) << "DEADLOCK WATCHDOG WARNING:"
                    << "lastHeartbeatAge:" << lastHeartbeatAge
                    << "elapsedMovingAverage:" << elapsedMovingAverage
                    << "maxElapsed:" << _maxElapsed
                    << "PREVIOUS maxElapsedAverage:" << _maxElapsedAverage
                    << "NEW maxElapsedAverage:" << elapsedMovingAverage << "** NEW MAX ELAPSED AVERAGE **"
                    << "samples:" << _movingAverage.getSamples();
#endif
                _maxElapsedAverage = elapsedMovingAverage;
            }
            if (lastHeartbeatAge > _maxElapsed) {
#if !defined(NDEBUG)
                qCDebug(interfaceapp_deadlock) << "DEADLOCK WATCHDOG WARNING:"
                    << "lastHeartbeatAge:" << lastHeartbeatAge
                    << "elapsedMovingAverage:" << elapsedMovingAverage
                    << "PREVIOUS maxElapsed:" << _maxElapsed
                    << "NEW maxElapsed:" << lastHeartbeatAge << "** NEW MAX ELAPSED **"
                    << "maxElapsedAverage:" << _maxElapsedAverage
                    << "samples:" << _movingAverage.getSamples();
#endif
                _maxElapsed = lastHeartbeatAge;
            }

#if !defined(NDEBUG)
            if (elapsedMovingAverage > WARNING_ELAPSED_HEARTBEAT) {
                qCDebug(interfaceapp_deadlock) << "DEADLOCK WATCHDOG WARNING:"
                    << "lastHeartbeatAge:" << lastHeartbeatAge
                    << "elapsedMovingAverage:" << elapsedMovingAverage << "** OVER EXPECTED VALUE **"
                    << "maxElapsed:" << _maxElapsed
                    << "maxElapsedAverage:" << _maxElapsedAverage
                    << "samples:" << _movingAverage.getSamples();
            }
#endif

            if (lastHeartbeatAge > MAX_HEARTBEAT_AGE_USECS) {
                qCDebug(interfaceapp_deadlock) << "DEADLOCK DETECTED -- "
                         << "lastHeartbeatAge:" << lastHeartbeatAge
                         << "[ lastHeartbeat :" << lastHeartbeat
                         << "now:" << now << " ]"
                         << "elapsedMovingAverage:" << elapsedMovingAverage
                         << "maxElapsed:" << _maxElapsed
                         << "maxElapsedAverage:" << _maxElapsedAverage
                         << "samples:" << _movingAverage.getSamples();

                // Don't actually crash in debug builds, in case this apparent deadlock is simply from
                // the developer actively debugging code
                #ifdef NDEBUG
                deadlockDetectionCrash();
                #endif
            }
        }
    }

    static std::atomic<bool> _paused;
    static std::atomic<uint64_t> _heartbeat;
    static std::atomic<uint64_t> _maxElapsed;
    static std::atomic<int> _maxElapsedAverage;
    static ThreadSafeMovingAverage<int, HEARTBEAT_SAMPLES> _movingAverage;

    bool _quit { false };

    Qt::HANDLE _mainThreadID = nullptr;
};

#endif  // hifi_DeadlockWatchdog_h
