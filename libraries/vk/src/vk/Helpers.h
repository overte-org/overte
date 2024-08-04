#pragma once

#include "Config.h"
#include <array>
#include <vector>

#include <QtCore/QUuid>


class QOpenGLContext;

namespace vks { namespace util {


inline VkColorComponentFlags fullColorWriteMask() {
    return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
           VK_COLOR_COMPONENT_A_BIT;
}

inline VkViewport viewport(float width, float height, float minDepth = 0, float maxDepth = 1) {
    VkViewport viewport;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return viewport;
}

inline VkViewport viewport(const glm::uvec2& size, float minDepth = 0, float maxDepth = 1) {
    return viewport((float)size.x, (float)size.y, minDepth, maxDepth);
}

inline VkViewport viewport(const VkExtent2D& size, float minDepth = 0, float maxDepth = 1) {
    return viewport((float)size.width, (float)size.height, minDepth, maxDepth);
}

inline VkRect2D rect2D(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) {
    VkRect2D rect2D;
    rect2D.extent.width = width;
    rect2D.extent.height = height;
    rect2D.offset.x = offsetX;
    rect2D.offset.y = offsetY;
    return rect2D;
}

inline VkRect2D rect2D(const glm::uvec2& size, const glm::ivec2& offset = glm::ivec2(0)) {
    return rect2D(size.x, size.y, offset.x, offset.y);
}

inline VkRect2D rect2D(const VkExtent2D& size, const VkOffset2D& offset = VkOffset2D()) {
    return rect2D(size.width, size.height, offset.x, offset.y);
}

inline VkAccessFlags accessFlagsForLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_ACCESS_HOST_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        default:
            return VkAccessFlags();
    }
}

inline VkClearColorValue clearColor(const glm::vec4& v = glm::vec4(0)) {
    VkClearColorValue result;
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
