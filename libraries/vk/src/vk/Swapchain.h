/*
* Class wrapping access to the swap chain
*
* A swap chain is a collection of framebuffers used for rendering
* The swap chain images can then presented to the windowing system
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Config.h"
#include <mutex>

namespace vks {
    struct SwapchainImage {
        vk::Image image;
        vk::ImageView view;
        vk::Fence fence;
    };

    struct Swapchain {
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        vk::PresentInfoKHR presentInfo;
        vk::Extent2D extent;
        const vks::Context& context{ vks::Context::get() };
        const vk::PhysicalDevice& physicalDevice{ context.physicalDevice };
        const vk::Device& device{ context.device };
        const uint32_t& graphicsQueueIndex{ context.queueIndices.graphics };
        const vk::Queue& queue{ context.queue };
        std::vector<SwapchainImage> images;
        vk::Format colorFormat;
        vk::ColorSpaceKHR colorSpace;
        uint32_t imageCount{ 0 };
        uint32_t currentImage{ 0 };

        Swapchain() {
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain;
            presentInfo.pImageIndices = &currentImage;
        }

        void waitIdle() {
            queue.waitIdle();
            device.waitIdle();
        }

        void setSurface(const vk::SurfaceKHR& newSurface) {
            surface = newSurface;
        }

        // Creates an os specific surface
        // Tries to find a graphics and a present queue
        void create(vk::Extent2D& size, bool vsync = false) {
            if (!physicalDevice || !device || !surface) {
                throw std::runtime_error("Initialize the physicalDevice, device, and queue members");
            }

            static std::once_flag once;
            std::call_once(once, [&] {
                // Get list of supported surface formats
                std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
                auto formatCount = surfaceFormats.size();

                physicalDevice.getSurfaceSupportKHR(graphicsQueueIndex, surface);


                // If the surface format list only includes one entry with  vk::Format::eUndefined,
                // there is no preferered format, so we assume  vk::Format::eB8G8R8A8Unorm
                if ((formatCount == 1) && (surfaceFormats[0].format == vk::Format::eUndefined)) {
                    colorFormat = vk::Format::eB8G8R8A8Unorm;
                } else {
                    // Always select the first available color format
                    // If you need a specific format (e.g. SRGB) you'd need to
                    // iterate over the list of available surface format and
                    // check for it's presence
                    colorFormat = surfaceFormats[0].format;
                }
                colorSpace = surfaceFormats[0].colorSpace;
            });


            vk::SwapchainKHR oldSwapchain = swapchain;
            currentImage = 0;

            // Get physical device surface properties and formats
            vk::SurfaceCapabilitiesKHR surfCaps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
            // Get available present modes
            std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
            auto presentModeCount = presentModes.size();

            // width and height are either both -1, or both not -1.
            if (surfCaps.currentExtent.width == -1) {
                // If the surface size is undefined, the size is set to
                // the size of the images requested.
                extent = size;
            } else {
                // If the surface size is defined, the swap chain size must match
                extent = surfCaps.currentExtent;
                size = surfCaps.currentExtent;
            }

            // Prefer mailbox mode if present, it's the lowest latency non-tearing present  mode
            vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

            if (!vsync) {
                for (size_t i = 0; i < presentModeCount; i++) {
                    if (presentModes[i] == vk::PresentModeKHR::eMailbox) {
                        swapchainPresentMode = vk::PresentModeKHR::eMailbox;
                        break;
                    }
                    if ((swapchainPresentMode != vk::PresentModeKHR::eMailbox) && (presentModes[i] == vk::PresentModeKHR::eImmediate)) {
                        swapchainPresentMode = vk::PresentModeKHR::eImmediate;
                    }
                }
            }

            // Determine the number of images
            uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
            if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount)) {
                desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
            }

            vk::SurfaceTransformFlagBitsKHR preTransform;
            if (surfCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
                preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
            } else {
                preTransform = surfCaps.currentTransform;
            }

            //auto imageFormat = context.physicalDevice.getImageFormatProperties(colorFormat, vk::ImageType::e2D, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageCreateFlags());
            vk::SwapchainCreateInfoKHR swapchainCI;
            swapchainCI.surface = surface;
            swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
            swapchainCI.imageFormat = colorFormat;
            swapchainCI.imageColorSpace = colorSpace;
            swapchainCI.imageExtent = extent;
            swapchainCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
            swapchainCI.preTransform = preTransform;
            swapchainCI.imageArrayLayers = 1;
            swapchainCI.imageSharingMode = vk::SharingMode::eExclusive;
            swapchainCI.queueFamilyIndexCount = 0;
            swapchainCI.pQueueFamilyIndices = NULL;
            swapchainCI.presentMode = swapchainPresentMode;
            swapchainCI.oldSwapchain = oldSwapchain;
            swapchainCI.clipped = true;
            swapchainCI.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

            swapchain = device.createSwapchainKHR(swapchainCI);


            // If an existing sawp chain is re-created, destroy the old swap chain
            // This also cleans up all the presentable images
            if (oldSwapchain) {
                for (uint32_t i = 0; i < imageCount; i++) {
                    device.destroyImageView(images[i].view);
                }
                device.destroySwapchainKHR(oldSwapchain);
            }

            vk::ImageViewCreateInfo colorAttachmentView;
            colorAttachmentView.format = colorFormat;
            colorAttachmentView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            colorAttachmentView.subresourceRange.levelCount = 1;
            colorAttachmentView.subresourceRange.layerCount = 1;
            colorAttachmentView.viewType = vk::ImageViewType::e2D;

            // Get the swap chain images
            auto swapChainImages = device.getSwapchainImagesKHR(swapchain);
            imageCount = (uint32_t)swapChainImages.size();

            // Get the swap chain buffers containing the image and imageview
            images.resize(imageCount);
            for (uint32_t i = 0; i < imageCount; i++) {
                images[i].image = swapChainImages[i];
                colorAttachmentView.image = swapChainImages[i];
                images[i].view = device.createImageView(colorAttachmentView);
                images[i].fence = vk::Fence();
            }
        }

        std::vector<vk::Framebuffer> createFramebuffers(vk::FramebufferCreateInfo framebufferCreateInfo) {
            // Verify that the first attachment is null
            assert(framebufferCreateInfo.pAttachments[0] == vk::ImageView());


            std::vector<vk::ImageView> attachments;
            attachments.resize(framebufferCreateInfo.attachmentCount);
            for (size_t i = 0; i < framebufferCreateInfo.attachmentCount; ++i) {
                attachments[i] = framebufferCreateInfo.pAttachments[i];
            }
            framebufferCreateInfo.pAttachments = attachments.data();

            std::vector<vk::Framebuffer> framebuffers;
            framebuffers.resize(imageCount);
            for (uint32_t i = 0; i < imageCount; i++) {
                attachments[0] = images[i].view;
                framebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
            }
            return framebuffers;
        }

        // Acquires the next image in the swap chain
        vk::ResultValue<uint32_t> acquireNextImage(const vk::Semaphore& presentCompleteSemaphore, const vk::Fence& fence = vk::Fence()) {
            auto resultValue = device.acquireNextImageKHR(swapchain, UINT64_MAX, presentCompleteSemaphore, fence);
            vk::Result result = resultValue.result;
            if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
                throw std::error_code(result);
            } 
            currentImage = resultValue.value;
            return resultValue;
        }

        void clearSubmitFence(uint32_t index) {
            images[index].fence = vk::Fence();
        }

        vk::Fence getSubmitFence(bool destroy = false) {
            auto& image = images[currentImage];
            while (image.fence) {
                vk::Result fenceRes = device.waitForFences(image.fence, VK_TRUE, UINT64_MAX);
                if (fenceRes == vk::Result::eSuccess) {
                    if (destroy) {
                        device.destroyFence(image.fence);
                    }
                    image.fence = vk::Fence();
                }
            }

            image.fence = device.createFence({});
            return image.fence;
        }

        // Present the current image to the queue
        vk::Result queuePresent(vk::Semaphore waitSemaphore) {
            presentInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
            presentInfo.pWaitSemaphores = &waitSemaphore;
            return queue.presentKHR(presentInfo);
        }

        // Free all Vulkan resources used by the swap chain
        void destroy() {
            for (uint32_t i = 0; i < imageCount; i++) {
                device.destroyImageView(images[i].view);
            }
            device.destroySwapchainKHR(swapchain);
        }

    private:
        uint32_t findQueue(const vk::QueueFlags& flags) const {
            std::vector<vk::QueueFamilyProperties> queueProps = physicalDevice.getQueueFamilyProperties();
            size_t queueCount = queueProps.size();
            for (uint32_t i = 0; i < queueCount; i++) {
                if (queueProps[i].queueFlags & flags) {
                    if (surface && !physicalDevice.getSurfaceSupportKHR(i, surface)) {
                        continue;
                    }
                    return i;
                }
            }
            throw std::runtime_error("No queue matches the flags " + vk::to_string(flags));
        }
    };

    using SwapchainPtr = std::shared_ptr<Swapchain>;
}

#if 0
#ifdef __ANDROID__
vk::AndroidSurfaceCreateInfoKHR surfaceCreateInfo;
surfaceCreateInfo.window = window;
surface = context.instance.createAndroidSurfaceKHR(surfaceCreateInfo);
#else
surface = glfw::createWindowSurface(context.instance, window);
#endif
#endif
