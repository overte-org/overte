#pragma once

#include "Config.h"

#include "Debug.h"
#include "Image.h"
#include "VulkanBuffer.h"
#include "Helpers.h"
#include "VulkanDevice.h"
#include "VulkanDebug.h"
#include "VulkanTools.h"
#include <unordered_set>

namespace vks {
using StringList = std::list<std::string>;
using CStringVector = std::vector<const char*>;

using DevicePickerFunction = std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&)>;
using DeviceExtensionsPickerFunction = std::function<std::set<std::string>(const VkPhysicalDevice&)>;
using InstanceExtensionsPickerFunction = std::function<std::set<std::string>()>;
using InstanceExtensionsPickerFunctions = std::list<InstanceExtensionsPickerFunction>;
using LayerVector = std::vector<const char*>;
using MipData = ::std::pair<VkExtent3D, VkDeviceSize>;

/*namespace queues {

struct DeviceCreateInfo : public VkDeviceCreateInfo {
    std::vector<VkDeviceQueueCreateInfo> deviceQueues;
    std::vector<std::vector<float>> deviceQueuesPriorities;

    void addQueueFamily(uint32_t queueFamilyIndex, vk::ArrayProxy<float> priorities) {
        deviceQueues.push_back({ {}, queueFamilyIndex });
        std::vector<float> prioritiesVector;
        prioritiesVector.resize(priorities.size());
        memcpy(prioritiesVector.data(), priorities.data(), sizeof(float) * priorities.size());
        deviceQueuesPriorities.push_back(prioritiesVector);
    }
    void addQueueFamily(uint32_t queueFamilyIndex, size_t count = 1) {
        std::vector<float> priorities;
        priorities.resize(count);
        std::fill(priorities.begin(), priorities.end(), 0.0f);
        addQueueFamily(queueFamilyIndex, priorities);
    }

    void update() {
        assert(deviceQueuesPriorities.size() == deviceQueues.size());
        size_t size = deviceQueues.size();
        for (size_t i = 0; i < size; ++i) {
            auto& deviceQueue = deviceQueues[i];
            auto& deviceQueuePriorities = deviceQueuesPriorities[i];
            deviceQueue.queueCount = (uint32_t)deviceQueuePriorities.size();
            deviceQueue.pQueuePriorities = deviceQueuePriorities.data();
        }

        this->queueCreateInfoCount = (uint32_t)deviceQueues.size();
        this->pQueueCreateInfos = deviceQueues.data();
    }
};
}*/  // namespace queues
///////////////////////////////////////////////////////////////////////
//
// Object destruction support
//
// It's often critical to avoid destroying an object that may be in use by the GPU.  In order to service this need
// the context class contains structures for objects that are pending deletion.
//
// The first container is the dumpster, and it just contains a set of lambda objects that when executed, destroy
// resources (presumably... in theory the lambda can do anything you want, but the purpose is to contain GPU object
// destruction calls).
//
// When the application makes use of a function that uses a fence, it can provide that fence to the context as a marker
// for destroying all the pending objects.  Anything in the dumpster is migrated to the recycler.
//
// Finally, an application can call the recycle function at regular intervals (perhaps once per frame, perhaps less often)
// in order to check the fences and execute the associated destructors for any that are signalled.
using VoidLambda = std::function<void()>;
using VoidLambdaList = std::list<VoidLambda>;
using FencedLambda = std::pair<VkFence, VoidLambda>;
using FencedLambdaQueue = std::queue<FencedLambda>;

struct Context {
private:
    //static CStringVector toCStrings(const StringList& values);

    //static CStringVector toCStrings(const vk::ArrayProxy<const std::string>& values);

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

    static bool isDeviceExtensionPresent(const VkPhysicalDevice& physicalDevice, const std::string& extension) {
        return getDeviceExtensionNames(physicalDevice).count(extension) != 0;
    }

    void requireExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions) {
        requiredExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
    }

    void requireDeviceExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions) {
        requiredDeviceExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
    }

    void addInstanceExtensionPicker(const InstanceExtensionsPickerFunction& function) {
        instanceExtensionsPickers.push_back(function);
    }

    void setDevicePicker(const DevicePickerFunction& picker) { devicePicker = picker; }

    void setDeviceExtensionsPicker(const DeviceExtensionsPickerFunction& picker) { deviceExtensionsPicker = picker; }

    void setValidationEnabled(bool enable);

    void createInstance();

    void destroyContext();

    uint32_t findQueue(const VkQueueFlags& desiredFlags, const VkSurfaceKHR& presentSurface = VkSurfaceKHR()) const;

    template <typename T>
    void trash(T value, std::function<void(T t)> destructor = [](T t) { t.destroy(); }) const {
        if (!value) {
            return;
        }
        dumpster.push_back([=] { destructor(value); });
    }

    template <typename T>
    void trashAll(const std::vector<T>& values, std::function<void(const std::vector<T>&)> destructor) const {
        if (values.empty()) {
            return;
        }
        dumpster.push_back([=] { destructor(values); });
    }

    //
    // Convenience functions for trashing specific types.  These functions know what kind of function
    // call to make for destroying a given Vulkan object.
    //

    /*void trashPipeline(VkPipeline& pipeline) const {
        trash<VkPipeline>(pipeline, [this](VkPipeline pipeline) { device.destroyPipeline(pipeline); });
    }*/

    void trashCommandBuffers(const std::vector<VkCommandBuffer>& cmdBuffers, VkCommandPool commandPool = nullptr) const;

    // Should be called from time to time by the application to migrate zombie resources
    // to the recycler along with a fence that will be signalled when the objects are
    // safe to delete.
    void emptyDumpster(vk::Fence fence);

    // Check the recycler fences for signalled status.  Any that are signalled will have their corresponding
    // lambdas executed, freeing up the associated resources
    void recycle();

    // Create an image memory barrier for changing the layout of
    // an image and put it into an active command buffer
    // See chapter 11.4 "vk::Image Layout" for details

    void setImageLayout(VkCommandBuffer cmdbuffer,
                        VkImage image,
                        VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout,
                        VkImageSubresourceRange subresourceRange) const;

    // Fixed sub resource on first mip level and layer
    void setImageLayout(VkCommandBuffer cmdbuffer,
                        VkImage image,
                        VkImageAspectFlags aspectMask,
                        VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout) const;

    void setImageLayout(VkImage image,
                        VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout,
                        VkImageSubresourceRange subresourceRange) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) {
            setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange);
        });
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(VkImage image,
                        VkImageAspectFlags aspectMask,
                        VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) {
            setImageLayout(commandBuffer, image, aspectMask, oldImageLayout, newImageLayout);
        });
    }

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
    //std::unordered_set<std::string> physicalDeviceExtensions;
    // TODO: this needs to be filled in
    VkPhysicalDeviceFeatures enabledFeatures{};

    std::shared_ptr<vks::VulkanDevice> device;

    VkQueue queue;

    const VkCommandPool& getCommandPool() const;

    /*std::vector<VkCommandBuffer> allocateCommandBuffers(
        uint32_t count,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;*/

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

    //void flushCommandBuffer(VkCommandBuffer& commandBuffer) const;

    // Create a short-lived command buffer which is immediately executed and released
    // This function is intended for initialization only.  It incurs a queue and device
    // flush and may impact performance if used in non-setup code
    void withPrimaryCommandBuffer(const std::function<void(const VkCommandBuffer& commandBuffer)>& f) const {
        vk::CommandBuffer commandBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
        };
        commandBuffer.begin(vkCommandBufferBeginInfo);
        f(commandBuffer);
        commandBuffer.end();
        device->flushCommandBuffer(commandBuffer, queue, true);
    }

    Image createImage(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags& memoryPropertyFlags) const;

    Image stageToDeviceImage(VkImageCreateInfo imageCreateInfo,
                             const VkMemoryPropertyFlags& memoryPropertyFlags,
                             VkDeviceSize size,
                             const void* data,
                             const std::vector<MipData>& mipData = {}) const;

    template <typename T>
    Image stageToDeviceImage(const VkImageCreateInfo& imageCreateInfo,
                             const VkMemoryPropertyFlags& memoryPropertyFlags,
                             const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, memoryPropertyFlags, data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Image stageToDeviceImage(const VkImageCreateInfo& imageCreateInfo, const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, data.size() * sizeof(T),
                                  (void*)data.data());
    }

    Buffer createBuffer(const VkBufferUsageFlags& usageFlags,
                        VkDeviceSize size,
                        const VkMemoryPropertyFlags& memoryPropertyFlags) const;

    Buffer createDeviceBuffer(const VkBufferUsageFlags& usageFlags, VkDeviceSize size) const;

    Buffer createStagingBuffer(VkDeviceSize size, const void* data = nullptr) const;

    template <typename T>
    Buffer createStagingBuffer(const std::vector<T>& data) const {
        return createBuffer(data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Buffer createStagingBuffer(const T& data) const {
        return createStagingBuffer(sizeof(T), &data);
    }

    /*template <typename T>
    Buffer createUniformBuffer(const T& data, size_t count = 3) const {
        auto alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        auto extra = sizeof(T) % alignment;
        auto alignedSize = sizeof(T) + (alignment - extra);
        auto allocatedSize = count * alignedSize;
        static const auto usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        static const auto memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        auto result = createBuffer(usageFlags, allocatedSize, memoryFlags);
        result.alignment = alignedSize;
        result.descriptor.range = result.alignment;
        VK_CHECK_RESULT(result.map());
        for (size_t i = 0; i < count; i++) {
            memcpy(result.mapped, );
        }
        // This is a bit weird, what's the purpose of "count", if data is not an array?
        //result.copy(data);
        return result;
    }*/

    Buffer stageToDeviceBuffer(const VkBufferUsageFlags& usage, size_t size, const void* data) const;

    template <typename T>
    Buffer stageToDeviceBuffer(const VkBufferUsageFlags& usage, const std::vector<T>& data) const {
        return stageToDeviceBuffer(usage, sizeof(T) * data.size(), data.data());
    }

    template <typename T>
    Buffer stageToDeviceBuffer(const VkBufferUsageFlags& usage, const T& data) const {
        return stageToDeviceBuffer(usage, sizeof(T), (void*)&data);
    }

    //vk::Bool32 getMemoryType(uint32_t typeBits, const VkMemoryPropertyFlags& properties, uint32_t* typeIndex) const;

    //uint32_t getMemoryType(uint32_t typeBits, const VkMemoryPropertyFlags& properties) const;

    VkFormat getSupportedDepthFormat() const;

private:
    // A collection of items queued for destruction.  Once a fence has been created
    // for a queued submit, these items can be moved to the recycler for actual destruction
    // by calling the rec
    mutable VoidLambdaList dumpster;
    FencedLambdaQueue recycler;

    InstanceExtensionsPickerFunctions instanceExtensionsPickers;
    // Set to true when example is created with enabled validation layers
    bool enableValidation = false;
    // Set to true when the debug marker extension is detected
    bool enableDebugMarkers = false;

    std::set<std::string> requiredExtensions;
    std::set<std::string> requiredDeviceExtensions;

    DevicePickerFunction devicePicker = [](const std::vector<VkPhysicalDevice>& devices) -> VkPhysicalDevice {
        return devices[0];
    };

    DeviceExtensionsPickerFunction deviceExtensionsPicker = [](const VkPhysicalDevice& device) -> std::set<std::string> {
        return {};
    };
};

using ContextPtr = std::shared_ptr<Context>;
}  // namespace vks

