#pragma once

#include "Config.h"

#include "Debug.h"
#include "Image.h"
#include "Buffer.h"
#include "Helpers.h"
#include "Device.h"
#include <unordered_set>

namespace vks {
using StringList = std::list<std::string>;
using CStringVector = std::vector<const char*>;

using DevicePickerFunction = std::function<vk::PhysicalDevice(const std::vector<vk::PhysicalDevice>&)>;
using DeviceExtensionsPickerFunction = std::function<std::set<std::string>(const vk::PhysicalDevice&)>;
using InstanceExtensionsPickerFunction = std::function<std::set<std::string>()>;
using InstanceExtensionsPickerFunctions = std::list<InstanceExtensionsPickerFunction>;
using LayerVector = std::vector<const char*>;
using MipData = ::std::pair<vk::Extent3D, vk::DeviceSize>;

namespace queues {

struct DeviceCreateInfo : public vk::DeviceCreateInfo {
    std::vector<vk::DeviceQueueCreateInfo> deviceQueues;
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
}  // namespace queues
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
using FencedLambda = std::pair<vk::Fence, VoidLambda>;
using FencedLambdaQueue = std::queue<FencedLambda>;

struct Context {
private:
    static CStringVector toCStrings(const StringList& values) {
        CStringVector result;
        result.reserve(values.size());
        for (const auto& string : values) {
            result.push_back(string.c_str());
        }
        return result;
    }

    static CStringVector toCStrings(const vk::ArrayProxy<const std::string>& values) {
        CStringVector result;
        result.reserve(values.size());
        for (const auto& string : values) {
            result.push_back(string.c_str());
        }
        return result;
    }

    static CStringVector filterLayers(const StringList& desiredLayers) {
        static std::set<std::string> validLayerNames = getAvailableLayers();
        CStringVector result;
        for (const auto& string : desiredLayers) {
            if (validLayerNames.count(string) != 0) {
                result.push_back(string.c_str());
            }
        }
        return result;
    }

    Context() {};
public:
    static Context& get();

    // Create application wide Vulkan instance
    static std::set<std::string> getAvailableLayers() {
        std::set<std::string> result;
        auto layers = vk::enumerateInstanceLayerProperties();
        for (auto layer : layers) {
            result.insert(layer.layerName);
        }
        return result;
    }

    static std::vector<vk::ExtensionProperties> getExtensions() { return vk::enumerateInstanceExtensionProperties(); }

    static std::set<std::string> getExtensionNames() {
        std::set<std::string> extensionNames;
        for (auto& ext : getExtensions()) {
            extensionNames.insert(ext.extensionName);
        }
        return extensionNames;
    }

    static bool isExtensionPresent(const std::string& extensionName) { return getExtensionNames().count(extensionName) != 0; }

    static std::vector<vk::ExtensionProperties> getDeviceExtensions(const vk::PhysicalDevice& physicalDevice) {
        return physicalDevice.enumerateDeviceExtensionProperties();
    }

    static std::set<std::string> getDeviceExtensionNames(const vk::PhysicalDevice& physicalDevice) {
        std::set<std::string> extensionNames;
        for (auto& ext : getDeviceExtensions(physicalDevice)) {
            extensionNames.insert(ext.extensionName);
        }
        return extensionNames;
    }

    static bool isDeviceExtensionPresent(const vk::PhysicalDevice& physicalDevice, const std::string& extension) {
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

    void setValidationEnabled(bool enable) {
        if (instance) {
            throw std::runtime_error("Cannot change validations state after instance creation");
        }
        enableValidation = enable;
    }

    void createInstance() {
        if (instance) {
            throw std::runtime_error("Instance already exists");
        }

        if (isExtensionPresent(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            requireExtensions({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME });
            enableValidation = true;
            enableDebugMarkers = true;
            qDebug() << "Found debug marker extension";
        }

        // Vulkan instance
        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "VulkanExamples";
        appInfo.pEngineName = "VulkanExamples";
        appInfo.apiVersion = VK_API_VERSION_1_0;



        std::set<std::string> instanceExtensions;
        instanceExtensions.insert(requiredExtensions.begin(), requiredExtensions.end());
        for (const auto& picker : instanceExtensionsPickers) {
            auto extensions = picker();
            instanceExtensions.insert(extensions.begin(), extensions.end());
        }

        std::vector<const char*> enabledExtensions;
        for (const auto& extension : instanceExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }

        // Enable surface extensions depending on os
        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        if (enabledExtensions.size() > 0) {
            instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }

        CStringVector layers;
        if (enableValidation) {
            layers = filterLayers(debug::getDefaultValidationLayers());
            instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
            instanceCreateInfo.ppEnabledLayerNames = layers.data();
        }

        instance = vk::createInstance(instanceCreateInfo);

        if (enableValidation) {
            debug::setupDebugging(instance);
        }

        if (enableDebugMarkers) {
            debug::marker::setup(instance);
        }
    }

    void destroyContext() {
        queue.waitIdle();
        device.waitIdle();
        for (const auto& trash : dumpster) {
            trash();
        }

        while (!recycler.empty()) {
            recycle();
        }

        destroyCommandPool();
        device.destroy();
        if (enableValidation) {
            debug::cleanupDebugging(instance);
        }
        instance.destroy();
    }

    uint32_t findQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& presentSurface = vk::SurfaceKHR()) const {
        uint32_t bestMatch{ VK_QUEUE_FAMILY_IGNORED };
        VkQueueFlags bestMatchExtraFlags{ VK_QUEUE_FLAG_BITS_MAX_ENUM };
        size_t queueCount = queueFamilyProperties.size();
        for (uint32_t i = 0; i < queueCount; ++i) {
            auto currentFlags = queueFamilyProperties[i].queueFlags;
            // Doesn't contain the required flags, skip it
            if (!(currentFlags & desiredFlags)) {
                continue;
            }

            VkQueueFlags currentExtraFlags = (currentFlags & ~desiredFlags).operator VkQueueFlags();

            // If we find an exact match, return immediately
            if (0 == currentExtraFlags) {
                return i;
            }

            if (bestMatch == VK_QUEUE_FAMILY_IGNORED || currentExtraFlags < bestMatchExtraFlags) {
                bestMatch = i;
                bestMatchExtraFlags = currentExtraFlags;
            }
        }

        return bestMatch;
    }

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

    void trashPipeline(vk::Pipeline& pipeline) const {
        trash<vk::Pipeline>(pipeline, [this](vk::Pipeline pipeline) { device.destroyPipeline(pipeline); });
    }

    void trashCommandBuffers(const std::vector<vk::CommandBuffer>& cmdBuffers, vk::CommandPool commandPool = nullptr) const {
        if (!commandPool) {
            commandPool = getCommandPool();
        }
        using DtorLambda = std::function<void(const std::vector<vk::CommandBuffer>&)>;
        DtorLambda destructor =
            [=](const std::vector<vk::CommandBuffer>& cmdBuffers) { 
            device.freeCommandBuffers(commandPool, cmdBuffers); 
        };
        trashAll<vk::CommandBuffer>(cmdBuffers, destructor);
    }

    // Should be called from time to time by the application to migrate zombie resources
    // to the recycler along with a fence that will be signalled when the objects are
    // safe to delete.
    void emptyDumpster(vk::Fence fence) {
        VoidLambdaList newDumpster;
        newDumpster.swap(dumpster);
        recycler.push(FencedLambda{ fence, [fence, newDumpster, this] {
                                       for (const auto& f : newDumpster) {
                                           f();
                                       }
                                   } });
    }

    // Check the recycler fences for signalled status.  Any that are signalled will have their corresponding
    // lambdas executed, freeing up the associated resources
    void recycle() {
        while (!recycler.empty()) {
            const auto& trashItem = recycler.front();
            const auto& fence = trashItem.first;
            auto fenceStatus = device.getFenceStatus(fence);
            if (vk::Result::eSuccess != fenceStatus) {
                break;
            }
            const VoidLambda& lambda = trashItem.second;
            lambda();
            device.destroyFence(fence);
            recycler.pop();
        }
    }

    // Create an image memory barrier for changing the layout of
    // an image and put it into an active command buffer
    // See chapter 11.4 "vk::Image Layout" for details

    void setImageLayout(vk::CommandBuffer cmdbuffer,
                        vk::Image image,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout,
                        vk::ImageSubresourceRange subresourceRange) const {
        // Create an image barrier object
        vk::ImageMemoryBarrier imageMemoryBarrier;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = vks::util::accessFlagsForLayout(oldImageLayout);
        imageMemoryBarrier.dstAccessMask = vks::util::accessFlagsForLayout(newImageLayout);

        // Put barrier on top
        // Put barrier inside setup command buffer
        cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                                  vk::DependencyFlags(), nullptr, nullptr, imageMemoryBarrier);
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(vk::CommandBuffer cmdbuffer,
                        vk::Image image,
                        vk::ImageAspectFlags aspectMask,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout) const {
        vk::ImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;
        setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange);
    }

    void setImageLayout(vk::Image image,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout,
                        vk::ImageSubresourceRange subresourceRange) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) {
            setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange);
        });
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(vk::Image image,
                        vk::ImageAspectFlags aspectMask,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) {
            setImageLayout(commandBuffer, image, aspectMask, oldImageLayout, newImageLayout);
        });
    }

    void createDevice(const vk::SurfaceKHR& surface = nullptr) {
        pickDevice(surface);

        buildDevice();

#if VULKAN_USE_VMA
        vks::Allocation::initAllocator(physicalDevice, device);
#endif

        // Get the graphics queue
        queue = device.getQueue(queueIndices.graphics, 0);
    }

protected:
    void pickDevice(const vk::SurfaceKHR& surface ) {
        // Physical device
        physicalDevices = instance.enumeratePhysicalDevices();

        // Note :
        // This example will always use the first physical device reported,
        // change the vector index if you have multiple Vulkan devices installed
        // and want to use another one
        physicalDevice = devicePicker(physicalDevices);
        struct Version {
            uint32_t patch : 12;
            uint32_t minor : 10;
            uint32_t major : 10;
        } _version;


        for (const auto& extensionProperties : physicalDevice.enumerateDeviceExtensionProperties()) {
            physicalDeviceExtensions.insert(extensionProperties.extensionName);
            qDebug() << "Device Extension " << extensionProperties.extensionName;
        }
        // Store properties (including limits) and features of the phyiscal device
        // So examples can check against them and see if a feature is actually supported
        queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        deviceProperties = physicalDevice.getProperties();
        memcpy(&_version, &deviceProperties.apiVersion, sizeof(uint32_t));
        deviceFeatures = physicalDevice.getFeatures();
        // Gather physical device memory properties
        deviceMemoryProperties = physicalDevice.getMemoryProperties();
        queueIndices.graphics = findQueue(vk::QueueFlagBits::eGraphics, surface);
        queueIndices.compute = findQueue(vk::QueueFlagBits::eCompute);
        queueIndices.transfer = findQueue(vk::QueueFlagBits::eTransfer);
    }

    void buildDevice() {
        // Vulkan device
        vks::queues::DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.addQueueFamily(queueIndices.graphics, queueFamilyProperties[queueIndices.graphics].queueCount);
        if (queueIndices.compute != VK_QUEUE_FAMILY_IGNORED && queueIndices.compute != queueIndices.graphics) {
            deviceCreateInfo.addQueueFamily(queueIndices.compute, queueFamilyProperties[queueIndices.compute].queueCount);
        }
        if (queueIndices.transfer != VK_QUEUE_FAMILY_IGNORED && queueIndices.transfer != queueIndices.graphics &&
            queueIndices.transfer != queueIndices.compute) {
            deviceCreateInfo.addQueueFamily(queueIndices.transfer, queueFamilyProperties[queueIndices.transfer].queueCount);
        }
        deviceCreateInfo.update();

        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        std::set<std::string> requestedDeviceExtensions = deviceExtensionsPicker(physicalDevice);
        requestedDeviceExtensions.insert(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        // enable the debug marker extension if it is present (likely meaning a debugging tool is present)

        std::vector<const char*> enabledExtensions;
        for (const auto& extension : requestedDeviceExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }


        if (enabledExtensions.size() > 0) {
            deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }
        device = physicalDevice.createDevice(deviceCreateInfo);
    }

public:
    // Vulkan instance, stores all per-application states
    vk::Instance instance;

    std::vector<vk::PhysicalDevice> physicalDevices;
    // Physical device (GPU) that Vulkan will ise
    vk::PhysicalDevice physicalDevice;
    std::unordered_set<std::string> physicalDeviceExtensions;

    // Queue family properties
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    // Stores physical device properties (for e.g. checking device limits)
    vk::PhysicalDeviceProperties deviceProperties;
    // Stores phyiscal device features (for e.g. checking if a feature is available)
    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::PhysicalDeviceFeatures enabledFeatures;
    // Stores all available memory (type) properties for the physical device
    vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
    // Logical device, application's view of the physical device (GPU)
    vks::Device device;

    struct QueueIndices {
        uint32_t graphics{ VK_QUEUE_FAMILY_IGNORED };
        uint32_t transfer{ VK_QUEUE_FAMILY_IGNORED };
        uint32_t compute{ VK_QUEUE_FAMILY_IGNORED };
    } queueIndices;

    vk::Queue queue;

    const vk::CommandPool& getCommandPool() const {
        if (!commandPool) {
            vk::CommandPoolCreateInfo cmdPoolInfo;
            cmdPoolInfo.queueFamilyIndex = queueIndices.graphics;
            cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            auto pool = device.createCommandPool(cmdPoolInfo);
            const_cast<vk::CommandPool&>(commandPool) = pool;
        }
        return commandPool;
    }

    void destroyCommandPool() {
        if (commandPool) {
            device.destroy(commandPool);
            commandPool = nullptr;
        }
    }

    std::vector<vk::CommandBuffer> allocateCommandBuffers(
        uint32_t count,
        vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
        std::vector<vk::CommandBuffer> result;
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.commandPool = getCommandPool();
        commandBufferAllocateInfo.commandBufferCount = count;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        result = device.allocateCommandBuffers(commandBufferAllocateInfo);
        return result;
    }

    vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
        vk::CommandBuffer cmdBuffer;
        vk::CommandBufferAllocateInfo cmdBufAllocateInfo;
        cmdBufAllocateInfo.commandPool = getCommandPool();
        cmdBufAllocateInfo.level = level;
        cmdBufAllocateInfo.commandBufferCount = 1;
        cmdBuffer = device.allocateCommandBuffers(cmdBufAllocateInfo)[0];
        return cmdBuffer;
    }

    void flushCommandBuffer(vk::CommandBuffer& commandBuffer) const {
        if (!commandBuffer) {
            return;
        }
        queue.submit(vk::SubmitInfo{ 0, nullptr, nullptr, 1, &commandBuffer }, vk::Fence());
        queue.waitIdle();
        device.waitIdle();
    }

    // Create a short lived command buffer which is immediately executed and released
    // This function is intended for initialization only.  It incurs a queue and device
    // flush and may impact performance if used in non-setup code
    void withPrimaryCommandBuffer(const std::function<void(const vk::CommandBuffer& commandBuffer)>& f) const {
        vk::CommandBuffer commandBuffer = createCommandBuffer(vk::CommandBufferLevel::ePrimary);
        commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        f(commandBuffer);
        commandBuffer.end();
        flushCommandBuffer(commandBuffer);
        device.freeCommandBuffers(getCommandPool(), commandBuffer);
    }

    Image createImage(const vk::ImageCreateInfo& imageCreateInfo, const vk::MemoryPropertyFlags& memoryPropertyFlags) const {
        Image result;
        result.device = device;
        result.format = imageCreateInfo.format;
        result.extent = imageCreateInfo.extent;

#if VULKAN_USE_VMA
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryPropertyFlags.operator unsigned int();
        auto pCreateInfo = &(imageCreateInfo.operator const VkImageCreateInfo&());
        auto pImage = &reinterpret_cast<VkImage&>(result.image);
        vmaCreateImage(Allocation::getAllocator(), pCreateInfo, &allocInfo, pImage, &result.allocation, nullptr);
#else
        result.image = device.createImage(imageCreateInfo);
        if ((memoryPropertyFlags & vk::MemoryPropertyFlagBits::eLazilyAllocated) !=
            vk::MemoryPropertyFlagBits::eLazilyAllocated) {
            vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(result.image);
            vk::MemoryAllocateInfo memAllocInfo;
            memAllocInfo.allocationSize = result.allocSize = memReqs.size;
            memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
            result.memory = device.allocateMemory(memAllocInfo);
            device.bindImageMemory(result.image, result.memory, 0);
        }
#endif
        return result;
    }

    Image stageToDeviceImage(vk::ImageCreateInfo imageCreateInfo,
                             const vk::MemoryPropertyFlags& memoryPropertyFlags,
                             vk::DeviceSize size,
                             const void* data,
                             const std::vector<MipData>& mipData = {}) const {
        Buffer staging = createStagingBuffer(size, data);
        imageCreateInfo.usage = imageCreateInfo.usage | vk::ImageUsageFlagBits::eTransferDst;
        Image result = createImage(imageCreateInfo, memoryPropertyFlags);

        withPrimaryCommandBuffer([&](const vk::CommandBuffer& copyCmd) {
            vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, imageCreateInfo.mipLevels, 0, 1);
            // Prepare for transfer
            setImageLayout(copyCmd, result.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, range);

            // Prepare for transfer
            std::vector<vk::BufferImageCopy> bufferCopyRegions;
            {
                vk::BufferImageCopy bufferCopyRegion;
                bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                bufferCopyRegion.imageSubresource.layerCount = 1;
                if (!mipData.empty()) {
                    for (uint32_t i = 0; i < imageCreateInfo.mipLevels; i++) {
                        bufferCopyRegion.imageSubresource.mipLevel = i;
                        bufferCopyRegion.imageExtent = mipData[i].first;
                        bufferCopyRegions.push_back(bufferCopyRegion);
                        bufferCopyRegion.bufferOffset += mipData[i].second;
                    }
                } else {
                    bufferCopyRegion.imageExtent = imageCreateInfo.extent;
                    bufferCopyRegions.push_back(bufferCopyRegion);
                }
            }
            copyCmd.copyBufferToImage(staging.buffer, result.image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
            // Prepare for shader read
            setImageLayout(copyCmd, result.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                           range);
        });
        staging.destroy();
        return result;
    }

    template <typename T>
    Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo,
                             const vk::MemoryPropertyFlags& memoryPropertyFlags,
                             const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, memoryPropertyFlags, data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo, const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, data.size() * sizeof(T),
                                  (void*)data.data());
    }

    Buffer createBuffer(const vk::BufferUsageFlags& usageFlags,
                        vk::DeviceSize size,
                        const vk::MemoryPropertyFlags& memoryPropertyFlags) const {
        Buffer result;
        result.device = device;
        result.size = size;
        result.descriptor.range = size;
        result.descriptor.offset = 0;

        vk::BufferCreateInfo createInfo{ {}, size, usageFlags };

#if VULKAN_USE_VMA
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.requiredFlags = memoryPropertyFlags.operator unsigned int();
        auto pCreateInfo = &createInfo.operator const VkBufferCreateInfo&();
        auto pBuffer = &reinterpret_cast<VkBuffer&>(result.buffer);
        vmaCreateBuffer(Allocation::getAllocator(), pCreateInfo, &allocInfo, pBuffer, &result.allocation, nullptr);
#else
        result.descriptor.buffer = result.buffer = device.createBuffer(bufferCreateInfo);
        vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(result.buffer);
        vk::MemoryAllocateInfo memAlloc;
        result.allocSize = memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
        result.memory = device.allocateMemory(memAlloc);
        device.bindBufferMemory(result.buffer, result.memory, 0);
#endif
        result.descriptor.buffer = result.buffer;
        return result;
    }

    Buffer createDeviceBuffer(const vk::BufferUsageFlags& usageFlags, vk::DeviceSize size) const {
        static const vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        return createBuffer(usageFlags, size, memoryProperties);
    }

    Buffer createStagingBuffer(vk::DeviceSize size, const void* data = nullptr) const {
        auto result = createBuffer(vk::BufferUsageFlagBits::eTransferSrc, size,
                                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        if (data != nullptr) {
            result.map();
            result.copy(size, data);
            result.unmap();
        }
        return result;
    }

    template <typename T>
    Buffer createStagingBuffer(const std::vector<T>& data) const {
        return createBuffer(data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Buffer createStagingBuffer(const T& data) const {
        return createStagingBuffer(sizeof(T), &data);
    }

    template <typename T>
    Buffer createUniformBuffer(const T& data, size_t count = 3) const {
        auto alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        auto extra = sizeof(T) % alignment;
        auto alignedSize = sizeof(T) + (alignment - extra);
        auto allocatedSize = count * alignedSize;
        static const auto usageFlags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
        static const auto memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        auto result = createBuffer(usageFlags, allocatedSize, memoryFlags);
        result.alignment = alignedSize;
        result.descriptor.range = result.alignment;
        result.map();
        result.copy(data);
        return result;
    }

    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, size_t size, const void* data) const {
        Buffer staging = createStagingBuffer(size, data);
        Buffer result = createDeviceBuffer(usage | vk::BufferUsageFlagBits::eTransferDst, size);
        withPrimaryCommandBuffer(
            [&](vk::CommandBuffer copyCmd) { copyCmd.copyBuffer(staging.buffer, result.buffer, vk::BufferCopy(0, 0, size)); });
        staging.destroy();
        return result;
    }

    template <typename T>
    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, const std::vector<T>& data) const {
        return stageToDeviceBuffer(usage, sizeof(T) * data.size(), data.data());
    }

    template <typename T>
    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, const T& data) const {
        return stageToDeviceBuffer(usage, sizeof(T), (void*)&data);
    }

    vk::Bool32 getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t* typeIndex) const {
        for (uint32_t i = 0; i < 32; i++) {
            if ((typeBits & 1) == 1) {
                if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    *typeIndex = i;
                    return true;
                }
            }
            typeBits >>= 1;
        }
        return false;
    }

    uint32_t getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const {
        uint32_t result = 0;
        if (!getMemoryType(typeBits, properties, &result)) {
            // todo : throw error
        }
        return result;
    }

    vk::Format getSupportedDepthFormat() const {
        // Since all depth formats may be optional, we need to find a suitable depth format to use
        // Start with the highest precision packed format
        std::vector<vk::Format> depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat,
                                                 vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint,
                                                 vk::Format::eD16Unorm };

        for (auto& format : depthFormats) {
            vk::FormatProperties formatProps;
            formatProps = physicalDevice.getFormatProperties(format);
            // vk::Format must support depth stencil attachment for optimal tiling
            if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                return format;
            }
        }

        throw std::runtime_error("No supported depth format");
    }


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

    DevicePickerFunction devicePicker = [](const std::vector<vk::PhysicalDevice>& devices) -> vk::PhysicalDevice {
        return devices[0];
    };

    DeviceExtensionsPickerFunction deviceExtensionsPicker = [](const vk::PhysicalDevice& device) -> std::set<std::string> {
        return {};
    };

    vk::CommandPool commandPool;
};

using ContextPtr = std::shared_ptr<Context>;
}  // namespace vks
