//
//  scriptMemoryReport.js
//  scripts/developer/debugging
//
//  Created by dr Karol Suprynowicz on 2023/04/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

// A simple script that prints memory usage statistics for a given script engine every 5 seconds
// It can be included for example as a part of default scripts or controller scripts

var memoryStatisticsIntervalHandle = Script.setInterval(function () {
    statistics = Script.getMemoryUsageStatistics();
    if (statistics.scriptValueCount != null) {
        print("GC test script memory usage: Total heap size: " + statistics.totalHeapSize
            + " usedHeapSize: " + statistics.usedHeapSize
            + " totalAvailableSize: " + statistics.totalAvailableSize
            + " totalGlobalHandlesSize: " + statistics.totalGlobalHandlesSize
            + " usedGlobalHandlesSize: " + statistics.usedGlobalHandlesSize
            + " scriptValueCount: " + statistics.scriptValueCount
            + " scriptValueProxyCount: " + statistics.scriptValueProxyCount
            + " qObjectCount: " + statistics.qObjectCount);
    } else {
        print("GC test script memory usage: Total heap size: " + statistics.totalHeapSize
            + " usedHeapSize: " + statistics.usedHeapSize
            + " totalAvailableSize: " + statistics.totalAvailableSize
            + " totalGlobalHandlesSize: " + statistics.totalGlobalHandlesSize
            + " usedGlobalHandlesSize: " + statistics.usedGlobalHandlesSize);
    }
}, 5000);
Script.setInterval(function () {
    for (let i = 0; i < 50000; i++) {
        let dbgobj = Script.createGarbageCollectorDebuggingObject();
    }}, 10);
