#pragma once

#include "Allocation.h"

namespace vks {
    // Encaspulates an image, the memory for that image, a view of the image,
    // as well as a sampler and the image format.
    //
    // The sampler is not populated by the allocation code, but is provided
    // for convenience and easy cleanup if it is populated.
    struct Image : public Allocation
    {
    private:
        using Parent = Allocation;
    public:
        VkImage image {nullptr};
        VkExtent3D extent {};
        VkImageView view {nullptr};
        VkSampler sampler {nullptr};
        VkFormat format{ VK_FORMAT_UNDEFINED };

        operator bool() const {
            return image != nullptr;
        }

        void destroy() override {
            vkDestroySampler(device, sampler, nullptr);
            vkDestroyImageView(device, view, nullptr);
            vkDestroyImage(device, image, nullptr);
            Parent::destroy();
        }
    };
}
