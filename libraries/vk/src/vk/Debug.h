#pragma once

#include <QtCore/qmetatype.h>
#include <QtCore/qtextstream.h>
#include "Config.h"

namespace vks { namespace debug {

using StringList = std::list<std::string>;
const StringList& getDefaultValidationLayers();

using SevBits = vk::DebugUtilsMessageSeverityFlagBitsEXT;
using TypeBits = vk::DebugUtilsMessageTypeFlagBitsEXT;
using SevFlags = vk::DebugUtilsMessageSeverityFlagsEXT;
using TypeFlags = vk::DebugUtilsMessageTypeFlagsEXT;
using CallbackData = vk::DebugUtilsMessengerCallbackDataEXT;

using Output = std::function<void(const SevFlags&, const std::string&)>;
Output setOutputFunction(const Output& function);

using MessageFormatter = std::function<std::string(const SevFlags&, const TypeFlags&, const CallbackData*, void*)>;
void setMessageFormatter(const MessageFormatter& function);

void setupDebugging(const vk::Instance& instance,
                    const SevFlags& severityFlags = SevBits::eError | SevBits::eWarning,
                    const TypeFlags& typeFlags = TypeBits::eGeneral | TypeBits::eValidation | TypeBits::ePerformance,
                    void* userData = nullptr);

// Clear debug callback
void cleanupDebugging(const vk::Instance& instance);

// Setup and functions for the VK_EXT_debug_marker_extension
// Extension spec can be found at https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/doc/specs/vulkan/appendices/VK_EXT_debug_marker.txt
// Note that the extension will only be present if run from an offline debugging application
// The actual check for extension presence and enabling it on the device is done in the example base class
// See ExampleBase::createInstance and ExampleBase::createDevice (base/vkx::ExampleBase.cpp)

namespace marker {

// Get function pointers for the debug report extensions from the device
void setup(const vk::Instance& instance);

// Start a new debug marker region
void beginRegion(const vk::CommandBuffer& cmdbuffer, const std::string& pMarkerName, const glm::vec4& color);

// Insert a new debug marker into the command buffer
void insert(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const glm::vec4& color);

// End the current debug marker region
void endRegion(const vk::CommandBuffer& cmdbuffer);

// Sets the debug name of an object
// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
// along with the object type
void setObjectName(const vk::Device& device, uint64_t object, vk::ObjectType objectType, const std::string& name);

// Object specific naming functions
void setName(const vk::Device& device, const vk::CommandBuffer& obj, const std::string& name);
void setName(const vk::Device& device, const vk::Queue& obj, const std::string& name);
void setName(const vk::Device& device, const vk::Image& obj, const std::string& name);
void setName(const vk::Device& device, const vk::Buffer& obj, const std::string& name);
void setName(const vk::Device& device, const vk::Framebuffer& obj, const std::string& name);
void setName(const vk::Device& device, const vk::Pipeline& obj, const std::string& name);

}  // namespace marker

}}  // namespace vks::debug
