#pragma once

#include <QtCore/qmetatype.h>
#include "Config.h"
#include <array>
#include <vector>

#include <QtCore/QUuid>


class QOpenGLContext;

namespace vks { namespace util {


inline vk::ColorComponentFlags fullColorWriteMask() {
    return vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
           vk::ColorComponentFlagBits::eA;
}

inline vk::Viewport viewport(float width, float height, float minDepth = 0, float maxDepth = 1) {
    vk::Viewport viewport;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return viewport;
}

inline vk::Viewport viewport(const glm::uvec2& size, float minDepth = 0, float maxDepth = 1) {
    return viewport((float)size.x, (float)size.y, minDepth, maxDepth);
}

inline vk::Viewport viewport(const vk::Extent2D& size, float minDepth = 0, float maxDepth = 1) {
    return viewport((float)size.width, (float)size.height, minDepth, maxDepth);
}

inline vk::Rect2D rect2D(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) {
    vk::Rect2D rect2D;
    rect2D.extent.width = width;
    rect2D.extent.height = height;
    rect2D.offset.x = offsetX;
    rect2D.offset.y = offsetY;
    return rect2D;
}

inline vk::Rect2D rect2D(const glm::uvec2& size, const glm::ivec2& offset = glm::ivec2(0)) {
    return rect2D(size.x, size.y, offset.x, offset.y);
}

inline vk::Rect2D rect2D(const vk::Extent2D& size, const vk::Offset2D& offset = vk::Offset2D()) {
    return rect2D(size.width, size.height, offset.x, offset.y);
}

inline vk::AccessFlags accessFlagsForLayout(vk::ImageLayout layout) {
    switch (layout) {
        case vk::ImageLayout::ePreinitialized:
            return vk::AccessFlagBits::eHostWrite;
        case vk::ImageLayout::eTransferDstOptimal:
            return vk::AccessFlagBits::eTransferWrite;
        case vk::ImageLayout::eTransferSrcOptimal:
            return vk::AccessFlagBits::eTransferRead;
        case vk::ImageLayout::eColorAttachmentOptimal:
            return vk::AccessFlagBits::eColorAttachmentWrite;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return vk::AccessFlagBits::eShaderRead;
        default:
            return vk::AccessFlags();
    }
}

inline vk::ClearColorValue clearColor(const glm::vec4& v = glm::vec4(0)) {
    vk::ClearColorValue result;
    memcpy(&result.float32, &v, sizeof(result.float32));
    return result;
}

bool loadPipelineCacheData(std::vector<uint8_t>& outCache);
void savePipelineCacheData(const std::vector<uint8_t>& cache);

namespace gl {

using UuidSet = std::set<QUuid>;

UuidSet getUuids();
bool contextSupported(QOpenGLContext*);

} // namespace vks::util::gl

}}  // namespace vks::util
