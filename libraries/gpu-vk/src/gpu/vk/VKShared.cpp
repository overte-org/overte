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

vk::Format gpu::vulkan::evalTexelFormatInternal(const gpu::Element& dstFormat) {
    vk::Format result = vk::Format::eR8G8B8Unorm;
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
                result = vk::Format::eR32Uint;
                break;
            case gpu::INT32:
                result = vk::Format::eR32Sint;
                break;
            case gpu::NUINT32:
                result = vk::Format::eR8Unorm;
                break;
            case gpu::NINT32:
                result = vk::Format::eR8Snorm;
                break;
            case gpu::FLOAT:
                result = vk::Format::eR32Sfloat;
                break;
            case gpu::UINT16:
                result = vk::Format::eR16Uint;
                break;
            case gpu::INT16:
                result = vk::Format::eR16Sint;
                break;
            case gpu::NUINT16:
                result = vk::Format::eR16Unorm;
                break;
            case gpu::NINT16:
                result = vk::Format::eR16Snorm;
                break;
            case gpu::HALF:
                result = vk::Format::eR16Sfloat;
                break;
            case gpu::UINT8:
                result = vk::Format::eR8Uint;
                break;
            case gpu::INT8:
                result = vk::Format::eR8Sint;
                break;

            case gpu::NUINT8:

                if ((dstFormat.getSemantic() == gpu::SRGB || dstFormat.getSemantic() == gpu::SRGBA)) {
                    result = vk::Format::eR8Srgb;
                } else {
                    result = vk::Format::eR8Unorm;
                }
                break;
            case gpu::NINT8:
                result = vk::Format::eR8Snorm;
                break;

            default:
                Q_UNREACHABLE();
                break;
            }
            break;

        case gpu::R11G11B10:
            // the type should be float
            result = vk::Format::eB10G11R11UfloatPack32;
            break;

        case gpu::RGB9E5:
            // the type should be float
            result = vk::Format::eE5B9G9R9UfloatPack32;
            break;

        case gpu::DEPTH:
            result = vk::Format::eD32Sfloat;
            switch (dstFormat.getType()) {
            case gpu::UINT32:
            case gpu::INT32:
            case gpu::NUINT32:
            case gpu::NINT32:
            case gpu::FLOAT:
                result = vk::Format::eD32Sfloat;
                break;
            case gpu::UINT16:
            case gpu::INT16:
            case gpu::NUINT16:
            case gpu::NINT16:
            case gpu::HALF:
                result = vk::Format::eD16Unorm;
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
            result = vk::Format::eD24UnormS8Uint;
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
                result = vk::Format::eR32G32Uint;
                break;
            case gpu::INT32:
                result = vk::Format::eR32G32Sint;
                break;
            case gpu::FLOAT:
                result = vk::Format::eR32G32Sfloat;
                break;
            case gpu::UINT16:
                result = vk::Format::eR16G16Uint;
                break;
            case gpu::INT16:
                result = vk::Format::eR16G16Sint;
                break;
            case gpu::NUINT16:
                result = vk::Format::eR16G16Unorm;
                break;
            case gpu::NINT16:
                result = vk::Format::eR16G16Snorm;
                break;
            case gpu::HALF:
                result = vk::Format::eR16G16Sfloat;
                break;
            case gpu::UINT8:
                result = vk::Format::eR8G8Uint;
                break;
            case gpu::INT8:
                result = vk::Format::eR8G8Sint;
                break;
            case gpu::NUINT8:
                result = vk::Format::eR8G8Unorm;
                break;
            case gpu::NINT8:
                result = vk::Format::eR8G8Snorm;
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
            result = vk::Format::eR8G8B8Unorm;
            break;

        case gpu::SRGB:
        case gpu::SRGBA:
            result = vk::Format::eR8G8B8Srgb; // standard 2.2 gamma correction color
            break;
        case gpu::XYZ:
            switch (dstFormat.getType()) { 
                case gpu::FLOAT:
                    result = vk::Format::eR32G32B32Sfloat;
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
            result = vk::Format::eR8G8B8Unorm;
            break;
        case gpu::RGBA:
        case gpu::XYZW:
            switch (dstFormat.getType()) {
            case gpu::UINT32:
                result = vk::Format::eR32G32B32A32Uint;
                break;
            case gpu::INT32:
                result = vk::Format::eR32G32B32A32Sint;
                break;
            case gpu::FLOAT:
                result = vk::Format::eR32G32B32A32Sfloat;
                break;
            case gpu::UINT16:
                result = vk::Format::eR16G16B16A16Uint;
                break;
            case gpu::INT16:
                result = vk::Format::eR16G16B16A16Sint;
                break;
            case gpu::NUINT16:
                result = vk::Format::eR16G16B16A16Unorm;
                break;
            case gpu::NINT16:
                result = vk::Format::eR16G16B16A16Snorm;
                break;
            case gpu::HALF:
                result = vk::Format::eR16G16B16A16Sfloat;
                break;
            case gpu::UINT8:
                result = vk::Format::eR8G8B8A8Uint;
                break;
            case gpu::INT8:
                result = vk::Format::eR8G8B8A8Sint;
                break;
            case gpu::NUINT8:
                result = vk::Format::eR8G8B8A8Unorm;
                break;
            case gpu::NINT8:
                result = vk::Format::eR8G8B8A8Snorm;
                break;
            case gpu::NINT2_10_10_10:
                result = vk::Format::eA2B10G10R10SnormPack32;
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
            result = vk::Format::eR8G8B8Srgb; // standard 2.2 gamma correction color
            break;
        case gpu::SRGBA:
            result = vk::Format::eR8G8B8A8Srgb; // standard 2.2 gamma correction color
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
            result = vk::Format::eBc4UnormBlock;
            break;
        case gpu::COMPRESSED_BC1_SRGB:
            result = vk::Format::eBc1RgbSrgbBlock;
            break;
        case gpu::COMPRESSED_BC1_SRGBA:
            result = vk::Format::eBc1RgbaSrgbBlock;
            break;
        case gpu::COMPRESSED_BC3_SRGBA:
            result = vk::Format::eBc3SrgbBlock;
            break;
        case gpu::COMPRESSED_BC5_XY:
            result = vk::Format::eBc5UnormBlock;
            break;
        case gpu::COMPRESSED_BC6_RGB:
            result = vk::Format::eBc6HUfloatBlock;
            break;
        case gpu::COMPRESSED_BC7_SRGBA:
            result = vk::Format::eBc7SrgbBlock;
            break;
        case gpu::COMPRESSED_ETC2_RGB:
            result = vk::Format::eEtc2R8G8B8UnormBlock;
            break;
        case gpu::COMPRESSED_ETC2_SRGB:
            result = vk::Format::eEtc2R8G8B8SrgbBlock;
            break;
        case gpu::COMPRESSED_ETC2_RGB_PUNCHTHROUGH_ALPHA:
            result = vk::Format::eEtc2R8G8B8A1UnormBlock;
            break;
        case gpu::COMPRESSED_ETC2_SRGB_PUNCHTHROUGH_ALPHA:
            result = vk::Format::eEtc2R8G8B8A1SrgbBlock;
            break;
        case gpu::COMPRESSED_ETC2_RGBA:
            result = vk::Format::eEtc2R8G8B8A8UnormBlock;
            break;
        case gpu::COMPRESSED_ETC2_SRGBA:
            result = vk::Format::eEtc2R8G8B8A8SrgbBlock;
            break;
        case gpu::COMPRESSED_EAC_RED:
            result = vk::Format::eEacR11UnormBlock;
            break;
        case gpu::COMPRESSED_EAC_RED_SIGNED:
            result = vk::Format::eEacR11SnormBlock;
            break;
        case gpu::COMPRESSED_EAC_XY:
            result = vk::Format::eEacR11G11UnormBlock;
            break;
        case gpu::COMPRESSED_EAC_XY_SIGNED:
            result = vk::Format::eEacR11G11SnormBlock;
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

bool gpu::vulkan::isDepthStencilFormat(vk::Format format) {
    switch (format) {
        case vk::Format::eD16Unorm:
        case vk::Format::eX8D24UnormPack32:
        case vk::Format::eD32Sfloat:
        case vk::Format::eS8Uint:
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return true;
        default:
            break;
    }
    return false;
}
