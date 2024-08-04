#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include "Context.h"

using namespace vks;

Context& Context::get() {
    static Context INSTANCE;
    return INSTANCE;
}

/*CStringVector Context::toCStrings(const StringList& values) {
    CStringVector result;
    result.reserve(values.size());
    for (const auto& string : values) {
        result.push_back(string.c_str());
    }
    return result;
}

CStringVector Context::toCStrings(const vk::ArrayProxy<const std::string>& values) {
    CStringVector result;
    result.reserve(values.size());
    for (const auto& string : values) {
        result.push_back(string.c_str());
    }
    return result;
}*/

CStringVector Context::filterLayers(const StringList& desiredLayers) {
    static std::set<std::string> validLayerNames = getAvailableLayers();
    CStringVector result;
    for (const auto& string : desiredLayers) {
        if (validLayerNames.count(string) != 0) {
            result.push_back(string.c_str());
        }
    }
    return result;
}

std::set<std::string> Context::getAvailableLayers() {
    std::set<std::string> result;
    uint32_t instanceLayerCount;
    VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
    std::vector<VkLayerProperties> layers(instanceLayerCount);
    VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data()));
    for (auto layer : layers) {
        result.insert(layer.layerName);
    }
    return result;
}

std::vector<VkExtensionProperties> Context::getExtensions() {
    uint32_t extensionPropertyCount;
    VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &extensionPropertyCount, nullptr));
    std::vector<VkExtensionProperties> result(extensionPropertyCount);
    VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &extensionPropertyCount, result.data()));
    return result;
}

std::set<std::string> Context::getExtensionNames() {
    std::set<std::string> extensionNames;
    for (auto& ext : getExtensions()) {
        extensionNames.insert(ext.extensionName);
    }
    return extensionNames;
}

std::vector<VkExtensionProperties> Context::getDeviceExtensions(const VkPhysicalDevice& physicalDevice) {
    uint32_t propertyCount;
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr));
    std::vector<VkExtensionProperties> result(propertyCount);
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, result.data()));
    return result;
}

std::set<std::string> Context::getDeviceExtensionNames(const VkPhysicalDevice& physicalDevice) {
    std::set<std::string> extensionNames;
    for (auto& ext : getDeviceExtensions(physicalDevice)) {
        extensionNames.insert(ext.extensionName);
    }
    return extensionNames;
}

void Context::setValidationEnabled(bool enable) {
    if (instance) {
        throw std::runtime_error("Cannot change validations state after instance creation");
    }
    enableValidation = enable;
}

void Context::createInstance() {
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
    VkApplicationInfo appInfo;
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
    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enabledExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }

    CStringVector layers;
    if (enableValidation) {
        std::list<std::string> validationLayers {"VK_LAYER_LUNARG_standard_validation"};
        layers = filterLayers(validationLayers);
        instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
        instanceCreateInfo.ppEnabledLayerNames = layers.data();
    } else {
        Q_ASSERT(false);
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    Q_ASSERT(result == VK_SUCCESS);

    if (enableValidation) {
        debug::setupDebugging(instance);
    }

    if (enableDebugMarkers) {
        debugutils::setup(instance);
    }
}

void Context::destroyContext() {
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));
    for (const auto& trash : dumpster) {
        trash();
    }

    while (!recycler.empty()) {
        recycle();
    }

    device.reset();
    if (enableValidation) {
        debug::freeDebugCallback(instance);
    }
    vkDestroyInstance(instance, nullptr);
}

/*uint32_t Context::findQueue(const VkQueueFlags& desiredFlags, const VkSurfaceKHR& presentSurface) const {
    uint32_t bestMatch{ VK_QUEUE_FAMILY_IGNORED };
    VkQueueFlags bestMatchExtraFlags{ VK_QUEUE_FLAG_BITS_MAX_ENUM };
    size_t queueCount = queueFamilyProperties.size();
    for (uint32_t i = 0; i < queueCount; ++i) {
        auto currentFlags = queueFamilyProperties[i].queueFlags;
        // Doesn't contain the required flags, skip it
        if (!(currentFlags & desiredFlags)) {
            continue;
        }

        VkQueueFlags currentExtraFlags = (currentFlags & ~desiredFlags);

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
}*/

void Context::trashCommandBuffers(const std::vector<VkCommandBuffer>& cmdBuffers, VkCommandPool commandPool) const {
    if (!commandPool) {
        commandPool = getCommandPool();
    }
    using DtorLambda = std::function<void(const std::vector<VkCommandBuffer>&)>;
    DtorLambda destructor =
        [=](const std::vector<VkCommandBuffer>& cmdBuffers) {
            vkFreeCommandBuffers(device->logicalDevice, commandPool, cmdBuffers.size(), cmdBuffers.data());
        };
    trashAll<VkCommandBuffer>(cmdBuffers, destructor);
}

void Context::emptyDumpster(vk::Fence fence) {
    VoidLambdaList newDumpster;
    newDumpster.swap(dumpster);
    recycler.push(FencedLambda{ fence, [fence, newDumpster, this] {
                                   for (const auto& f : newDumpster) {
                                       f();
                                   }
                               } });
}

void Context::recycle() {
    while (!recycler.empty()) {
        const auto& trashItem = recycler.front();
        const auto& fence = trashItem.first;
        auto fenceStatus = vkGetFenceStatus(device->logicalDevice, fence);
        if (fenceStatus != VK_SUCCESS) {
            break;
        }
        const VoidLambda& lambda = trashItem.second;
        lambda();
        vkDestroyFence(device->logicalDevice, fence, nullptr);
        recycler.pop();
    }
}

void Context::setImageLayout(VkCommandBuffer cmdbuffer,
                    VkImage image,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout,
                    VkImageSubresourceRange subresourceRange) const {
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = vks::util::accessFlagsForLayout(oldImageLayout);
    imageMemoryBarrier.dstAccessMask = vks::util::accessFlagsForLayout(newImageLayout);

    // Put barrier on top
    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &imageMemoryBarrier);
}

// Fixed sub resource on first mip level and layer
void Context::setImageLayout(VkCommandBuffer cmdbuffer,
                    VkImage image,
                    VkImageAspectFlags aspectMask,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout) const {
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange);
}

/*void Context::setImageLayout(VkImage image,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout,
                    VkImageSubresourceRange subresourceRange) const {
    withPrimaryCommandBuffer([&](const auto& commandBuffer) {
        setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange);
    });
}

// Fixed sub resource on first mip level and layer
void Context::setImageLayout(VkImage image,
                    VkImageAspectFlags aspectMask,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout) const {
    withPrimaryCommandBuffer([&](const auto& commandBuffer) {
        setImageLayout(commandBuffer, image, aspectMask, oldImageLayout, newImageLayout);
    });
}*/

void Context::createDevice(const VkSurfaceKHR& surface) {
    pickDevice(surface);

    buildDevice();

#if VULKAN_USE_VMA
    vks::Allocation::initAllocator(physicalDevice, device->logicalDevice);
#endif

    // Get the graphics queue
    vkGetDeviceQueue(device->logicalDevice, device->queueFamilyIndices.graphics, 0, &queue);
    //queue = device.getQueue(queueIndices.graphics, 0);
}

void Context::pickDevice(const VkSurfaceKHR& surface ) {
    // Physical device
    uint32_t physicalDeviceCount;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    physicalDevices.resize(physicalDeviceCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    // Note :
    // This example will always use the first physical device reported,
    // change the vector index if you have multiple Vulkan devices installed
    // and want to use another one
    physicalDevice = devicePicker(physicalDevices);
    /*struct Version {
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
    queueIndices.graphics = findQueue(VK_QUEUE_GRAPHICS_BIT, surface);
    queueIndices.compute = findQueue(VK_QUEUE_COMPUTE_BIT);
    queueIndices.transfer = findQueue(VK_QUEUE_TRANSFER_BIT);*/
}

void Context::buildDevice() {
    /*vks::queues::DeviceCreateInfo deviceCreateInfo;
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
    device = physicalDevice.createDevice(deviceCreateInfo);*/

    std::set<std::string> requestedDeviceExtensions = deviceExtensionsPicker(physicalDevice);
    requestedDeviceExtensions.insert(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

    std::vector<const char*> enabledExtensions;
    for (const auto& extension : requestedDeviceExtensions) {
        enabledExtensions.push_back(extension.c_str());
    }

    Q_ASSERT(!device);
    device.reset(new VulkanDevice(physicalDevice));
    device->createLogicalDevice(device->enabledFeatures, enabledExtensions, nullptr, true,
                                VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
}

/*std::vector<VkCommandBuffer> Context::allocateCommandBuffers(
    uint32_t count,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
    std::vector<VkCommandBuffer> result;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.commandPool = getCommandPool();
    commandBufferAllocateInfo.commandBufferCount = count;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = device.allocateCommandBuffers(commandBufferAllocateInfo);
    return result;
}*/

/*VkCommandBuffer Context::createCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cmdBufAllocateInfo;
    cmdBufAllocateInfo.commandPool = getCommandPool();
    cmdBufAllocateInfo.level = level;
    cmdBufAllocateInfo.commandBufferCount = 1;
    cmdBuffer = device.allocateCommandBuffers(cmdBufAllocateInfo)[0];
    return cmdBuffer;
}*/

/*void Context::flushCommandBuffer(VkCommandBuffer& commandBuffer) const {
    if (!commandBuffer) {
        return;
    }
    queue.submit(VkSubmitInfo{ 0, nullptr, nullptr, 1, &commandBuffer }, VkFence());
    queue.waitIdle();
    device.waitIdle();
}*/

const VkCommandPool& Context::getCommandPool() const {
    return device->commandPool;
}

Image Context::createImage(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags& memoryPropertyFlags) const {
    Image result;
    result.device = device->logicalDevice;
    result.format = imageCreateInfo.format;
    result.extent = imageCreateInfo.extent;

#if VULKAN_USE_VMA
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = memoryPropertyFlags;
    auto pCreateInfo = &imageCreateInfo;
    auto pImage = &result.image;
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

Image Context::stageToDeviceImage(VkImageCreateInfo imageCreateInfo,
                         const VkMemoryPropertyFlags& memoryPropertyFlags,
                         VkDeviceSize size,
                         const void* data,
                         const std::vector<MipData>& mipData) const {
    Buffer staging = createStagingBuffer(size, data);
    imageCreateInfo.usage = imageCreateInfo.usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    Image result = createImage(imageCreateInfo, memoryPropertyFlags);

    withPrimaryCommandBuffer([&](const VkCommandBuffer& copyCmd) {
        VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, imageCreateInfo.mipLevels, 0, 1};
        // Prepare for transfer
        setImageLayout(copyCmd, result.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);

        // Prepare for transfer
        std::vector<VkBufferImageCopy> bufferCopyRegions;
        {
            VkBufferImageCopy bufferCopyRegion {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
        vkCmdCopyBufferToImage(copyCmd, staging.buffer, result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               bufferCopyRegions.size(), bufferCopyRegions.data());
        // Prepare for shader read
        setImageLayout(copyCmd, result.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       range);
    });
    staging.destroy();
    return result;
}

Buffer Context::createBuffer(const VkBufferUsageFlags& usageFlags,
                    VkDeviceSize size,
                    const VkMemoryPropertyFlags& memoryPropertyFlags) const {
    Buffer result;
    result.device = device->logicalDevice;
    result.size = size;
    result.descriptor.range = size;
    result.descriptor.offset = 0;

    VkBufferCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

#if VULKAN_USE_VMA
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = memoryPropertyFlags;
    auto pCreateInfo = &createInfo;
    auto pBuffer = &result.buffer;
    VK_CHECK_RESULT(vmaCreateBuffer(Allocation::getAllocator(), pCreateInfo, &allocInfo,
                    pBuffer, &result.allocation, nullptr));
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

Buffer Context::createDeviceBuffer(const VkBufferUsageFlags& usageFlags, VkDeviceSize size) const {
    static const VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    return createBuffer(usageFlags, size, memoryProperties);
}

Buffer Context::createStagingBuffer(VkDeviceSize size, const void* data) const {
    auto result = createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (data != nullptr) {
        result.map();
        memcpy(result.mapped, data, size);
        result.unmap();
    }
    return result;
}

Buffer Context::stageToDeviceBuffer(const VkBufferUsageFlags& usage, size_t size, const void* data) const {
    Buffer staging = createStagingBuffer(size, data);
    Buffer result = createDeviceBuffer(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
    withPrimaryCommandBuffer(
        [&](vk::CommandBuffer copyCmd) { copyCmd.copyBuffer(staging.buffer, result.buffer, vk::BufferCopy(0, 0, size)); });
    staging.destroy();
    return result;
}

/*vk::Bool32 Context::getMemoryType(uint32_t typeBits, const VkMemoryPropertyFlags& properties, uint32_t* typeIndex) const {
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
}*/

/*uint32_t Context::getMemoryType(uint32_t typeBits, const VkMemoryPropertyFlags& properties) const {
    uint32_t result = 0;
    if (!getMemoryType(typeBits, properties, &result)) {
        // todo : throw error
    }
    return result;
}*/

VkFormat Context::getSupportedDepthFormat() const {
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
                                           VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                           VK_FORMAT_D16_UNORM };

    for (auto& format : depthFormats) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice,format,&formatProperties);
        // VkFormat must support depth stencil attachment for optimal tiling
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }

    throw std::runtime_error("No supported depth format");
}
