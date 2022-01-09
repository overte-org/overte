/*
* Vulkan examples debug wrapper
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include "Debug.h"

#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <mutex>


namespace vks { namespace debug {

const StringList& getDefaultValidationLayers() {
    static const StringList validationLayerNames {
#if defined(__ANDROID__)
        "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_swapchain", "VK_LAYER_GOOGLE_unique_objects",
#else
        "VK_LAYER_LUNARG_standard_validation",
#endif
    };
    return validationLayerNames;
}

const Output DEFAULT_OUTPUT = [](const SevFlags& sevFlags, const std::string& message) {
#ifdef _MSC_VER
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
#endif
    std::stringstream buf;
    if (sevFlags & SevBits::eError) {
        std::cout << "ERROR: ";
    } else if (sevFlags & SevBits::eWarning) {
        std::cout << "WARNING: ";
    } else if (sevFlags & SevBits::eInfo) {
        std::cout << "INFO: ";
    } else if (sevFlags & SevBits::eVerbose) {
        std::cout << "VERBOSE: ";
    } else {
        std::cout << "Unknown sev: ";
    }

    std::cout << message << std::endl;
};

const MessageFormatter DEFAULT_MESSAGE_FORMATTER =
    [](const SevFlags& sevFlags, const TypeFlags& typeFlags, const CallbackData* callbackData, void*) -> std::string {
    // FIXME improve on this
    return std::string(callbackData->pMessage);
};

MessageFormatter CURRENT_FORMATTER = DEFAULT_MESSAGE_FORMATTER;
Output CURRENT_OUTPUT = DEFAULT_OUTPUT;

Output setOutputFunction(const Output& function) {
    Output result = function;
    std::swap(result, CURRENT_OUTPUT);
    return result;
}

void setMessageFormatter(const MessageFormatter& function) {
    CURRENT_FORMATTER = function;
}

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                       void* pUserData) {
    SevFlags sevFlags;
    reinterpret_cast<VkDebugUtilsMessageSeverityFlagBitsEXT&>(sevFlags) = messageSeverity;
    TypeFlags typeFlags{ messageType };
    auto callbackData = reinterpret_cast<const CallbackData*>(pCallbackData);
    auto message = CURRENT_FORMATTER(sevFlags, typeFlags, callbackData, pUserData);
    CURRENT_OUTPUT(sevFlags, message);
    return VK_TRUE;
}

static vk::DebugUtilsMessengerEXT messenger{};

const vk::DispatchLoaderDynamic& getInstanceDispatcher(const vk::Instance& instance = nullptr) {
    static vk::DispatchLoaderDynamic dispatcher;
    static std::once_flag once;
    if (instance) {
        std::call_once(once, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
    }
    return dispatcher;
}

void setupDebugging(const vk::Instance& instance, const SevFlags& severityFlags, const TypeFlags& typeFlags, void* userData) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{ {}, severityFlags, typeFlags, debugCallback, userData };
    messenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, getInstanceDispatcher(instance));
}

void cleanupDebugging(const vk::Instance& instance) {
    instance.destroyDebugUtilsMessengerEXT(messenger, nullptr, getInstanceDispatcher(instance));
}

namespace marker {

static bool active = false;

void setup(const vk::Instance& instance) {
    qDebug() << "QQQ" << __FUNCTION__ << "setup debugging";
    const auto& dispatcher = getInstanceDispatcher(instance);
    active = (nullptr != dispatcher.vkSetDebugUtilsObjectTagEXT);
}

void setObjectName(const vk::Device& device, uint64_t object, vk::ObjectType objectType, const std::string& name) {
    const auto& dispatcher = getInstanceDispatcher();
    if (active) {
        device.setDebugUtilsObjectNameEXT({ objectType, object, name.c_str() }, getInstanceDispatcher());
    }
}

void beginRegion(const vk::CommandBuffer& cmdbuffer, const std::string& name, const glm::vec4& color) {
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (active) {
        cmdbuffer.beginDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT{ name.c_str(), { { color.r, color.g, color.b, color.a } } },
                                          getInstanceDispatcher());
    }
}

void insert(const vk::CommandBuffer& cmdbuffer, const std::string& name, const glm::vec4& color) {
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (active) {
        cmdbuffer.insertDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT{ name.c_str(), { { color.r, color.g, color.b, color.a } } },
                                           getInstanceDispatcher());
    }
}

void endRegion(const vk::CommandBuffer& cmdbuffer) {
    // Check for valid function (may not be present if not running in a debugging application)
    if (active) {
        cmdbuffer.endDebugUtilsLabelEXT(getInstanceDispatcher());
    }
}

void setName(const vk::Device& device, const vk::CommandBuffer& obj, const std::string& name) {
    setObjectName(device, (uint64_t)obj.operator VkCommandBuffer(), vk::ObjectType::eCommandBuffer, name);
}

void setName(const vk::Device& device, const vk::Queue& obj, const std::string& name) {
    setObjectName(device, (uint64_t)obj.operator VkQueue(), vk::ObjectType::eQueue, name);
}

void setName(const vk::Device& device, const vk::Image& obj, const std::string& name) {
    setObjectName(device, (uint64_t)obj.operator VkImage(), vk::ObjectType::eImage, name);
}

void setName(const vk::Device& device, const vk::Buffer& obj, const std::string& name) {
    setObjectName(device, (uint64_t)obj.operator VkBuffer(), vk::ObjectType::eBuffer, name);
}

void setName(const vk::Device& device, const vk::Framebuffer& obj, const std::string& name) {
    setObjectName(device, (uint64_t)obj.operator VkFramebuffer(), vk::ObjectType::eFramebuffer, name);
}

void setName(const vk::Device& device, const vk::Pipeline& obj, const std::string& name) {
    setObjectName(device, (uint64_t)obj.operator VkPipeline(), vk::ObjectType::ePipeline, name);
}

}  // namespace marker

}}  // namespace vks::debug
