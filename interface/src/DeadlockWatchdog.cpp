//
//  DeadlockWatchdog.cpp
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

#include "DeadlockWatchdog.h"

std::atomic<bool> DeadlockWatchdogThread::_paused;
std::atomic<uint64_t> DeadlockWatchdogThread::_heartbeat;
std::atomic<uint64_t> DeadlockWatchdogThread::_maxElapsed;
std::atomic<int> DeadlockWatchdogThread::_maxElapsedAverage;
ThreadSafeMovingAverage<int, DeadlockWatchdogThread::HEARTBEAT_SAMPLES> DeadlockWatchdogThread::_movingAverage;
