#pragma once

#include "Config.h"

#include "Debug.h"
//#include "Image.h"
#include "VulkanBuffer.h"
#include "Helpers.h"
#include "VulkanDevice.h"
#include "VulkanDebug.h"
#include "VulkanTools.h"
#include <unordered_set>

namespace vks {
using StringList = std::list<std::string>;
using CStringVector = std::vector<const char*>;

struct Context {
private:
    static CStringVector filterLayers(const StringList& desiredLayers);

    Context() {};
public:
    static Context& get();

    // Create application wide Vulkan instance
    static std::set<std::string> getAvailableLayers();

    static std::vector<VkExtensionProperties> getExtensions();

    static std::set<std::string> getExtensionNames();

    static bool isExtensionPresent(const std::string& extensionName) { return getExtensionNames().count(extensionName) != 0; }

    static std::vector<VkExtensionProperties> getDeviceExtensions(const VkPhysicalDevice& physicalDevice);

    static std::set<std::string> getDeviceExtensionNames(const VkPhysicalDevice& physicalDevice);

    void requireExtensions(const std::set<std::string>& requestedExtensions) {
        requiredExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
    }

    void requireDeviceExtensions(const std::set<std::string>& requestedExtensions) {
        requiredDeviceExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
    }

    void setValidationEnabled(bool enable);

    void createInstance();

    void destroyContext();

    void createDevice();

protected:
    void pickDevice();

    void buildDevice();

public:
    // Vulkan instance, stores all per-application states
    VkInstance instance;

    std::vector<VkPhysicalDevice> physicalDevices;
    // Physical device (GPU) that Vulkan will use
    VkPhysicalDevice physicalDevice;
    // TODO: this needs to be filled in
    VkPhysicalDeviceFeatures enabledFeatures{};

    std::shared_ptr<vks::VulkanDevice> device;

    VkQueue graphicsQueue;
    VkQueue transferQueue;

    VkCommandBuffer createCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

    Buffer createBuffer(const VkBufferUsageFlags& usageFlags,
                        VkDeviceSize size,
                        const VkMemoryPropertyFlags& memoryPropertyFlags) const;

    Buffer createDeviceBuffer(const VkBufferUsageFlags& usageFlags, VkDeviceSize size) const;

private:
    // Set to true when example is created with enabled validation layers
    bool enableValidation = false;
    // Set to true when the debug marker extension is detected
    bool enableDebugMarkers = false;

    std::set<std::string> requiredExtensions;
    std::set<std::string> requiredDeviceExtensions;
};

}  // namespace vks

