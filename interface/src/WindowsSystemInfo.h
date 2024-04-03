//
//  WindowSystemInfo.h
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

#ifndef hifi_WindowSystemInfo_h
#define hifi_WindowSystemInfo_h

#include <qsystemdetection.h>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <TCHAR.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "ntdll.lib")

#include <mutex>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <QThread>
#include <QTimer>

#include "Profile.h"
#include "ThreadHelpers.h"

extern "C" {
    enum SYSTEM_INFORMATION_CLASS {
        SystemBasicInformation = 0,
        SystemProcessorPerformanceInformation = 8,
    };

    struct SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
        LARGE_INTEGER IdleTime;
        LARGE_INTEGER KernelTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER DpcTime;
        LARGE_INTEGER InterruptTime;
        ULONG InterruptCount;
    };

    struct SYSTEM_BASIC_INFORMATION {
        ULONG Reserved;
        ULONG TimerResolution;
        ULONG PageSize;
        ULONG NumberOfPhysicalPages;
        ULONG LowestPhysicalPageNumber;
        ULONG HighestPhysicalPageNumber;
        ULONG AllocationGranularity;
        ULONG_PTR MinimumUserModeAddress;
        ULONG_PTR MaximumUserModeAddress;
        ULONG_PTR ActiveProcessorsAffinityMask;
        CCHAR NumberOfProcessors;
    };

    NTSYSCALLAPI LONG NTAPI NtQuerySystemInformation(
        _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
        _Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
        _In_ ULONG SystemInformationLength,
        _Out_opt_ PULONG ReturnLength
    );

}
template <typename T>
LONG NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, T& t) {
    return NtQuerySystemInformation(SystemInformationClass, &t, (ULONG)sizeof(T), nullptr);
}

template <typename T>
LONG NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, std::vector<T>& t) {
    return NtQuerySystemInformation(SystemInformationClass, t.data(), (ULONG)(sizeof(T) * t.size()), nullptr);
}


template <typename T>
void updateValueAndDelta(std::pair<T, T>& pair, T newValue) {
    auto& value = pair.first;
    auto& delta = pair.second;
    delta = (value != 0) ? newValue - value : 0;
    value = newValue;
}

struct MyCpuInfo {
    using ValueAndDelta = std::pair<LONGLONG, LONGLONG>;
    std::string name;
    ValueAndDelta kernel { 0, 0 };
    ValueAndDelta user { 0, 0 };
    ValueAndDelta idle { 0, 0 };
    float kernelUsage { 0.0f };
    float userUsage { 0.0f };

    void update(const SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION& cpuInfo) {
        updateValueAndDelta(kernel, cpuInfo.KernelTime.QuadPart);
        updateValueAndDelta(user, cpuInfo.UserTime.QuadPart);
        updateValueAndDelta(idle, cpuInfo.IdleTime.QuadPart);
        auto totalTime = kernel.second + user.second + idle.second;
        if (totalTime != 0) {
            kernelUsage = (FLOAT)kernel.second / totalTime;
            userUsage = (FLOAT)user.second / totalTime;
        } else {
            kernelUsage = userUsage = 0.0f;
        }
    }
};

void updateCpuInformation() {
    static std::once_flag once;
    static SYSTEM_BASIC_INFORMATION systemInfo {};
    static SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION cpuTotals;
    static std::vector<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> cpuInfos;
    static std::vector<MyCpuInfo> myCpuInfos;
    static MyCpuInfo myCpuTotals;
    std::call_once(once, [&] {
        NtQuerySystemInformation( SystemBasicInformation, systemInfo);
        cpuInfos.resize(systemInfo.NumberOfProcessors);
        myCpuInfos.resize(systemInfo.NumberOfProcessors);
        for (size_t i = 0; i < systemInfo.NumberOfProcessors; ++i) {
            myCpuInfos[i].name = "cpu." + std::to_string(i);
        }
        myCpuTotals.name = "cpu.total";
    });
    NtQuerySystemInformation(SystemProcessorPerformanceInformation, cpuInfos);

    // Zero the CPU totals.
    memset(&cpuTotals, 0, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
    for (size_t i = 0; i < systemInfo.NumberOfProcessors; ++i) {
        auto& cpuInfo = cpuInfos[i];
        // KernelTime includes IdleTime.
        cpuInfo.KernelTime.QuadPart -= cpuInfo.IdleTime.QuadPart;

        // Update totals
        cpuTotals.IdleTime.QuadPart += cpuInfo.IdleTime.QuadPart;
        cpuTotals.KernelTime.QuadPart += cpuInfo.KernelTime.QuadPart;
        cpuTotals.UserTime.QuadPart += cpuInfo.UserTime.QuadPart;

        // Update friendly structure
        auto& myCpuInfo = myCpuInfos[i];
        myCpuInfo.update(cpuInfo);
        PROFILE_COUNTER(app, myCpuInfo.name.c_str(), {
            { "kernel", myCpuInfo.kernelUsage },
            { "user", myCpuInfo.userUsage }
        });
    }

    myCpuTotals.update(cpuTotals);
    PROFILE_COUNTER(app, myCpuTotals.name.c_str(), {
        { "kernel", myCpuTotals.kernelUsage },
        { "user", myCpuTotals.userUsage }
    });
}


static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;
static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void initCpuUsage() {
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));

    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

void getCpuUsage(glm::vec3& systemAndUser) {
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    systemAndUser.x = (sys.QuadPart - lastSysCPU.QuadPart);
    systemAndUser.y = (user.QuadPart - lastUserCPU.QuadPart);
    systemAndUser /= (float)(now.QuadPart - lastCPU.QuadPart);
    systemAndUser /= (float)numProcessors;
    systemAndUser *= 100.0f;
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;

    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    systemAndUser.z = (float)counterVal.doubleValue;
}

void setupCpuMonitorThread() {
    initCpuUsage();
    auto cpuMonitorThread = QThread::currentThread();
    setThreadName("CPU Monitor Thread");

    QTimer* timer = new QTimer();
    timer->setInterval(50);
    QObject::connect(timer, &QTimer::timeout, [] {
        updateCpuInformation();
        glm::vec3 kernelUserAndSystem;
        getCpuUsage(kernelUserAndSystem);
        PROFILE_COUNTER(app, "cpuProcess", { { "system", kernelUserAndSystem.x }, { "user", kernelUserAndSystem.y } });
        PROFILE_COUNTER(app, "cpuSystem", { { "system", kernelUserAndSystem.z } });
    });
    QObject::connect(cpuMonitorThread, &QThread::finished, [=] {
        timer->deleteLater();
        cpuMonitorThread->deleteLater();
    });
    timer->start();
}

#endif

#endif  // hifi_WindowSystemInfo_h
