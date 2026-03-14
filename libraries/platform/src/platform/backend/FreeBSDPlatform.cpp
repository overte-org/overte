//
//  Created by Amer Cerkic 05/02/2019
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FreeBSDPlatform.h"
#include "../PlatformKeys.h"

#include <thread>
#include <string>
#include <CPUIdent.h>

#include <QtCore/QtGlobal>

#include <GPUIdent.h>
#include <QSysInfo>

#ifdef Q_OS_FREEBSD
extern "C" {
#include <sys/sysctl.h>
}
#endif

using namespace platform;

void FreeBSDInstance::enumerateCpus() {
    json cpu = {};

    cpu[keys::cpu::vendor] = CPUIdent::Vendor();
    cpu[keys::cpu::model] = CPUIdent::Brand();
    cpu[keys::cpu::numCores] = std::thread::hardware_concurrency();

    _cpus.push_back(cpu);
}

void FreeBSDInstance::enumerateGpusAndDisplays() {
    GPUIdent* ident = GPUIdent::getInstance();
    json gpu = {};
    gpu[keys::gpu::model] = ident->getName().toUtf8().constData();
    gpu[keys::gpu::vendor] = findGPUVendorInDescription(gpu[keys::gpu::model].get<std::string>());
    gpu[keys::gpu::videoMemory] = ident->getMemory();
    gpu[keys::gpu::driver] = ident->getDriver().toUtf8().constData();

    _gpus.push_back(gpu);
    _displays = ident->getOutput();
}

void FreeBSDInstance::enumerateMemory() {
    json ram = {};

#ifdef Q_OS_FREEBSD
    u_int v_page_size, v_page_count;
    size_t size = sizeof(v_page_size);
    if (sysctlbyname("vm.stat.vm.v_page_size", &v_page_size, &size, NULL, 0) == -1)
        v_page_size = 0;
    size = sizeof(v_page_count);
    if (sysctlbyname("vm.stat.vm.v_page_count", &v_page_count, &size, NULL, 0) == -1)
        v_page_count = 0;

    ram[keys::memory::memTotal] = v_page_count * v_page_size;
#endif
    _memory = ram;
}

void FreeBSDInstance::enumerateComputer() {

    _computer[keys::computer::OS] = keys::computer::OS_FREEBSD;
    _computer[keys::computer::vendor] = "";
    _computer[keys::computer::model] = "";

    auto sysInfo = QSysInfo();

    _computer[keys::computer::OSVersion] = sysInfo.kernelVersion().toStdString();
}

