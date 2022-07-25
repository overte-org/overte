#pragma once

#include "Config.h"

#include "Context.h"
#include "Buffer.h"
#include "Image.h"

namespace vks { namespace texture {

    /** @brief Vulkan texture base class */
    class Texture : public vks::Image {
        using Parent = vks::Image;
    public:
        vk::Device device;
        vk::ImageLayout imageLayout;
        uint32_t mipLevels;
        uint32_t layerCount{ 1 };
        vk::DescriptorImageInfo descriptor;

        Texture& operator=(const vks::Image& image) {
            destroy();
            (vks::Image&)*this = image;
            return *this;
        }

        /** @brief Update image descriptor from current sampler, view and image layout */
        void updateDescriptor() {
            descriptor.sampler = sampler;
            descriptor.imageView = view;
            descriptor.imageLayout = imageLayout;
        }

        /** @brief Release all Vulkan resources held by this texture */
        void destroy() {
            Parent::destroy();
        }
    };

    /** @brief 2D texture */
    class Texture2D : public Texture {
        using Parent = Texture;
    public:
        /**
        * Load a 2D texture including all mip levels
        *
        * @param filename File to load (supports .ktx and .dds)
        * @param format Vulkan format of the image data stored in the file
        * @param device Vulkan device to create the texture on
        * @param copyQueue Queue used for the texture staging copy commands (must support transfer)
        * @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
        * @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        * @param (Optional) forceLinear Force linear tiling (not advised, defaults to false)
        *
        */
        void loadFromFile(
            const vks::Context& context,
            const std::string& filename, 
            vk::Format format,
            vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
            vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal, 
            bool forceLinear = false)
        {
            this->imageLayout = imageLayout;
            std::shared_ptr<gli::texture2d> tex2Dptr;
            vks::file::withBinaryFileContents(filename, [&](size_t size, const void* data) {
                tex2Dptr = std::make_shared<gli::texture2d>(gli::load((const char*)data, size));
            });
            const auto& tex2D = *tex2Dptr;
            assert(!tex2D.empty());

            device = context.device;
            extent.width = static_cast<uint32_t>(tex2D[0].extent().x);
            extent.height = static_cast<uint32_t>(tex2D[0].extent().y);
            extent.depth = 1;
            mipLevels = static_cast<uint32_t>(tex2D.levels());
            layerCount = 1;
            
            // Create optimal tiled target image
            vk::ImageCreateInfo imageCreateInfo;
            imageCreateInfo.imageType = vk::ImageType::e2D;
            imageCreateInfo.format = format;
            imageCreateInfo.mipLevels = mipLevels;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.extent = extent;
            imageCreateInfo.usage = imageUsageFlags | vk::ImageUsageFlagBits::eTransferDst;

#if 1
            ((vks::Image&)(*this)) = context.stageToDeviceImage(imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, tex2D);
            
#else
            ((vks::Image&)*this) = context.createImage(imageCreateInfo);
            auto stagingBuffer = context.createBuffer(vk::BufferUsageFlagBits::eTransferSrc, tex2D);

            // Setup buffer copy regions for each layer including all of it's miplevels
            std::vector<vk::BufferImageCopy> bufferCopyRegions;
            size_t offset = 0;
            vk::BufferImageCopy bufferCopyRegion;
            bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.depth = 1;
            for (uint32_t level = 0; level < mipLevels; level++) {
                auto image = tex2D[level];
                auto imageExtent = image.extent();
                bufferCopyRegion.imageSubresource.mipLevel = level;
                bufferCopyRegion.imageSubresource.baseArrayLayer = 1;
                bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(imageExtent.x);
                bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(imageExtent.y);
                bufferCopyRegion.bufferOffset = offset;
                bufferCopyRegions.push_back(bufferCopyRegion);
                // Increase offset into staging buffer for next level / face
                offset += image.size();
            }



            vk::ImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            subresourceRange.levelCount = mipLevels;
            subresourceRange.layerCount = layerCount;

            // Use a separate command buffer for texture loading
            context.withPrimaryCommandBuffer([&](const vk::CommandBuffer& copyCmd) {
                // Image barrier for optimal image (target)
                // Set initial layout for all array layers (faces) of the optimal (target) tiled texture
                context.setImageLayout(copyCmd, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);
                // Copy the layers and mip levels from the staging buffer to the optimal tiled image
                copyCmd.copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
                // Change texture image layout to shader read after all faces have been copied
                context.setImageLayout(copyCmd, image, vk::ImageLayout::eTransferDstOptimal, imageLayout, subresourceRange);
            });

            // Clean up staging resources
            stagingBuffer.destroy();
#endif


            // Create sampler
            vk::SamplerCreateInfo samplerCreateInfo;
            samplerCreateInfo.magFilter = vk::Filter::eLinear;
            samplerCreateInfo.minFilter = vk::Filter::eLinear;
            samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            // Max level-of-detail should match mip level count
            samplerCreateInfo.maxLod = (float)mipLevels;
            // Only enable anisotropic filtering if enabled on the devicec
            samplerCreateInfo.maxAnisotropy = context.deviceFeatures.samplerAnisotropy ? context.deviceProperties.limits.maxSamplerAnisotropy : 1.0f;
            samplerCreateInfo.anisotropyEnable = context.deviceFeatures.samplerAnisotropy;
            samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            sampler = device.createSampler(samplerCreateInfo);

            // Create image view
            vk::ImageViewCreateInfo viewCreateInfo;
            viewCreateInfo.viewType = vk::ImageViewType::e2D;
            viewCreateInfo.image = image;
            viewCreateInfo.format = format;
            viewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, layerCount };
            view = context.device.createImageView(viewCreateInfo);

            // Update descriptor image info member that can be used for setting up descriptor sets
            updateDescriptor();
        }

#if 0
        /**
        * Creates a 2D texture from a buffer
        *
        * @param buffer Buffer containing texture data to upload
        * @param bufferSize Size of the buffer in machine units
        * @param width Width of the texture to create
        * @param height Height of the texture to create
        * @param format Vulkan format of the image data stored in the file
        * @param device Vulkan device to create the texture on
        * @param copyQueue Queue used for the texture staging copy commands (must support transfer)
        * @param (Optional) filter Texture filtering for the sampler (defaults to VK_FILTER_LINEAR)
        * @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
        * @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        */
        void fromBuffer(
            void* buffer,
            VkDeviceSize bufferSize,
            VkFormat format,
            uint32_t width,
            uint32_t height,
            VkQueue copyQueue,
            VkFilter filter = VK_FILTER_LINEAR,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            assert(buffer);

            this->device = device;
            width = width;
            height = height;
            mipLevels = 1;

            VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
            VkMemoryRequirements memReqs;

            // Use a separate command buffer for texture loading
            VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

            // Create a host-visible staging buffer that contains the raw image data
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
            bufferCreateInfo.size = bufferSize;
            // This buffer is used as a transfer source for the buffer copy
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

            // Get memory requirements for the staging buffer (alignment, memory type bits)
            vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);

            memAllocInfo.allocationSize = memReqs.size;
            // Get memory type index for a host visible buffer
            memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
            VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

            // Copy texture data into staging buffer
            uint8_t *data;
            VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void **)&data));
            memcpy(data, buffer, bufferSize);
            vkUnmapMemory(device->logicalDevice, stagingMemory);

            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = width;
            bufferCopyRegion.imageExtent.height = height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = 0;

            // Create optimal tiled target image
            VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.mipLevels = mipLevels;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.extent = { width, height, 1 };
            imageCreateInfo.usage = imageUsageFlags;
            // Ensure that the TRANSFER_DST bit is set for staging
            if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
            {
                imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &image));

            vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);

            memAllocInfo.allocationSize = memReqs.size;

            memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &deviceMemory));
            VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));

            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = mipLevels;
            subresourceRange.layerCount = 1;

            // Image barrier for optimal image (target)
            // Optimal image will be used as destination for the copy
            vks::tools::setImageLayout(
                copyCmd,
                image,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                subresourceRange);

            // Copy mip levels from staging buffer
            vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion
            );

            // Change texture image layout to shader read after all mip levels have been copied
            this->imageLayout = imageLayout;
            vks::tools::setImageLayout(
                copyCmd,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                imageLayout,
                subresourceRange);

            device->flushCommandBuffer(copyCmd, copyQueue);

            // Clean up staging resources
            vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
            vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);

            // Create sampler
            VkSamplerCreateInfo samplerCreateInfo = {};
            samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.magFilter = filter;
            samplerCreateInfo.minFilter = filter;
            samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.mipLodBias = 0.0f;
            samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
            samplerCreateInfo.minLod = 0.0f;
            samplerCreateInfo.maxLod = 0.0f;
            samplerCreateInfo.maxAnisotropy = 1.0f;
            VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &sampler));

            // Create image view
            VkImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.pNext = NULL;
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewCreateInfo.format = format;
            viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.image = image;
            VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &view));

            // Update descriptor image info member that can be used for setting up descriptor sets
            updateDescriptor();
        }
#endif
    };

    /** @brief 2D array texture */
    class Texture2DArray : public Texture {
    public:
        /**
        * Load a 2D texture array including all mip levels
        *
        * @param filename File to load (supports .ktx and .dds)
        * @param format Vulkan format of the image data stored in the file
        * @param device Vulkan device to create the texture on
        * @param copyQueue Queue used for the texture staging copy commands (must support transfer)
        * @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
        * @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        *
        */
        void loadFromFile(
            const vks::Context& context,
            std::string filename,
            vk::Format format,
            vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
            vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal) {
            this->device = device;

            std::shared_ptr<gli::texture2d_array> texPtr;
            vks::file::withBinaryFileContents(filename, [&](size_t size, const void* data) {
                texPtr = std::make_shared<gli::texture2d_array>(gli::load((const char*)data, size));
            });

            const gli::texture2d_array& tex2DArray = *texPtr; 

            extent.width = static_cast<uint32_t>(tex2DArray.extent().x);
            extent.height = static_cast<uint32_t>(tex2DArray.extent().y);
            extent.depth = 1;
            layerCount = static_cast<uint32_t>(tex2DArray.layers());
            mipLevels = static_cast<uint32_t>(tex2DArray.levels());

            auto stagingBuffer = context.createStagingBuffer(tex2DArray);

            // Setup buffer copy regions for each layer including all of it's miplevels
            std::vector<vk::BufferImageCopy> bufferCopyRegions;
            size_t offset = 0;
            vk::BufferImageCopy bufferCopyRegion;
            bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.depth = 1;
            for (uint32_t layer = 0; layer < layerCount; layer++) {
                for (uint32_t level = 0; level < mipLevels; level++) {
                    auto image = tex2DArray[layer][level];
                    auto imageExtent = image.extent();
                    bufferCopyRegion.imageSubresource.mipLevel = level;
                    bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
                    bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(imageExtent.x);
                    bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(imageExtent.y);
                    bufferCopyRegion.bufferOffset = offset;
                    bufferCopyRegions.push_back(bufferCopyRegion);
                    // Increase offset into staging buffer for next level / face
                    offset += image.size();
                }
            }

            
            // Create optimal tiled target image
            vk::ImageCreateInfo imageCreateInfo;
            imageCreateInfo.imageType = vk::ImageType::e2D;
            imageCreateInfo.format = format;
            imageCreateInfo.extent = extent;
            imageCreateInfo.usage = imageUsageFlags | vk::ImageUsageFlagBits::eTransferDst;
            imageCreateInfo.arrayLayers = layerCount;
            imageCreateInfo.mipLevels = mipLevels;
            ((vks::Image&)*this) = context.createImage(imageCreateInfo);

            vk::ImageSubresourceRange subresourceRange;
            subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            subresourceRange.levelCount = mipLevels;
            subresourceRange.layerCount = layerCount;

            // Use a separate command buffer for texture loading
            context.withPrimaryCommandBuffer([&](const vk::CommandBuffer& copyCmd) {
                // Image barrier for optimal image (target)
                // Set initial layout for all array layers (faces) of the optimal (target) tiled texture
                context.setImageLayout(copyCmd, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);
                // Copy the layers and mip levels from the staging buffer to the optimal tiled image
                copyCmd.copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
                // Change texture image layout to shader read after all faces have been copied
                context.setImageLayout(copyCmd, image, vk::ImageLayout::eTransferDstOptimal, imageLayout, subresourceRange);
            });

            // Clean up staging resources
            stagingBuffer.destroy();

            // Create sampler
            vk::SamplerCreateInfo samplerCreateInfo;
            samplerCreateInfo.magFilter = vk::Filter::eLinear;
            samplerCreateInfo.minFilter = vk::Filter::eLinear;
            samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
            samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
            samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
            samplerCreateInfo.maxAnisotropy = context.deviceFeatures.samplerAnisotropy ? context.deviceProperties.limits.maxSamplerAnisotropy : 1.0f;
            samplerCreateInfo.maxLod = (float)mipLevels;
            samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            sampler = context.device.createSampler(samplerCreateInfo);

            // Create image view
            vk::ImageViewCreateInfo viewCreateInfo;
            viewCreateInfo.viewType = vk::ImageViewType::e2DArray;
            viewCreateInfo.image = image;
            viewCreateInfo.format = format;
            viewCreateInfo.subresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, layerCount };
            view = context.device.createImageView(viewCreateInfo);

            // Update descriptor image info member that can be used for setting up descriptor sets
            updateDescriptor();
        }
    };

    /** @brief Cube map texture */
    class TextureCubeMap : public Texture {
    public:
        /**
        * Load a cubemap texture including all mip levels from a single file
        *
        * @param filename File to load (supports .ktx and .dds)
        * @param format Vulkan format of the image data stored in the file
        * @param device Vulkan device to create the texture on
        * @param copyQueue Queue used for the texture staging copy commands (must support transfer)
        * @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
        * @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        *
        */
        void loadFromFile(
            const vks::Context& context,
            const std::string& filename,
            vk::Format format,
            vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
            vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal) {
            device = context.device;

            std::shared_ptr<const gli::texture_cube> texPtr; 
            vks::file::withBinaryFileContents(filename, [&](size_t size, const void* data) {
                texPtr = std::make_shared<const gli::texture_cube>(gli::load((const char*)data, size));
            });
            const auto& texCube = *texPtr;
            assert(!texCube.empty());

            
            extent.width = static_cast<uint32_t>(texCube.extent().x);
            extent.height = static_cast<uint32_t>(texCube.extent().y);
            extent.depth = 1;
            mipLevels = static_cast<uint32_t>(texCube.levels());
            auto stagingBuffer = context.createStagingBuffer(texCube);

            // Setup buffer copy regions for each face including all of it's miplevels
            std::vector<vk::BufferImageCopy> bufferCopyRegions;
            size_t offset = 0;
            vk::BufferImageCopy bufferImageCopy;
            bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferImageCopy.imageSubresource.layerCount = 1;
            bufferImageCopy.imageExtent.depth = 1;
            for (uint32_t face = 0; face < 6; face++) {
                for (uint32_t level = 0; level < mipLevels; level++) {
                    auto image = (texCube)[face][level];
                    auto imageExtent = image.extent();
                    bufferImageCopy.bufferOffset = offset;
                    bufferImageCopy.imageSubresource.mipLevel = level;
                    bufferImageCopy.imageSubresource.baseArrayLayer = face;
                    bufferImageCopy.imageExtent.width = (uint32_t)imageExtent.x;
                    bufferImageCopy.imageExtent.height = (uint32_t)imageExtent.y;
                    bufferCopyRegions.push_back(bufferImageCopy);
                    // Increase offset into staging buffer for next level / face
                    offset += image.size();
                }
            }

            // Create optimal tiled target image
            vk::ImageCreateInfo imageCreateInfo;
            imageCreateInfo.imageType = vk::ImageType::e2D;
            imageCreateInfo.format = format;
            imageCreateInfo.mipLevels = mipLevels;
            imageCreateInfo.extent = extent;
            // Cube faces count as array layers in Vulkan
            imageCreateInfo.arrayLayers = 6;
            // Ensure that the TRANSFER_DST bit is set for staging
            imageCreateInfo.usage = imageUsageFlags | vk::ImageUsageFlagBits::eTransferDst;
            // This flag is required for cube map images
            imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
            ((vks::Image&)*this) = context.createImage(imageCreateInfo);

            context.withPrimaryCommandBuffer([&](const vk::CommandBuffer& copyCmd) {
                // Image barrier for optimal image (target)
                // Set initial layout for all array layers (faces) of the optimal (target) tiled texture
                vk::ImageSubresourceRange subresourceRange { vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 6 };
                context.setImageLayout(copyCmd, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);
                // Copy the cube map faces from the staging buffer to the optimal tiled image
                copyCmd.copyBufferToImage(stagingBuffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
                // Change texture image layout to shader read after all faces have been copied
                this->imageLayout = imageLayout;
                context.setImageLayout(copyCmd, image, vk::ImageLayout::eTransferDstOptimal, imageLayout, subresourceRange);
            });


            // Create sampler
            // Create a defaultsampler
            vk::SamplerCreateInfo samplerCreateInfo;
            samplerCreateInfo.magFilter = vk::Filter::eLinear;
            samplerCreateInfo.minFilter = vk::Filter::eLinear;
            samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
            samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
            samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
            // Max level-of-detail should match mip level count
            samplerCreateInfo.maxLod = (float)mipLevels;
            // Only enable anisotropic filtering if enabled on the devicec
            samplerCreateInfo.maxAnisotropy = context.deviceFeatures.samplerAnisotropy ? context.deviceProperties.limits.maxSamplerAnisotropy : 1.0f;
            samplerCreateInfo.anisotropyEnable = context.deviceFeatures.samplerAnisotropy;
            samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            sampler = device.createSampler(samplerCreateInfo);

            // Create image view
            // Textures are not directly accessed by the shaders and
            // are abstracted by image views containing additional
            // information and sub resource ranges
            view = device.createImageView(vk::ImageViewCreateInfo{
                {}, image, vk::ImageViewType::eCube, format,{}, 
                vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 6 }
                });
            stagingBuffer.destroy();

            // Update descriptor image info member that can be used for setting up descriptor sets
            updateDescriptor();
        }
    };
} }
