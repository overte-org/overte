#pragma once

#include "Config.h"

#include <unordered_set>
#include <mutex>

#include "Debug.h"
#include "VulkanBuffer.h"
#include "Helpers.h"
#include "VulkanDevice.h"
#include "VulkanDebug.h"
#include "VulkanTools.h"

namespace gpu::vk {
    class VKFramebuffer;
    class VKBuffer;
    class VKTexture;
    class VKQuery;
    class VKBackend;

    // Extension for sharing memory with OpenGL, needed by QML UI and Web entities.
    extern PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR;
}

class VKWindow;
class VKWidget;

namespace vks {

// Start of VKS code

using StringList = std::list<std::string>;
using CStringVector = std::vector<const char*>;

struct Context {
    friend class ::VKWindow;
    friend class ::VKWidget;
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

private:
    // Set to true when example is created with enabled validation layers
    bool enableValidation = false;
    // Set to true when the debug marker extension is detected
    bool enableDebugMarkers = false;

    std::set<std::string> requiredExtensions;
    std::set<std::string> requiredDeviceExtensions;

    // End of VKS code

public:


    void registerWindow(VKWindow *window); // Every Vulkan window registers itself here.

    void unregisterWindow(VKWindow *window); // When window closes before backend was shut down, it removes itself from the list.

    void registerWidget(VKWidget *widget); // Every Vulkan widget registers itself here, so its.

    void unregisterWidget(VKWidget *widget); // When widget closes before backend was shut down, it removes itself from the list.

    void shutdownWindows(); // When backend shuts down, it cleans up after all windows and removes them from the list.

    // Contains objects that need to be deleted on Vulkan backend thread after frame is rendered.
    // It's filled by destructors of objects like gpu::Texture and gpu::Buffer, since these destroy
    // backend counterpart of their objects.
    class Recycler {
    public:
        // This means that for every GPU object mutex will be locked and unlocked several times.
        // VKTODO: It would be good to do profiling and check if it impacts performance or not.
        void trashVkSampler(VkSampler &sampler);
        void trashVkFramebuffer(VkFramebuffer &framebuffer);
        void trashVkImageView(VkImageView &imageView);
        void trashVkImage(VkImage &image);
        void trashVkBuffer(VkBuffer &buffer);
        void trashVkRenderPass(VkRenderPass &renderPass);
        void trashVkDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);
        void trashVkPipelineLayout(VkPipelineLayout& pipelineLayout);
        void trashVkPipeline(VkPipeline &pipeline);
        void trashVkShaderModule(VkShaderModule &module);
        void trashVkSwapchainKHR(VkSwapchainKHR &swapchain);
        void trashVKSurfaceKHR(VkSurfaceKHR &surface);
        void trashVkSemaphore(VkSemaphore &semaphore);
        void trashVkDeviceMemory(VkDeviceMemory &memory);
        void trashVmaAllocation(VmaAllocation &allocation);

        void framebufferDeleted(gpu::vk::VKFramebuffer *framebuffer);
        void bufferDeleted(gpu::vk::VKBuffer *buffer);
        void textureDeleted(gpu::vk::VKTexture *texture);
        void queryDeleted(gpu::vk::VKQuery *query);

    private:
        std::recursive_mutex recyclerMutex;

        std::vector<VkFramebuffer> vkFramebuffers;
        std::vector<VkSampler> vkSamplers;
        std::vector<VkImageView> vkImageViews;
        std::vector<VkImage> vkImages;
        std::vector<VkBuffer> vkBuffers;
        std::vector<VkRenderPass> vkRenderPasses;
        std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
        std::vector<VkPipelineLayout > vkPipelineLayouts;
        std::vector<VkPipeline> vkPipelines;
        std::vector<VkShaderModule> vkShaderModules;
        std::vector<VkSwapchainKHR> vkSwapchainsKHR;
        std::vector<VkSurfaceKHR> vkSurfacesKHR;
        std::vector<VkSemaphore> vkSemaphores;
        std::vector<VkDeviceMemory> vkDeviceMemories;
        std::vector<VmaAllocation> vmaAllocations;

        // List of pointers to objects that were deleted and need to be removed from backend object sets.
        std::vector<gpu::vk::VKFramebuffer*> deletedFramebuffers;
        std::vector<gpu::vk::VKBuffer*> deletedBuffers;
        std::vector<gpu::vk::VKTexture*> deletedTextures;
        std::vector<gpu::vk::VKQuery*> deletedQueries;
        friend class gpu::vk::VKBackend;
    } recycler;

private:
    std::recursive_mutex vulkanWindowsMutex;
    std::list<VKWindow*> vulkanWindows;
    std::list<VKWidget*> vulkanWidgets;

};

}  // namespace vks

