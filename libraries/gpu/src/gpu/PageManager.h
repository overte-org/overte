//
//  Created by Sam Gateau on 10/8/2014.
//  Split from Resource.h/Resource.cpp by Bradley Austin Davis on 2016/08/07
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_PageManager_h
#define hifi_gpu_PageManager_h

#include "Forward.h"

#include <vector>

namespace gpu {

/**
 * Used for partial updates of buffers. Divides memory into pages, and then only pages that changed need to be updated on the GPU.
 */
struct PageManager {
    static const Size DEFAULT_PAGE_SIZE = 4096;

    // Currently only one flag... 'dirty'
    enum Flag {
        DIRTY = 0x01,
    };

    using FlagType = uint8_t;

    // A list of flags
    using Vector = std::vector<FlagType>;
    // A list of pages
    using Pages = std::vector<Size>;

    Vector _pages;
    uint8 _flags{ 0 };
    const Size _pageSize;

    PageManager(Size pageSize = DEFAULT_PAGE_SIZE);
    PageManager& operator=(const PageManager& other);

    /**
     * @brief Check for given flags in this page manager.
     *
     * @param desiredFlags Bitwise and of flags to be checked for.
     * @return `true` if at least one of the desired flags is on for this page manager.
     */
    bool operator()(uint8 desiredFlags) const;

    /**
     * @brief Add given flag fot a page.
     *
     * The function also toggles the flag in object-wide flags property.
     * @param index Index of the page that needs to be flagged.
     * @param markFlags A bitwise and operation of flags that need to be added.
     */
    void markPage(Size index, uint8 markFlags = DIRTY);

    /**
     * @brief Mark memory area in page manager with a desired flag.
     *
     * @param offset Start of the memory area in bytes.
     * @param bytes Size in bytes of the area.
     * @param markFlags A bitwise and operation of flags that need to be added.
     */
    void markRegion(Size offset, Size bytes, uint8 markFlags = DIRTY);

    /**
     * @param desiredFlags Bitwise and of flags to be checked for.
     * @return Number of pages tagged with at least one of the desired flags.
     */
    Size getPageCount(uint8_t desiredFlags = DIRTY) const;

    /**
     * @param desiredFlags Bitwise and of flags to be checked for.
     * @return Size in bytes of the pages tagged with at least one of the desired flag.
     */
    Size getSize(uint8_t desiredFlags = DIRTY) const;

    /**
     * @brief Resizes page manager to a specified number of pages.
     *
     * @param count Number of pages to resize to.
     */
    void setPageCount(Size count);

    /**
     * @param size Data size in bytes.
     * @return Number of pages required to store given number of bytes of data.
     */
    Size getRequiredPageCount(Size size) const;

    /**
     * Since whole pages are required to be allocated, amount of space required to store data is often higher than data size.
     * @param size Data size in bytes.
     * @return Number of bytes required to store given number of bytes of data.
     */
    Size getRequiredSize(Size size) const;

    /**
     * Resize the page manager to fir the given number of bytes of data.
     *
     * @param size Requested size in bytes.
     * @return New size. May be higher than requested, because requested size is rounded up to a page size.
     */
    Size accommodate(Size size);

    /**
     * Get pages with at least one of the specified flags, optionally clearing the flags as we go.
     *
     * @param desiredFlags Bitwise and of flags for pages to be retrieved.
     * @param clear Should the flags be cleared.
     * @return Indices of pages that have at least one of the desired flags.
     */
    Pages getMarkedPages(uint8_t desiredFlags = DIRTY, bool clear = true);

    /**
     * @brief Gets a block of data that needs to be transferred to the GPU.
     *
     * @param outOffset Current offset in bytes is written to this variable.
     * @param outSize Size of the requested block is written to this variable.
     * @param currentPage Before first call should be initialized to 0. Incremented on each call.
     * @return `true` if new transfer block is available.
     */
    bool getNextTransferBlock(Size& outOffset, Size& outSize, Size& currentPage);
};

};

#endif
