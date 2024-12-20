// Based on Vulkan samples.
// TODO: add copyright header

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include "Context.h"

using namespace vks;

// Start of VKS code

Context& Context::get() {
    static Context INSTANCE;
    return INSTANCE;
}

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
    if (device) {
        throw std::runtime_error("Vulkan device already exists");
    }

    if (isExtensionPresent(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        requireExtensions({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME });
        enableValidation = true;
        enableDebugMarkers = true;
        qDebug() << "Found debug marker extension";
    }

    //requireExtensions({ VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME });
    //requireExtensions({ VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME });

    // Vulkan instance
    VkApplicationInfo appInfo{};
    appInfo.pApplicationName = "VulkanExamples";
    appInfo.pEngineName = "VulkanExamples";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    std::set<std::string> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

// Enable surface extensions depending on OS
#if defined(_WIN32)
    instanceExtensions.insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    instanceExtensions.insert(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
    instanceExtensions.insert(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    instanceExtensions.insert(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    instanceExtensions.insert(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    instanceExtensions.insert(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    instanceExtensions.insert(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    instanceExtensions.insert(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    instanceExtensions.insert(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
    instanceExtensions.insert(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    instanceExtensions.insert(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME);
#endif

    instanceExtensions.insert(requiredExtensions.begin(), requiredExtensions.end());

    std::vector<const char*> enabledExtensions;
    for (const auto& extension : instanceExtensions) {
        enabledExtensions.push_back(extension.c_str());
    }

    // Enable surface extensions depending on os
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enabledExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }

    CStringVector layers;
    if (enableValidation) {
        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
        std::list<std::string> validationLayers {validationLayerName}; // Just one layer for now
        layers = filterLayers(validationLayers);
        if (layers.size() == 1) {
            instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
            instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
        } else {
            qWarning() << "Cannot find VK_LAYER_KHRONOS_validation layer, validation not enabled";
        }
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
    VK_CHECK_RESULT(vkQueueWaitIdle(graphicsQueue));

    device.reset();
    if (enableValidation) {
        debug::freeDebugCallback(instance);
    }
    vkDestroyInstance(instance, nullptr);
}

void Context::createDevice() {
    pickDevice();

    buildDevice();

#if VULKAN_USE_VMA
    vks::Allocation::initAllocator(physicalDevice, device->logicalDevice);
#endif

    // Get the graphics queue
    vkGetDeviceQueue(device->logicalDevice, device->queueFamilyIndices.graphics, 0, &graphicsQueue);
    vkGetDeviceQueue(device->logicalDevice, device->queueFamilyIndices.transfer, 0, &transferQueue);
}

void Context::pickDevice() {
    // Physical device
    uint32_t physicalDeviceCount;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    physicalDevices.resize(physicalDeviceCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    // Note :
    // This example will always use the first physical device reported,
    // change the vector index if you have multiple Vulkan devices installed
    // and want to use another one
    physicalDevice = physicalDevices[0];
}

void Context::buildDevice() {
    std::set<std::string> requestedDeviceExtensions = {};
    requestedDeviceExtensions.insert(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

    std::vector<const char*> enabledExtensions;
    for (const auto& extension : requestedDeviceExtensions) {
        enabledExtensions.push_back(extension.c_str());
    }
    enabledExtensions.push_back(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME);

    // Needed for OpenGL depth buffer compatibility
    VkPhysicalDeviceDepthClipControlFeaturesEXT depthClipControl{};
    depthClipControl.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;
    depthClipControl.depthClipControl = true;

    void *pNextChain = &depthClipControl;

    enabledFeatures.depthClamp = true;

    Q_ASSERT(!device);
    device.reset(new VulkanDevice(physicalDevice));
    device->createLogicalDevice(enabledFeatures, enabledExtensions, pNextChain, true,
                                VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
}

VkCommandBuffer Context::createCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level) const {
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, level, 1);
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device->logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
    return cmdBuffer;
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
    VmaAllocationCreateInfo allocationCI = {};
    //allocationCI.requiredFlags = memoryPropertyFlags; //VKTODO: this will replace allocationCI.usage probably
    allocationCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // VKTODO: different kind will be required later for device-local memory
    allocationCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    auto pCreateInfo = &createInfo;
    auto pBuffer = &result.buffer;
    VK_CHECK_RESULT(vmaCreateBuffer(Allocation::getAllocator(), pCreateInfo, &allocationCI,
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

// End of VKS code

void Context::Recycler::trashVkSampler(VkSampler& sampler) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkSamplers.push_back(sampler);
}

void Context::Recycler::trashVkFramebuffer(VkFramebuffer& framebuffer) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkFramebuffer.push_back(framebuffer);
}

void Context::Recycler::trashVkImageView(VkImageView& imageView) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkImageViews.push_back(imageView);
}

void Context::Recycler::trashVkImage(VkImage& image) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkImages.push_back(image);
}

void Context::Recycler::trashVkBuffer(VkBuffer& buffer) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkBuffers.push_back(buffer);
}

void Context::Recycler::trashVkRenderPass(VkRenderPass& renderPass) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkRenderPasses.push_back(renderPass);
}

void Context::Recycler::trashVkPipeline(VkPipeline& pipeline) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkPipelines.push_back(pipeline);
}

void Context::Recycler::trashVkShaderModule(VkShaderModule& module) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkShaderModules.push_back(module);
}

void Context::Recycler::trashVkSwapchainKHR(VkSwapchainKHR& swapchain) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    vkSwapchainsKHR.push_back(swapchain);
}

void Context::Recycler::trashVKSurfaceKHR(VkSurfaceKHR& surface) {
    vkSurfacesKHR.push_back(surface);
}

void Context::Recycler::trashVmaAllocation(VmaAllocation& allocation) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
}

void Context::Recycler::framebufferDeleted(gpu::vk::VKFramebuffer* framebuffer) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    deletedFramebuffers.push_back(framebuffer);
}

void Context::Recycler::bufferDeleted(gpu::vk::VKBuffer* buffer) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    deletedBuffers.push_back(buffer);
}

void Context::Recycler::textureDeleted(gpu::vk::VKTexture* texture) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    deletedTextures.push_back(texture);
}

void Context::Recycler::queryDeleted(gpu::vk::VKQuery* query) {
    std::lock_guard<std::recursive_mutex> lockGuard(recyclerMutex);
    deletedQueries.push_back(query);
}
