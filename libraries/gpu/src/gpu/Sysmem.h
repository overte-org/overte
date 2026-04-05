//
//  Created by Sam Gateau on 10/8/2014.
//  Split from Resource.h/Resource.cpp by Bradley Austin Davis on 2016/08/07
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Sysmem_h
#define hifi_gpu_Sysmem_h


#include "Forward.h"

namespace gpu {

/// Sysmem is the underneath cache for the data of a resource in CPU RAM.
class Sysmem {
public:
    static const Size NOT_ALLOCATED = INVALID_SIZE;

    Sysmem();

    /**
     * @param size Size of the sysmem buffer in bytes.
     * @param bytes Pointer to the data to be copied to the buffer.
     */
    Sysmem(Size size, const Byte* bytes);
    Sysmem(const Sysmem& sysmem); // deep copy of the sysmem buffer
    Sysmem& operator=(const Sysmem& sysmem); // deep copy of the sysmem buffer
    ~Sysmem();

    /**
     * @return Size in bytes of the buffer.
     */
    Size getSize() const { return _size; }

    /**
     * @brief Allocate the byte array.
     *
     * Previous content is not copied over to the newly allocated memory.
     * @param pSize Size in bytes of the new allocation.
     * @return Buffer size in bytes after allocation. Zero if allocation fails.
     */
    Size allocate(Size pSize);

    /**
     * @brief Resize the byte array.
     *
     * Keeps previous data [0 to min(pSize, mSize)].
     * @param pSize New buffer size in bytes.
     * @return Buffer size in bytes after resizing. Returns old size if allocation fails.
     */
    Size resize(Size pSize);

    /**
     * @brief Assign data bytes and size (allocate for size, then copy bytes if exists).
     * @param size Size in bytes.
     * @param bytes Pointer to the data.
     * @return Buffer size after setting data. If allocation fails returned size will be different from requested.
     */
    Size setData(Size size, const Byte* bytes);

    /**
     * @brief Update a subset of buffer data,
     *
     * Doesn't reallocate and only copies specified number of bytes at the offset location.
     * Fails if the data doesn't fit in currently allocated buffer.
     * @param offset Offset in bytes where the subset starts.
     * @param size Size of the data subset in bytes.
     * @param bytes Pointer to new data to be copied.
     * @return Subset size on success, zero on failure.
     */
    Size setSubData(Size offset, Size size, const Byte* bytes);

    // Append new data at the end of the current buffer
    // do a resize( size + getSIze) and copy the new data
    // \return
    /**
     * @brief Append new data at the end of the current buffer.
     *
     * Resizes buffer to (size + getSIze) and copy the new data
     * @param size Size in bytes of the data to append.
     * @param data Pointer to the data o append.
     * @return The number of bytes copied or 0 if it fails.
     */
    Size append(Size size, const Byte* data);

    /**
     * @brief Access the byte array.
     * @return Pointer to the data.
     */
    const Byte* readData() const { return _data; }

    /**
     * @brief Access the byte array for reading and writing.
     * @return Pointer to the data.
     */
    Byte* editData() { return _data; }

    /**
     * @tparam T Type to cast data to.
     * @return Data as a requested type.
     */
    template< typename T > const T* read() const { return reinterpret_cast< T* > (_data); }

    /**
     * @tparam T Type to cast data to.
     * @return Data as a requested type.
     */
    template< typename T > T* edit() { return reinterpret_cast< T* > (_data); }

    /**
     * Stamp is a counter that is incremented when sysmem object changes size.
     * It's not incremented when contents are edited without resizing.
     * @return Current stamp.
     */
    Stamp getStamp() const { return _stamp; }

private:
    /**
     * @brief Helper function for allocating memory.
     * @param memAllocated Pointer to allocated memory will be stored in this variable.
     * @param size Size in bytes to allocate.
     * @return Allocated size or NOT_ALLOCATED on failure.
     */
    static Size allocateMemory(Byte** memAllocated, Size size);

    /**
     * @brief Helper function for freeing memory.
     * @param memDeallocated Pointer for which memory will be freed.
     * @param size Size of the memory to be freed (not used).
     */
    static void deallocateMemory(Byte* memDeallocated, Size size);

    /**
     * @return True if this sysmem object has allocated memory.
     */
    bool isAvailable() const { return (_data != nullptr); }


    Stamp _stamp{ 0 };
    Size  _size{ 0 };
    Byte* _data{ nullptr };
}; // Sysmem

}

#endif
