//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "VKShared.h"

Q_LOGGING_CATEGORY(gpu_vk_logging, "hifi.gpu.vk")
Q_LOGGING_CATEGORY(trace_gpu_vk, "trace.gpu.vk")
Q_LOGGING_CATEGORY(trace_gpu_vk_detail, "trace.gpu.vk.detail")

VkFormat gpu::vk::evalTexelFormatInternal(const gpu::Element& dstFormat) {
    VkFormat result = VK_FORMAT_R8G8B8_UNORM;
    switch (dstFormat.getDimension()) {
    case gpu::SCALAR:
    {
        switch (dstFormat.getSemantic()) {
        case gpu::RED:
        case gpu::RGB:
        case gpu::RGBA:
        case gpu::SRGB:
        case gpu::SRGBA:
            switch (dstFormat.getType()) {
            case gpu::UINT32:
                result = VK_FORMAT_R32_UINT;
                break;
            case gpu::INT32:
                result = VK_FORMAT_R32_SINT;
                break;
            case gpu::NUINT32:
                result = VK_FORMAT_R8_UNORM;
                break;
            case gpu::NINT32:
                result = VK_FORMAT_R8_SNORM;
                break;
            case gpu::FLOAT:
                result = VK_FORMAT_R32_SFLOAT;
                break;
            case gpu::UINT16:
                result = VK_FORMAT_R16_UINT;
                break;
            case gpu::INT16:
                result = VK_FORMAT_R16_SINT;
                break;
            case gpu::NUINT16:
                result = VK_FORMAT_R16_UNORM;
                break;
            case gpu::NINT16:
                result = VK_FORMAT_R16_SNORM;
                break;
            case gpu::HALF:
                result = VK_FORMAT_R16_SFLOAT;
                break;
            case gpu::UINT8:
                result = VK_FORMAT_R8_UINT;
                break;
            case gpu::INT8:
                result = VK_FORMAT_R8_SINT;
                break;

            case gpu::NUINT8:

                if ((dstFormat.getSemantic() == gpu::SRGB || dstFormat.getSemantic() == gpu::SRGBA)) {
                    result = VK_FORMAT_R8_SRGB;
                } else {
                    result = VK_FORMAT_R8_UNORM;
                }
                break;
            case gpu::NINT8:
                result = VK_FORMAT_R8_SNORM;
                break;

            default:
                Q_UNREACHABLE();
                break;
            }
            break;

        case gpu::R11G11B10:
            // the type should be float
            result = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            break;

        case gpu::RGB9E5:
            // the type should be float
            result = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            break;

        case gpu::DEPTH:
            result = VK_FORMAT_D32_SFLOAT;
            switch (dstFormat.getType()) {
            case gpu::UINT32:
            case gpu::INT32:
            case gpu::NUINT32:
            case gpu::NINT32:
            case gpu::FLOAT:
                result = VK_FORMAT_D32_SFLOAT;
                break;
            case gpu::UINT16:
            case gpu::INT16:
            case gpu::NUINT16:
            case gpu::NINT16:
            case gpu::HALF:
                result = VK_FORMAT_D16_UNORM;
                break;
            case gpu::UINT8:
            case gpu::INT8:
            case gpu::NUINT8:
            case gpu::NINT8:
            default:
                Q_UNREACHABLE();
                break;
            }
            break;

        case gpu::DEPTH_STENCIL:
            result = VK_FORMAT_D24_UNORM_S8_UINT;
            break;

        default:
            qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
        }
        break;
    }

    case gpu::VEC2:
    {
        switch (dstFormat.getSemantic()) {
        case gpu::RGB:
        case gpu::RGBA:
        case gpu::XY:
        case gpu::XYZ:
        case gpu::UV:
            switch (dstFormat.getType()) {
            case gpu::UINT32:
                result = VK_FORMAT_R32G32_UINT;
                break;
            case gpu::INT32:
                result = VK_FORMAT_R32G32_SINT;
                break;
            case gpu::FLOAT:
                result = VK_FORMAT_R32G32_SFLOAT;
                break;
            case gpu::UINT16:
                result = VK_FORMAT_R16G16_UINT;
                break;
            case gpu::INT16:
                result = VK_FORMAT_R16G16_SINT;
                break;
            case gpu::NUINT16:
                result = VK_FORMAT_R16G16_UNORM;
                break;
            case gpu::NINT16:
                result = VK_FORMAT_R16G16_SNORM;
                break;
            case gpu::HALF:
                result = VK_FORMAT_R16G16_SFLOAT;
                break;
            case gpu::UINT8:
                result = VK_FORMAT_R8G8_UINT;
                break;
            case gpu::INT8:
                result = VK_FORMAT_R8G8_SINT;
                break;
            case gpu::NUINT8:
                result = VK_FORMAT_R8G8_UNORM;
                break;
            case gpu::NINT8:
                result = VK_FORMAT_R8G8_SNORM;
                break;
            case gpu::NUINT32:
            case gpu::NINT32:
            case gpu::NUINT2:
            case gpu::NINT2_10_10_10:
            case gpu::COMPRESSED:
            case gpu::NUM_TYPES: // quiet compiler
                Q_UNREACHABLE();
            }
            break;
        default:
            qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
        }

        break;
    }

    case gpu::VEC3:
    {
        switch (dstFormat.getSemantic()) {
        case gpu::RGB:
        case gpu::RGBA:
            result = VK_FORMAT_R8G8B8_UNORM;
            break;

        case gpu::SRGB:
        case gpu::SRGBA:
            result = VK_FORMAT_R8G8B8_SRGB; // standard 2.2 gamma correction color
            break;
        case gpu::XYZ:
            switch (dstFormat.getType()) { 
                case gpu::FLOAT:
                    result = VK_FORMAT_R32G32B32_SFLOAT;
                    break;
                default:
                    qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
            }
            break;

        default:
            qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
        }

        break;
    }

    case gpu::VEC4:
    {
        switch (dstFormat.getSemantic()) {
        case gpu::RGB:
            result = VK_FORMAT_R8G8B8_UNORM;
            break;
        case gpu::RGBA:
        case gpu::XYZW:
            switch (dstFormat.getType()) {
            case gpu::UINT32:
                result = VK_FORMAT_R32G32B32A32_UINT;
                break;
            case gpu::INT32:
                result = VK_FORMAT_R32G32B32A32_SINT;
                break;
            case gpu::FLOAT:
                result = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            case gpu::UINT16:
                result = VK_FORMAT_R16G16B16A16_UINT;
                break;
            case gpu::INT16:
                result = VK_FORMAT_R16G16B16A16_SINT;
                break;
            case gpu::NUINT16:
                result = VK_FORMAT_R16G16B16A16_UNORM;
                break;
            case gpu::NINT16:
                result = VK_FORMAT_R16G16B16A16_SNORM;
                break;
            case gpu::HALF:
                result = VK_FORMAT_R16G16B16A16_SFLOAT;
                break;
            case gpu::UINT8:
                result = VK_FORMAT_R8G8B8A8_UINT;
                break;
            case gpu::INT8:
                result = VK_FORMAT_R8G8B8A8_SINT;
                break;
            case gpu::NUINT8:
                result = VK_FORMAT_R8G8B8A8_UNORM;
                break;
            case gpu::NINT8:
                result = VK_FORMAT_R8G8B8A8_SNORM;
                break;
            case gpu::NINT2_10_10_10:
                result = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
                break;

            case gpu::NUINT32:
            case gpu::NINT32:
            case gpu::NUINT2:
            case gpu::COMPRESSED:
            case gpu::NUM_TYPES: // quiet compiler
                Q_UNREACHABLE();
            }
            break;
        case gpu::SRGB:
            result = VK_FORMAT_R8G8B8_SRGB; // standard 2.2 gamma correction color
            break;
        case gpu::SRGBA:
            result = VK_FORMAT_R8G8B8A8_SRGB; // standard 2.2 gamma correction color
            break;
        default:
            qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
        }
        break;
    }
    case gpu::TILE4x4:
    {
        switch (dstFormat.getSemantic()) {
        case gpu::COMPRESSED_BC4_RED:
            result = VK_FORMAT_BC4_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_BC1_SRGB:
            result = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_BC1_SRGBA:
            result = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_BC3_SRGBA:
            result = VK_FORMAT_BC3_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_BC5_XY:
            result = VK_FORMAT_BC5_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_BC6_RGB:
            result = VK_FORMAT_BC6H_UFLOAT_BLOCK;
            break;
        case gpu::COMPRESSED_BC7_SRGBA:
            result = VK_FORMAT_BC7_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_ETC2_RGB:
            result = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_ETC2_SRGB:
            result = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_ETC2_RGB_PUNCHTHROUGH_ALPHA:
            result = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_ETC2_SRGB_PUNCHTHROUGH_ALPHA:
            result = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_ETC2_RGBA:
            result = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_ETC2_SRGBA:
            result = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
            break;
        case gpu::COMPRESSED_EAC_RED:
            result = VK_FORMAT_EAC_R11_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_EAC_RED_SIGNED:
            result = VK_FORMAT_EAC_R11_SNORM_BLOCK;
            break;
        case gpu::COMPRESSED_EAC_XY:
            result = VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
            break;
        case gpu::COMPRESSED_EAC_XY_SIGNED:
            result = VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
            break;
        default:
            qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
        }
        break;
    }

    default:
        qCWarning(gpu_vk_logging) << "Unknown combination of texel format";
    }
    return result;
}

bool gpu::vk::isDepthStencilFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            break;
    }
    return false;
}
