//
//  Created by Bradley Austin Davis on 2018/10/29
//
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
//  Contains parts of Vulkan Samples, Copyright (c) 2018, Sascha Willems, distributed on MIT License.
//

#pragma once

#include <cstdint>
#include <string>
#include <sstream>

namespace vks {

    // Version information for Vulkan is stored in a single 32 bit integer
    // with individual bits representing the major, minor and patch versions.
    // The maximum possible major and minor version is 512 (look out nVidia)
    // while the maximum possible patch version is 2048
    struct Version {
        Version() : vulkan_major(0), vulkan_minor(0), vulkan_patch(0) {
        }
        explicit Version(uint32_t version) : Version() {
            *this = version;
        }

        Version& operator =(uint32_t version) {
            memcpy(this, &version, sizeof(uint32_t));
            return *this;
        }

        explicit operator uint32_t() const {
            uint32_t result;
            memcpy(&result, this, sizeof(uint32_t));
            return result;
        }

        std::string toString() const {
            std::stringstream buffer;
            buffer << vulkan_major << "." << vulkan_minor << "." << vulkan_patch;
            return buffer.str();
        }

        const uint32_t vulkan_major : 10;
        const uint32_t vulkan_minor : 10;
        const uint32_t vulkan_patch : 12;

    };
}
