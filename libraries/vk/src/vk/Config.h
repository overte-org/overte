//
//  Created by Bradley Austin Davis on 2016/03/19
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include <glm/glm.hpp>


#include <QtGlobal>
#if defined(Q_OS_WIN)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(Q_OS_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(Q_OS_DARWIN)
#else
//#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif

#define VKCPP_ENHANCED_MODE
#include <vulkan/vulkan.hpp>

#define VULKAN_USE_VMA 1

#if VULKAN_USE_VMA
#include <vma/vk_mem_alloc.h>
#endif
