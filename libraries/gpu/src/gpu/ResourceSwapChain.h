//
//  Created by Olivier Prat on 2018/02/19
//  Copyright 2013-2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_ResourceSwapChain_h
#define hifi_gpu_ResourceSwapChain_h

#include <memory>
#include <array>

namespace gpu {
    /// Base class for swapchains.
    class SwapChain {
    public:
        /**
         * @param size Number of objects stored in the swapchain.
         */
        SwapChain(uint8_t size = 2U) : _size{ size } {}
        virtual ~SwapChain() {}

        /**
         * Choose next object from circular buffer as the active one.
         */
        void advance() {
            _frontIndex = (_frontIndex + 1) % _size;
        }

        /**
         * @return Number of objects stored in the swapchain.
         */
        uint8_t getSize() const { return _size; }

    protected:
        /// Number of objects stored in the swapchain.
        const uint8_t _size;

        /// Index of the current object in the circular buffer.
        uint8_t _frontIndex{ 0U };

    };
    typedef std::shared_ptr<SwapChain> SwapChainPointer;

    /// Swapchain container template.
    /// Used with Framebuffer objects.
    template <class R>
    class ResourceSwapChain : public SwapChain {
    public:

        enum {
            MAX_SIZE = 4
        };

        using Type = R;
        using TypePointer = std::shared_ptr<R>;
        using TypeConstPointer = std::shared_ptr<const R>;

        /**
         * @param v Type of the object that is stored in the swapchain.
         */
        ResourceSwapChain(const std::vector<TypePointer>& v) : SwapChain{ std::min<uint8_t>((uint8_t)v.size(), MAX_SIZE) } {
            for (size_t i = 0; i < _size; ++i) {
                _resources[i] = v[i];
            }
        }

        /**
         * For example `index = 0` returns active object, and `index = 1` returns next one.
         *
         * @param index Object index relative to currently active object.
         * @return Reference to the object with particular index in relation to currently active object.
         */
        const TypePointer& get(unsigned int index) const { return _resources[(index + _frontIndex) % _size]; }

    private:

        /// Circular buffer storing objects.
        std::array<TypePointer, MAX_SIZE> _resources;
    };
}

#endif
