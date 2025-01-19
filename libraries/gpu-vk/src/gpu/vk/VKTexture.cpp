//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "VKTexture.h"

#include <QtCore/QThread>
#include <NumericalConstants.h>
#include <gl/GLHelpers.h>

#include "VKBackend.h"
#include "vk/Allocation.h"

using namespace gpu;
using namespace gpu::vk;


/*const uint32_t VKTexture::CUBE_FACE_LAYOUT[VKTexture::TEXTURE_CUBE_NUM_FACES] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};*/

const uint32_t VKTexture::WRAP_MODES[Sampler::NUM_WRAP_MODES] = {
    VK_SAMPLER_ADDRESS_MODE_REPEAT,                         // WRAP_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,                // WRAP_MIRROR,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,                  // WRAP_CLAMP,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,                // WRAP_BORDER,
    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE        // WRAP_MIRROR_ONCE,
};

const VKFilterMode VKTexture::FILTER_MODES[Sampler::NUM_FILTERS] = {
    { VK_FILTER_NEAREST, VK_FILTER_NEAREST },  //FILTER_MIN_MAG_POINT,
    { VK_FILTER_NEAREST, VK_FILTER_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR,
    { VK_FILTER_LINEAR, VK_FILTER_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR },  //FILTER_MIN_MAG_LINEAR,

    // VKTODO: VkSamplerMipmapMode has some related setting too
    // VKTODO: This is not really accurate, but hopefully good enough for now
    { VK_FILTER_NEAREST, VK_FILTER_NEAREST },  //FILTER_MIN_MAG_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_NEAREST },  //FILTER_MIN_MAG_POINT_MIP_LINEAR,
    { VK_FILTER_NEAREST, VK_FILTER_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR },  //FILTER_MIN_POINT_MAG_MIP_LINEAR,
    { VK_FILTER_NEAREST, VK_FILTER_NEAREST },  //FILTER_MIN_LINEAR_MAG_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    { VK_FILTER_NEAREST, VK_FILTER_LINEAR },  //FILTER_MIN_MAG_LINEAR_MIP_POINT,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR },  //FILTER_MIN_MAG_MIP_LINEAR,
    { VK_FILTER_LINEAR, VK_FILTER_LINEAR }  //FILTER_ANISOTROPIC,
};

static constexpr size_t MAX_PIXEL_BYTE_SIZE{ 4 };
static constexpr size_t MAX_TRANSFER_DIMENSION{ 1024 };

/*const uvec3 GLVariableAllocationSupport::MAX_TRANSFER_DIMENSIONS{ MAX_TRANSFER_DIMENSION, MAX_TRANSFER_DIMENSION, 1 };
const uvec3 GLVariableAllocationSupport::INITIAL_MIP_TRANSFER_DIMENSIONS{ 64, 64, 1 };
const size_t GLVariableAllocationSupport::MAX_TRANSFER_SIZE = MAX_TRANSFER_DIMENSION * MAX_TRANSFER_DIMENSION * MAX_PIXEL_BYTE_SIZE;
const size_t GLVariableAllocationSupport::MAX_BUFFER_SIZE = MAX_TRANSFER_SIZE;*/

/*GLenum VKTexture::getVKTextureType(const Texture& texture) {
    switch (texture.getType()) {
        case Texture::TEX_2D:
            if (!texture.isArray()) {
                if (!texture.isMultisample()) {
                    return GL_TEXTURE_2D;
                } else {
                    return GL_TEXTURE_2D_MULTISAMPLE;
                }
            } else {
                if (!texture.isMultisample()) {
                    return GL_TEXTURE_2D_ARRAY;
                } else {
                    return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
                }
            }
            break;

        case Texture::TEX_CUBE:
            return GL_TEXTURE_CUBE_MAP;
            break;

        default:
            qFatal("Unsupported texture type");
    }
    Q_UNREACHABLE();
    return GL_TEXTURE_2D;
}


uint8_t VKTexture::getFaceCount(GLenum target) {
    switch (target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            return TEXTURE_2D_NUM_FACES;
        case GL_TEXTURE_CUBE_MAP:
            return TEXTURE_CUBE_NUM_FACES;
        default:
            Q_UNREACHABLE();
            break;
    }
}
const std::vector<GLenum>& VKTexture::getFaceTargets(GLenum target) {
    static const std::vector<GLenum> cubeFaceTargets {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    static const std::vector<GLenum> face2DTargets {
        GL_TEXTURE_2D
    };
    static const std::vector<GLenum> face2DMSTargets{
        GL_TEXTURE_2D_MULTISAMPLE
    };
    static const std::vector<GLenum> arrayFaceTargets{
        GL_TEXTURE_2D_ARRAY
    };
    switch (target) {
        case GL_TEXTURE_2D:
            return face2DTargets;
        case GL_TEXTURE_2D_MULTISAMPLE:
            return face2DMSTargets;
        case GL_TEXTURE_2D_ARRAY:
            return arrayFaceTargets;
        case GL_TEXTURE_CUBE_MAP:
            return cubeFaceTargets;
        default:
            Q_UNREACHABLE();
            break;
    }
    Q_UNREACHABLE();
    return face2DTargets;
}*/

VKTexture::VKTexture(const std::weak_ptr<VKBackend>& backend, const Texture& texture, bool isTransferable) :
    VKObject(*backend.lock(), texture),
    _storageStamp(texture.getStamp()),
    _target(getVKTextureType(texture)),
    _transferable(isTransferable),
    _downsampleSource(backend),
    _vkTexelFormat(evalTexelFormatInternal(texture.getTexelFormat()))
{
    Backend::setGPUObject(texture, this);
}

VKTexture::~VKTexture() {
    qDebug() << "VKTexture destroyed: " << this;
    auto backend = _backend.lock();
    backend->getContext().recycler.textureDeleted(this);
    if (backend && _vkImage == VK_NULL_HANDLE) {
        // VKTODO
        // backend->releaseTexture(_id, 0);
    }
}

VkImageViewType VKTexture::getVKTextureType(const Texture& texture) {
    switch (texture.getType()) {
        case Texture::TEX_2D:
            if (!texture.isArray()) {
                if (!texture.isMultisample()) {
                    return VK_IMAGE_VIEW_TYPE_2D;
                } else {
                    // VKTODO: multisample?
                    return VK_IMAGE_VIEW_TYPE_2D;
                }
            } else {
                if (!texture.isMultisample()) {
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                } else {
                    // VKTODO multisample?
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                }
            }
            break;

        case Texture::TEX_CUBE:
            return VK_IMAGE_VIEW_TYPE_CUBE;
            break;

        default:
            qFatal("Unsupported texture type");
    }
    Q_UNREACHABLE();
    return VK_IMAGE_VIEW_TYPE_2D;
}

// From VKS
void VKAttachmentTexture::createTexture(VKBackend &backend) {
    VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    imageCI.extent.width = _gpuObject.getWidth();
    imageCI.extent.height = _gpuObject.getHeight();
    imageCI.extent.depth = 1;
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = _gpuObject.isArray() ? _gpuObject.getNumSlices() : 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    /*if (_gpuObject.isColorRenderTarget()
        || _gpuObject.getTexelFormat().getSemantic() == gpu::R11G11B10
        || _gpuObject.getTexelFormat().getSemantic() == gpu::SRGB
        || _gpuObject.getTexelFormat().getSemantic() == gpu::SRGBA) {*/
    if (_gpuObject.isDepthStencilRenderTarget()) {
        imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    } else {
        imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    // Create image for this attachment
    /*VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &_texture));
    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, _texture, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = _backend.lock()->getContext().device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &_vkDeviceMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, _texture, _vkDeviceMemory, 0));*/

    VmaAllocationCreateInfo allocationCI = {};
    allocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(vks::Allocation::getAllocator(), &imageCI, &allocationCI, &_vkImage, &_vmaAllocation, nullptr);

    /*attachment.subresourceRange = {};
    attachment.subresourceRange.aspectMask = aspectMask;
    attachment.subresourceRange.levelCount = 1;
    attachment.subresourceRange.layerCount = createinfo.layerCount;*/

    /*VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
    imageView.viewType = (_gpuObject.isArray()) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    imageView.format = imageCI.format;
    // VKTODO: what is subresource?
    imageView.subresourceRange = attachment.subresourceRange;
    imageView.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
    imageView.image = attachment.imageCI;
    VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr, &attachment.view));*/

}

VKAttachmentTexture::~VKAttachmentTexture() {
    // VKTODO: Redo destructors for cleanup to happen on present thread
    auto backend = _backend.lock();
    auto device = backend->getContext().device->logicalDevice;
    auto &recycler = backend->getContext().recycler;
    if (_vkImageView) {
        recycler.trashVkImageView(_vkImageView);
    }
    if (_vkSampler) {
        recycler.trashVkSampler(_vkSampler);
    }
    recycler.trashVkImage(_vkImage);
    recycler.trashVmaAllocation(_vmaAllocation);
}

VkDescriptorImageInfo VKAttachmentTexture::getDescriptorImageInfo() {
    if (_vkSampler == VK_NULL_HANDLE) {
        auto backend = _backend.lock();
        auto device = backend->getContext().device;
        // Create sampler
        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;  // VKTODO
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;  // VKTODO
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &_vkSampler));
    }
    if (_vkImageView == VK_NULL_HANDLE) {
        auto backend = _backend.lock();
        auto device = backend->getContext().device;
        // Create image view
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.pNext = nullptr;
        viewCreateInfo.viewType = getVKTextureType(_gpuObject);
        viewCreateInfo.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
        if (viewCreateInfo.format == VK_FORMAT_D24_UNORM_S8_UINT) {
            //viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
            // VKTODO: both VK_IMAGE_ASPECT_DEPTH_BIT and VK_IMAGE_ASPECT_STENCIL_BIT cannot be set at the same time, but I'm not sure which one to set.
            viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        } else {
            viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        }
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.image = _vkImage;
        VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &_vkImageView));
    }

    VkDescriptorImageInfo result {};
    result.sampler = _vkSampler;
    result.imageLayout = _vkImageLayout;  // VKTODO: this needs to be updated on blits and other image writes
    result.imageView = _vkImageView;
    return result;
};


void VKStrictResourceTexture::createTexture(VKBackend &backend) {
    VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    imageCI.extent.width = _gpuObject.getWidth();
    imageCI.extent.height = _gpuObject.getHeight();
    imageCI.extent.depth = 1;
    imageCI.arrayLayers = _gpuObject.isArray() ? _gpuObject.getNumSlices() : 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (_gpuObject.getType() == Texture::TEX_CUBE) {
        imageCI.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.arrayLayers = 6;
    }

    //auto device = _backend.lock()->getContext().device->logicalDevice;

    // Create image for this attachment
    /*VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &_texture));
    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, _texture, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = _backend.lock()->getContext().device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &_vkDeviceMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, _texture, _vkDeviceMemory, 0));*/
    // We need to lock mip data here so that it doesn't change or get deleted before transfer

    _transferData.mipLevels = _gpuObject.getNumMips();
    _transferData.width = _gpuObject.getWidth();
    _transferData.height = _gpuObject.getHeight();

    _transferData.buffer_size = 0;

    for (uint16_t sourceMip = 0; sourceMip < _transferData.mipLevels; ++sourceMip) {
        if (!_gpuObject.isStoredMipFaceAvailable(sourceMip)) {
            continue;
        }
        _transferData.mips.emplace_back();
        //VKTODO: error out if needed
        size_t face_count = 1;
        if (_gpuObject.getType() == Texture::TEX_CUBE) {
            Q_ASSERT(_gpuObject.getNumFaces() == 6);
            face_count = 6;
        } else {
            Q_ASSERT(_gpuObject.getNumFaces() == 1);
        }

        // Is conversion from RGB to RGBA needed?
        bool needsAddingAlpha = false;
        bool needsBGRToRGB = false;
        auto storedFormat = _gpuObject.getStoredMipFormat();
        auto texelFormat = _gpuObject.getTexelFormat();
        if ((storedFormat.getSemantic() == gpu::BGRA || storedFormat.getSemantic() == gpu::SBGRA) &&
            !(texelFormat.getSemantic() == gpu::BGRA || texelFormat.getSemantic() == gpu::SBGRA)) {
            needsBGRToRGB = true;
        }
        auto storedVkFormat = evalTexelFormatInternal(_gpuObject.getStoredMipFormat());
        auto texelVkFormat = evalTexelFormatInternal(_gpuObject.getTexelFormat());
        if (storedFormat.getDimension() != texelFormat.getDimension()) {
            if (storedFormat.getDimension() == gpu::VEC3 && texelFormat.getDimension() == gpu::VEC4) {
                // It's best to make sure that this is not happening in unexpected cases and causing bugs
                Q_ASSERT((storedVkFormat == VK_FORMAT_R8G8B8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_UNORM) ||
                         (storedVkFormat == VK_FORMAT_R8G8B8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_SRGB));
                needsAddingAlpha = true;
            } else {
                qDebug() << "Format mismatch, stored: " << storedVkFormat << " texel: " << texelVkFormat;
                Q_ASSERT(false);
            }
        }

        for (size_t face = 0; face < face_count; face++) {
            auto dim = _gpuObject.evalMipDimensions(sourceMip);
            auto mipData = _gpuObject.accessStoredMipFace(sourceMip, face);  // VKTODO: only one face for now
            auto mipSize = _gpuObject.getStoredMipFaceSize(sourceMip, face);
            if (mipData) {
                TransferData::Mip mip{};
                mip.offset = _transferData.buffer_size;
                mip.size = mipSize;
                mip.data = mipData;
                mip.width = dim.x;
                mip.height = dim.y;
                mip.needsAddingAlpha = needsAddingAlpha;
                mip.needsBGRToRGB = needsBGRToRGB;
                if (needsAddingAlpha) {
                    Q_ASSERT(mipSize % 3 == 0);
                    _transferData.buffer_size += mipSize / 3 * 4;
                } else {
                    _transferData.buffer_size += mipSize;
                }
                _transferData.mips.back().push_back(mip);
                // VKTODO auto texelFormat = evalTexelFormatInternal(_gpuObject.getStoredMipFormat());
                //return copyMipFaceLinesFromTexture(targetMip, face, dim, 0, texelFormat.internalFormat, texelFormat.format, texelFormat.type, mipSize, mipData->readData());
            } else {
                qCDebug(gpu_vk_logging) << "Missing mipData level=" << sourceMip
                                        << " face=" << 0 /*(int)face*/ << " for texture " << _gpuObject.source().c_str();
            }
        }
    }

    imageCI.mipLevels = _transferData.mips.size();

    VmaAllocationCreateInfo allocationCI = {};
    allocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    qDebug() << "storedSize: " << _gpuObject.getStoredSize();
    VK_CHECK_RESULT(vmaCreateImage(vks::Allocation::getAllocator(), &imageCI, &allocationCI, &_vkImage, &_vmaAllocation, nullptr));
    /*if ( _vkImage != (VkImage)(0x260000000026UL)) {
        printf("0x260000000026UL");
    }*/
}

void VKStrictResourceTexture::transfer(VKBackend &backend) {
    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;
    auto device = backend.getContext().device;

    // From VKS
    // Use a separate command buffer for texture loading
    VkCommandBuffer copyCmd = device->createCommandBuffer(device->transferCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Create a host-visible staging buffer that contains the raw image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
    // This buffer is used as a transfer source for the buffer copy
    bufferCreateInfo.size = _transferData.buffer_size;
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
    for (auto &mip : _transferData.mips) {
        for (auto &face : mip) {
            if (face.needsAddingAlpha) {
                // VKTODO: adding alpha and swapping channels at the same time
                Q_ASSERT(!face.needsBGRToRGB);
                size_t pixels = face.size/3;
                for (size_t i = 0; i < pixels; i++) {
                    size_t sourcePos = face.offset + i * 3;
                    size_t destPos = i * 4;
                    data[destPos] = face.data->data()[face.offset + sourcePos];
                    data[destPos + 1] = face.data->data()[face.offset + sourcePos + 1];
                    data[destPos + 2] = face.data->data()[face.offset + sourcePos + 2];
                    data[destPos + 3] = 255;
                }
            } else if (face.needsBGRToRGB) {
                Q_ASSERT(face.size % 4 == 0);
                size_t pixels = face.size/4;
                for (size_t i = 0; i < pixels; i++) {
                    size_t sourcePos = i * 4;
                    size_t destPos = face.offset + i * 4;
                    data[destPos] = face.data->data()[sourcePos + 2];
                    data[destPos + 1] = face.data->data()[sourcePos + 1];
                    data[destPos + 2] = face.data->data()[sourcePos];
                    data[destPos + 3] = face.data->data()[sourcePos + 3];
                }
            } else {
                memcpy(data + face.offset, face.data->data(), face.data->size());
            }
        }
    }
    vkUnmapMemory(device->logicalDevice, stagingMemory);

    std::vector<VkBufferImageCopy> bufferCopyRegions;

    for (size_t mipLevel = 0; mipLevel < _transferData.mips.size(); mipLevel++) {
        for (size_t face = 0; face < _transferData.mips[mipLevel].size(); face++) {
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = mipLevel;
            bufferCopyRegion.imageSubresource.baseArrayLayer = face;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = _transferData.mips[mipLevel][face].width;
            bufferCopyRegion.imageExtent.height = _transferData.mips[mipLevel][face].height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = _transferData.mips[mipLevel][face].offset;
            bufferCopyRegions.push_back(bufferCopyRegion);
        }
    }

    // Create optimal tiled target image
    VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    imageCreateInfo.mipLevels = _transferData.mips.size();
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = { _transferData.width, _transferData.height, 1 };
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    // Ensure that the TRANSFER_DST bit is set for staging
    if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
    {
        imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = _transferData.mips.size();
    if (_gpuObject.getType() == Texture::TEX_CUBE) {
        subresourceRange.layerCount = 6;
    }else{
        subresourceRange.layerCount = 1;
    }

    // Image barrier for optimal image (target)
    // Optimal image will be used as destination for the copy
    vks::tools::setImageLayout(
        copyCmd,
        _vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresourceRange);

    // Copy mip levels from staging buffer
    vkCmdCopyBufferToImage(
        copyCmd,
        stagingBuffer,
        _vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        bufferCopyRegions.size(),
        bufferCopyRegions.data()
    );

    // Change texture image layout to shader read after all mip levels have been copied
    _vkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vks::tools::setImageLayout(
        copyCmd,
        _vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        _vkImageLayout,
        subresourceRange);

    device->flushCommandBuffer(copyCmd, backend.getContext().transferQueue, device->transferCommandPool);

    // Clean up staging resources
    vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
}

void VKStrictResourceTexture::postTransfer(VKBackend &backend) {
    auto device = backend.getContext().device;
    // Create sampler
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR; // VKTODO
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR; // VKTODO
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &_vkSampler));

    // Create image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = nullptr;
    viewCreateInfo.viewType = getVKTextureType(_gpuObject);
    viewCreateInfo.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    viewCreateInfo.subresourceRange.levelCount = 1;
    if (_gpuObject.getType() == Texture::TEX_CUBE) {
        viewCreateInfo.subresourceRange.layerCount = 6;
    } else {
        viewCreateInfo.subresourceRange.layerCount = 1;
    }
    viewCreateInfo.image = _vkImage;
    VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &_vkImageView));
};

VKStrictResourceTexture::~VKStrictResourceTexture() {
    auto backend = _backend.lock();
    auto device = backend->getContext().device->logicalDevice;
    auto &recycler = backend->getContext().recycler;
    recycler.trashVkImageView(_vkImageView);
    if (_vkSampler) {
        recycler.trashVkSampler(_vkSampler);
    }
    recycler.trashVkImage(_vkImage);
    recycler.trashVmaAllocation(_vmaAllocation);
}

void VKExternalTexture::createTexture(VKBackend &backend) {
    auto device = backend.getContext().device;

    VkImageCreateInfo imageCI = vks::initializers::imageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    imageCI.extent.width = _gpuObject.getWidth();
    imageCI.extent.height = _gpuObject.getHeight();
    imageCI.extent.depth = 1;
    imageCI.arrayLayers = _gpuObject.isArray() ? _gpuObject.getNumSlices() : 1;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (_gpuObject.getType() == Texture::TEX_CUBE) {
        imageCI.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageCI.arrayLayers = 6;
    }

    _transferData.mipLevels = 1; // VKTODO: generate mipmaps for web textures later
    _transferData.width = _gpuObject.getWidth();
    _transferData.height = _gpuObject.getHeight();

    _transferData.buffer_size = 0;

    /*for (uint16_t sourceMip = 0; sourceMip < _transferData.mipLevels; ++sourceMip) {
        if (!_gpuObject.isStoredMipFaceAvailable(sourceMip)) {
            continue;
        }
        _transferData.mips.emplace_back();
        //VKTODO: error out if needed
        size_t face_count = 1;
        if (_gpuObject.getType() == Texture::TEX_CUBE) {
            Q_ASSERT(_gpuObject.getNumFaces() == 6);
            face_count = 6;
        }else{
            Q_ASSERT(_gpuObject.getNumFaces() == 1);
        }

        // Is conversion from RGB to RGBA needed?
        bool needsAddingAlpha = false;
        bool needsBGRToRGB = false;
        auto storedFormat = _gpuObject.getStoredMipFormat();
        auto texelFormat = _gpuObject.getTexelFormat();
        if ((storedFormat.getSemantic() == gpu::BGRA || storedFormat.getSemantic() == gpu::SBGRA)
            && !(texelFormat.getSemantic() == gpu::BGRA || texelFormat.getSemantic() == gpu::SBGRA)) {
            needsBGRToRGB = true;
        }
        auto storedVkFormat = evalTexelFormatInternal(_gpuObject.getStoredMipFormat());
        auto texelVkFormat = evalTexelFormatInternal(_gpuObject.getTexelFormat());
        if (storedFormat.getDimension() != texelFormat.getDimension()) {
            if (storedFormat.getDimension() == gpu::VEC3 && texelFormat.getDimension() == gpu::VEC4) {
                // It's best to make sure that this is not happening in unexpected cases and causing bugs
                Q_ASSERT((storedVkFormat == VK_FORMAT_R8G8B8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_UNORM)
                         || (storedVkFormat == VK_FORMAT_R8G8B8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_SRGB));
                needsAddingAlpha = true;
            } else {
                qDebug() << "Format mismatch, stored: " << storedVkFormat << " texel: " << texelVkFormat;
                Q_ASSERT(false);
            }
        }

        for (size_t face = 0; face < face_count; face++) {
            auto dim = _gpuObject.evalMipDimensions(sourceMip);
            auto mipData = _gpuObject.accessStoredMipFace(sourceMip, face);  // VKTODO: only one face for now
            auto mipSize = _gpuObject.getStoredMipFaceSize(sourceMip, face);
            if (mipData) {
                TransferData::Mip mip{};
                mip.offset = _transferData.buffer_size;
                mip.size = mipSize;
                mip.data = mipData;
                mip.width = dim.x;
                mip.height = dim.y;
                mip.needsAddingAlpha = needsAddingAlpha;
                mip.needsBGRToRGB = needsBGRToRGB;
                if (needsAddingAlpha) {
                    Q_ASSERT(mipSize % 3 == 0);
                    _transferData.buffer_size += mipSize / 3 * 4;
                } else {
                    _transferData.buffer_size += mipSize;
                }
                _transferData.mips.back().push_back(mip);
                // VKTODO auto texelFormat = evalTexelFormatInternal(_gpuObject.getStoredMipFormat());
                //return copyMipFaceLinesFromTexture(targetMip, face, dim, 0, texelFormat.internalFormat, texelFormat.format, texelFormat.type, mipSize, mipData->readData());
            } else {
                qCDebug(gpu_vk_logging) << "Missing mipData level=" << sourceMip
                                        << " face=" << 0 << " for texture " << _gpuObject.source().c_str();
            }
        }
    }*/

    imageCI.mipLevels = 1;

    VkExternalMemoryImageCreateInfo externalMemoryImageCI {};
    externalMemoryImageCI.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
#ifdef WIN32
    externalMemoryImageCI.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    externalMemoryImageCI.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    imageCI.pNext = &externalMemoryImageCI;

    VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCI, nullptr, &_vkImage));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device->logicalDevice, _vkImage, &memoryRequirements);

    _sharedMemorySize = memoryRequirements.size;

    //VmaAllocationCreateInfo allocationCI = {};
    //allocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    //qDebug() << "storedSize: " << _gpuObject.getStoredSize();
    VkExportMemoryAllocateInfo exportMemoryAllocateInfo {};
    exportMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
#ifdef WIN32
    exportMemoryAllocateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_BIT;
#else
    exportMemoryAllocateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = device->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    memoryAllocateInfo.pNext = &exportMemoryAllocateInfo;

    VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memoryAllocateInfo, nullptr, &_sharedMemory));
#ifdef WIN32
    VkMemoryGetWin32HandleInfoKHR memoryGetWin32HandleInfoKHR {};
    memoryGetWin32HandleInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
    memoryGetWin32HandleInfoKHR.memory = _sharedMemory,
    memoryGetWin32HandleInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
    VK_CHECK_RESULT(vkGetMemoryWin32HandleKHR(device->logicalDevice, &memoryGetWin32HandleInfoKHR, &_sharedHandle));
#else
    VkMemoryGetFdInfoKHR memoryGetFdInfoKHR {};
    memoryGetFdInfoKHR.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    memoryGetFdInfoKHR.memory = _sharedMemory;
    memoryGetFdInfoKHR.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    VK_CHECK_RESULT(vkGetMemoryFdKHR(device->logicalDevice, &memoryGetFdInfoKHR, &_sharedFd));
#endif
    VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, _vkImage, _sharedMemory, 0));

    VkCommandBuffer transferCmd = device->createCommandBuffer(device->graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1; // VKTODO
    if (_gpuObject.getType() == Texture::TEX_CUBE) {
        subresourceRange.layerCount = 6;
    }else{
        subresourceRange.layerCount = 1;
    }

    vks::tools::setImageLayout(
        transferCmd,
        _vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresourceRange);

    VkClearColorValue color = { .float32 = {0.5, 0.0, 0.0} };
    VkImageSubresourceRange imageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdClearColorImage(transferCmd, _vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &imageSubresourceRange);

    _vkImageLayout = VK_IMAGE_LAYOUT_GENERAL;

    vks::tools::setImageLayout(
        transferCmd,
        _vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        subresourceRange);

    device->flushCommandBuffer(transferCmd, backend.getContext().graphicsQueue, device->graphicsCommandPool);
}

void VKExternalTexture::initGL(gpu::vk::VKBackend& backend) {
    glCreateMemoryObjectsEXT(1, &_openGLMemoryObject); // VKTODO: clean these up later
    ::gl::checkGLError("GL to VK");
#ifdef WIN32
    glImportMemoryWin32HandleEXT(_openGLMemoryObject, _sharedMemorySize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, _sharedHandle);
    ::gl::checkGLError("GL to VK");
#else
    glImportMemoryFdEXT(_openGLMemoryObject, _sharedMemorySize, GL_HANDLE_TYPE_OPAQUE_FD_EXT, _sharedFd);
    _sharedFd = -1; // File descriptor can be used only once?
    ::gl::checkGLError("GL to VK");
#endif

    glCreateTextures(GL_TEXTURE_2D, 1, &_openGLId);
    ::gl::checkGLError("GL to VK");
    Q_ASSERT(evalTexelFormatInternal(_gpuObject.getTexelFormat()) == VK_FORMAT_R8G8B8A8_UNORM);
    glTextureStorageMem2DEXT(_openGLId, 1, GL_RGBA8, _gpuObject.getWidth(), _gpuObject.getHeight(), _openGLMemoryObject, 0);
    ::gl::checkGLError("GL to VK");
    std::array<GLubyte, 4> clearColor {128,128,128,255};
    // Can be enabled for testing
    //glClearTexImage(_openGLId, 0, GL_RGBA, GL_UNSIGNED_BYTE, clearColor.data());
    ::gl::checkGLError("GL to VK");
}

void VKExternalTexture::transferGL(VKBackend &backend) {
    glCopyImageSubData(
        _openGLSourceId, GL_TEXTURE_2D
        , 0, 0, 0, 0
        , _openGLId, GL_TEXTURE_2D
        , 0, 0, 0, 0
        , _gpuObject.getWidth(), _gpuObject.getHeight(), 1);
    ::gl::checkGLError("GL to VK");
    // VKTODO: generate mipmaps
}

void VKExternalTexture::postTransfer(VKBackend &backend) {
    auto device = backend.getContext().device;
    // Create sampler
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR; // VKTODO
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR; // VKTODO
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &_vkSampler));

    // Create image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = nullptr;
    viewCreateInfo.viewType = getVKTextureType(_gpuObject);
    viewCreateInfo.format = evalTexelFormatInternal(_gpuObject.getTexelFormat());
    viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    viewCreateInfo.subresourceRange.levelCount = 1;
    if (_gpuObject.getType() == Texture::TEX_CUBE) {
        viewCreateInfo.subresourceRange.layerCount = 6;
    } else {
        viewCreateInfo.subresourceRange.layerCount = 1;
    }
    viewCreateInfo.image = _vkImage;
    VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &_vkImageView));
};

void VKExternalTexture::releaseExternalTexture() {
    auto backend = _backend.lock();
    if (backend) {
        auto recycler = _gpuObject.getExternalRecycler();
        if (recycler) {
            backend->releaseExternalTexture(_openGLSourceId, recycler);
        } else {
            qWarning() << "No recycler available for texture " << _openGLSourceId << " possible leak";
        }
        const_cast<GLuint&>(_openGLSourceId) = 0;
    }
    Backend::textureExternalCount.decrement();
}

void VKExternalTexture::setSource(GLuint source) {
    if (_openGLSourceId != source) {
        releaseExternalTexture();
        _openGLSourceId = source;
    }
}

VKExternalTexture::~VKExternalTexture() {
    releaseExternalTexture();
    auto backend = _backend.lock();
    auto device = backend->getContext().device->logicalDevice;
    auto &recycler = backend->getContext().recycler;
    recycler.trashVkImageView(_vkImageView);
    if (_vkSampler) {
        recycler.trashVkSampler(_vkSampler);
    }
    recycler.trashVkImage(_vkImage);
    recycler.trashVkDeviceMemory(_sharedMemory);
}

VkDescriptorImageInfo VKExternalTexture::getDescriptorImageInfo() {
    VkDescriptorImageInfo result {};
    result.sampler = _vkSampler;
    result.imageLayout = _vkImageLayout;
    result.imageView = _vkImageView;
    return result;
}

/*Size VKTexture::copyMipFaceFromTexture(uint16_t sourceMip, uint16_t targetMip, uint8_t face) const {
    if (!_gpuObject.isStoredMipFaceAvailable(sourceMip)) {
        return 0;
    }
    auto dim = _gpuObject.evalMipDimensions(sourceMip);
    auto mipData = _gpuObject.accessStoredMipFace(sourceMip, face);
    auto mipSize = _gpuObject.getStoredMipFaceSize(sourceMip, face);
    if (mipData) {
        GLTexelFormat texelFormat = GLTexelFormat::evalGLTexelFormat(_gpuObject.getTexelFormat(), _gpuObject.getStoredMipFormat());
        return copyMipFaceLinesFromTexture(targetMip, face, dim, 0, texelFormat.internalFormat, texelFormat.format, texelFormat.type, mipSize, mipData->readData());
    } else {
        qCDebug(gpugllogging) << "Missing mipData level=" << sourceMip << " face=" << (int)face << " for texture " << _gpuObject.source().c_str();
    }
    return 0;
}*/

//VKTODO:
/*GLExternalTexture::GLExternalTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture, GLuint id)
    : Parent(backend, texture, id) {
    Backend::textureExternalCount.increment();
}

GLExternalTexture::~GLExternalTexture() {
    auto backend = _backend.lock();
    if (backend) {
        auto recycler = _gpuObject.getExternalRecycler();
        if (recycler) {
            backend->releaseExternalTexture(_id, recycler);
        } else {
            qCWarning(gpugllogging) << "No recycler available for texture " << _id << " possible leak";
        }
        const_cast<GLuint&>(_id) = 0;
    }
    Backend::textureExternalCount.decrement();
}

GLVariableAllocationSupport::GLVariableAllocationSupport() {
    Backend::textureResourceCount.increment();
}

GLVariableAllocationSupport::~GLVariableAllocationSupport() {
    Backend::textureResourceCount.decrement();
    Backend::textureResourceGPUMemSize.update(_size, 0);
    Backend::textureResourcePopulatedGPUMemSize.update(_populatedSize, 0);
}

void GLVariableAllocationSupport::incrementPopulatedSize(Size delta) const {
    _populatedSize += delta;
    // Keep the 2 code paths to be able to debug
    if (_size < _populatedSize) {
        Backend::textureResourcePopulatedGPUMemSize.update(0, delta);
    } else  {
        Backend::textureResourcePopulatedGPUMemSize.update(0, delta);
    }
}

void GLVariableAllocationSupport::decrementPopulatedSize(Size delta) const {
    _populatedSize -= delta;
    // Keep the 2 code paths to be able to debug
    if (_size < _populatedSize) {
        Backend::textureResourcePopulatedGPUMemSize.update(delta, 0);
    } else  {
        Backend::textureResourcePopulatedGPUMemSize.update(delta, 0);
    }
}

void GLVariableAllocationSupport::sanityCheck() const {
    if (_populatedMip < _allocatedMip) {
        qCWarning(gpugllogging) << "Invalid mip levels";
    }
}*/

//VKTODO:
/*TransferJob::TransferJob(const Texture& texture,
                         uint16_t sourceMip,
                         uint16_t targetMip,
                         uint8_t face,
                         uint32_t lines,
                         uint32_t lineOffset) {
    auto transferDimensions = texture.evalMipDimensions(sourceMip);
    GLenum format;
    GLenum internalFormat;
    GLenum type;
    GLTexelFormat texelFormat = GLTexelFormat::evalGLTexelFormat(texture.getTexelFormat(), texture.getStoredMipFormat());
    format = texelFormat.format;
    internalFormat = texelFormat.internalFormat;
    type = texelFormat.type;
    _transferSize = texture.getStoredMipFaceSize(sourceMip, face);

    // If we're copying a subsection of the mip, do additional calculations to find the size and offset of the segment
    if (0 != lines) {
        transferDimensions.y = lines;
        auto dimensions = texture.evalMipDimensions(sourceMip);
        auto bytesPerLine = (uint32_t)_transferSize / dimensions.y;
        _transferOffset = bytesPerLine * lineOffset;
        _transferSize = bytesPerLine * lines;
    }

    Backend::texturePendingGPUTransferMemSize.update(0, _transferSize);

    if (_transferSize > GLVariableAllocationSupport::MAX_TRANSFER_SIZE) {
        qCWarning(gpugllogging) << "Transfer size of " << _transferSize << " exceeds theoretical maximum transfer size";
    }

    // Buffering can invoke disk IO, so it should be off of the main and render threads
    _bufferingLambda = [=](const TexturePointer& texture) {
        auto mipStorage = texture->accessStoredMipFace(sourceMip, face);
        if (mipStorage) {
            _mipData = mipStorage->createView(_transferSize, _transferOffset);
        } else {
            qCWarning(gpugllogging) << "Buffering failed because mip could not be retrieved from texture "
                                    << texture->source().c_str();
        }
    };

    _transferLambda = [=](const TexturePointer& texture) {
        if (_mipData) {
            auto gltexture = Backend::getGPUObject<VKTexture>(*texture);
            gltexture->copyMipFaceLinesFromTexture(targetMip, face, transferDimensions, lineOffset, internalFormat, format,
                                                   type, _mipData->size(), _mipData->readData());
            _mipData.reset();
        } else {
            qCWarning(gpugllogging) << "Transfer failed because mip could not be retrieved from texture "
                                    << texture->source().c_str();
        }
    };
}

TransferJob::TransferJob(uint16_t sourceMip, const std::function<void()>& transferLambda) :
    _sourceMip(sourceMip), _bufferingRequired(false), _transferLambda([=](const TexturePointer&) { transferLambda(); }) {}

TransferJob::~TransferJob() {
    Backend::texturePendingGPUTransferMemSize.update(_transferSize, 0);
}*/

VkDescriptorImageInfo VKStrictResourceTexture::getDescriptorImageInfo() {
    VkDescriptorImageInfo result {};
    result.sampler = _vkSampler;
    result.imageLayout = _vkImageLayout;
    result.imageView = _vkImageView;
    return result;
}

#if 0
#include "VKTexture.h"

#include <NumericalConstants.h>

#include "VKTextureTransfer.h"
#include "VKBackend.h"

using namespace gpu;
using namespace gpu::gl;

std::shared_ptr<VKTextureTransferHelper> VKTexture::_textureTransferHelper;
static std::map<uint16, size_t> _textureCountByMips;
static uint16 _currentMaxMipCount { 0 };

// FIXME placeholder for texture memory over-use
#define DEFAULT_MAX_MEMORY_MB 256

const VKenum VKTexture::CUBE_FACE_LAYOUT[6] = {
    VK_TEXTURE_CUBE_MAP_POSITIVE_X, VK_TEXTURE_CUBE_MAP_NEGATIVE_X,
    VK_TEXTURE_CUBE_MAP_POSITIVE_Y, VK_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    VK_TEXTURE_CUBE_MAP_POSITIVE_Z, VK_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

const VKenum VKTexture::WRAP_MODES[Sampler::NUM_WRAP_MODES] = {
    VK_REPEAT,                         // WRAP_REPEAT,
    VK_MIRRORED_REPEAT,                // WRAP_MIRROR,
    VK_CLAMP_TO_EDGE,                  // WRAP_CLAMP,
    VK_CLAMP_TO_BORDER,                // WRAP_BORDER,
    VK_MIRROR_CLAMP_TO_EDGE_EXT        // WRAP_MIRROR_ONCE,
};     

const VKFilterMode VKTexture::FILTER_MODES[Sampler::NUM_FILTERS] = {
    { VK_NEAREST, VK_NEAREST },  //FILTER_MIN_MAG_POINT,
    { VK_NEAREST, VK_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR,
    { VK_LINEAR, VK_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT,
    { VK_LINEAR, VK_LINEAR },  //FILTER_MIN_MAG_LINEAR,

    { VK_NEAREST_MIPMAP_NEAREST, VK_NEAREST },  //FILTER_MIN_MAG_MIP_POINT,
    { VK_NEAREST_MIPMAP_LINEAR, VK_NEAREST },  //FILTER_MIN_MAG_POINT_MIP_LINEAR,
    { VK_NEAREST_MIPMAP_NEAREST, VK_LINEAR },  //FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    { VK_NEAREST_MIPMAP_LINEAR, VK_LINEAR },  //FILTER_MIN_POINT_MAG_MIP_LINEAR,
    { VK_LINEAR_MIPMAP_NEAREST, VK_NEAREST },  //FILTER_MIN_LINEAR_MAG_MIP_POINT,
    { VK_LINEAR_MIPMAP_LINEAR, VK_NEAREST },  //FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    { VK_LINEAR_MIPMAP_NEAREST, VK_LINEAR },  //FILTER_MIN_MAG_LINEAR_MIP_POINT,
    { VK_LINEAR_MIPMAP_LINEAR, VK_LINEAR },  //FILTER_MIN_MAG_MIP_LINEAR,
    { VK_LINEAR_MIPMAP_LINEAR, VK_LINEAR }  //FILTER_ANISOTROPIC,
};

VKenum VKTexture::getGLTextureType(const Texture& texture) {
    switch (texture.getType()) {
    case Texture::TEX_2D:
        return VK_TEXTURE_2D;
        break;

    case Texture::TEX_CUBE:
        return VK_TEXTURE_CUBE_MAP;
        break;

    default:
        qFatal("Unsupported texture type");
    }
    Q_UNREACHABLE();
    return VK_TEXTURE_2D;
}


const std::vector<VKenum>& VKTexture::getFaceTargets(VKenum target) {
    static std::vector<VKenum> cubeFaceTargets {
        VK_TEXTURE_CUBE_MAP_POSITIVE_X, VK_TEXTURE_CUBE_MAP_NEGATIVE_X,
        VK_TEXTURE_CUBE_MAP_POSITIVE_Y, VK_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        VK_TEXTURE_CUBE_MAP_POSITIVE_Z, VK_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    static std::vector<VKenum> faceTargets {
        VK_TEXTURE_2D
    };
    switch (target) {
    case VK_TEXTURE_2D:
        return faceTargets;
    case VK_TEXTURE_CUBE_MAP:
        return cubeFaceTargets;
    default:
        Q_UNREACHABLE();
        break;
    }
    Q_UNREACHABLE();
    return faceTargets;
}

float VKTexture::getMemoryPressure() {
    // Check for an explicit memory limit
    auto availableTextureMemory = Texture::getAllowedGPUMemoryUsage();

    // If no memory limit has been set, use a percentage of the total dedicated memory
    if (!availableTextureMemory) {
        auto totalGpuMemory = gpu::vk::getDedicatedMemory();

        // If no limit has been explicitly set, and the dedicated memory can't be determined, 
        // just use a fallback fixed value of 256 MB
        if (!totalGpuMemory) {
            totalGpuMemory = MB_TO_BYTES(DEFAULT_MAX_MEMORY_MB);
        }

        // Allow 75% of all available GPU memory to be consumed by textures
        // FIXME overly conservative?
        availableTextureMemory = (totalGpuMemory >> 2) * 3;
    }

    // Return the consumed texture memory divided by the available texture memory.
    auto consumedGpuMemory = Context::getTextureGPUMemoryUsage();
    return (float)consumedGpuMemory / (float)availableTextureMemory;
}

VKTexture::DownsampleSource::DownsampleSource(const std::weak_ptr<vk::VKBackend>& backend, VKTexture* oldTexture) :
    _backend(backend),
    _size(oldTexture ? oldTexture->_size : 0),
    _texture(oldTexture ? oldTexture->takeOwnership() : 0),
    _minMip(oldTexture ? oldTexture->_minMip : 0),
    _maxMip(oldTexture ? oldTexture->_maxMip : 0)
{
}

VKTexture::DownsampleSource::~DownsampleSource() {
    if (_texture) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseTexture(_texture, _size);
        }
    }
}

VKTexture::VKTexture(const std::weak_ptr<VKBackend>& backend, const gpu::Texture& texture, VKuint id, VKTexture* originalTexture, bool transferrable) :
    VKObject(backend, texture, id),
    _storageStamp(texture.getStamp()),
    _target(getGLTextureType(texture)),
    _maxMip(texture.maxMip()),
    _minMip(texture.minMip()),
    _virtualSize(texture.evalTotalSize()),
    _transferrable(transferrable),
    _downsampleSource(backend, originalTexture)
{
    if (_transferrable) {
        uint16 mipCount = usedMipLevels();
        _currentMaxMipCount = std::max(_currentMaxMipCount, mipCount);
        if (!_textureCountByMips.count(mipCount)) {
            _textureCountByMips[mipCount] = 1;
        } else {
            ++_textureCountByMips[mipCount];
        }
    } 
    Backend::incrementTextureGPUCount();
    Backend::updateTextureGPUVirtualMemoryUsage(0, _virtualSize);
}


// Create the texture and allocate storage
VKTexture::VKTexture(const std::weak_ptr<vk::VKBackend>& backend, const Texture& texture, VKuint id, bool transferrable) :
    VKTexture(backend, texture, id, nullptr, transferrable)
{
    // FIXME, do during allocation
    //Backend::updateTextureGPUMemoryUsage(0, _size);
    Backend::setGPUObject(texture, this);
}

// Create the texture and copy from the original higher resolution version
VKTexture::VKTexture(const std::weak_ptr<vk::VKBackend>& backend, const gpu::Texture& texture, VKuint id, VKTexture* originalTexture) :
    VKTexture(backend, texture, id, originalTexture, originalTexture->_transferrable)
{
    Q_ASSERT(_minMip >= originalTexture->_minMip);
    // Set the GPU object last because that implicitly destroys the originalTexture object
    Backend::setGPUObject(texture, this);
}

VKTexture::~VKTexture() {
    if (_transferrable) {
        uint16 mipCount = usedMipLevels();
        Q_ASSERT(_textureCountByMips.count(mipCount));
        auto& numTexturesForMipCount = _textureCountByMips[mipCount];
        --numTexturesForMipCount;
        if (0 == numTexturesForMipCount) {
            _textureCountByMips.erase(mipCount);
            if (mipCount == _currentMaxMipCount) {
                _currentMaxMipCount = (_textureCountByMips.empty() ? 0 : _textureCountByMips.rbegin()->first);
            }
        }
    }

    if (_id) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseTexture(_id, _size);
        }
    }
    Backend::updateTextureGPUVirtualMemoryUsage(_virtualSize, 0);
}

void VKTexture::createTexture() {
    withPreservedTexture([&] {
        allocateStorage();
        (void)CHECK_VK_ERROR();
        syncSampler();
        (void)CHECK_VK_ERROR();
    });
}

void VKTexture::setSize(VKuint size) const {
    Backend::updateTextureGPUMemoryUsage(_size, size);
    const_cast<VKuint&>(_size) = size;
}

bool VKTexture::isInvalid() const {
    return _storageStamp < _gpuObject.getStamp();
}

bool VKTexture::isOutdated() const {
    return VKSyncState::Idle == _syncState && _contentStamp < _gpuObject.getDataStamp();
}

bool VKTexture::isOverMaxMemory() const {
    // FIXME switch to using the max mip count used from the previous frame
    if (usedMipLevels() < _currentMaxMipCount) {
        return false;
    }
    Q_ASSERT(usedMipLevels() == _currentMaxMipCount);

    if (getMemoryPressure() < 1.0f) {
        return false;
    }

    return true;
}

bool VKTexture::isReady() const {
    // If we have an invalid texture, we're never ready
    if (isInvalid()) {
        return false;
    }

    // If we're out of date, but the transfer is in progress, report ready
    // as a special case
    auto syncState = _syncState.load();

    if (isOutdated()) {
        return Idle != syncState;
    }

    if (Idle != syncState) {
        return false;
    }

    return true;
}


// Do any post-transfer operations that might be required on the main context / rendering thread
void VKTexture::postTransfer() {
    setSyncState(VKSyncState::Idle);
    ++_transferCount;

    //// The public gltexture becaomes available
    //_id = _privateTexture;

    _downsampleSource.reset();

    // At this point the mip pixels have been loaded, we can notify the gpu texture to abandon it's memory
    switch (_gpuObject.getType()) {
        case Texture::TEX_2D:
            for (uint16_t i = 0; i < Sampler::MAX_MIP_LEVEL; ++i) {
                if (_gpuObject.isStoredMipFaceAvailable(i)) {
                    _gpuObject.notifyMipFaceGPULoaded(i);
                }
            }
            break;

        case Texture::TEX_CUBE:
            // transfer pixels from each faces
            for (uint8_t f = 0; f < CUBE_NUM_FACES; f++) {
                for (uint16_t i = 0; i < Sampler::MAX_MIP_LEVEL; ++i) {
                    if (_gpuObject.isStoredMipFaceAvailable(i, f)) {
                        _gpuObject.notifyMipFaceGPULoaded(i, f);
                    }
                    }
                }
            break;

        default:
            qCWarning(gpu_vk_logging) << __FUNCTION__ << " case for Texture Type " << _gpuObject.getType() << " not supported";
            break;
    }
}

void VKTexture::initTextureTransferHelper() {
    _textureTransferHelper = std::make_shared<VKTextureTransferHelper>();
}
#endif