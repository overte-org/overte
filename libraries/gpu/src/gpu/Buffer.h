//
//  Created by Sam Gateau on 10/8/2014.
//  Split from Resource.h/Resource.cpp by Bradley Austin Davis on 2016/08/07
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Buffer_h
#define hifi_gpu_Buffer_h

#include <atomic>

#if _DEBUG
#include <QtCore/QDebug>
#include "GPULogging.h"
#endif

#include "Forward.h"
#include "Format.h"
#include "Resource.h"
#include "Sysmem.h"
#include "PageManager.h"
#include "Metric.h"

namespace gpu {

/**
 * Represents various kinds of buffers (index, vertex, uniform, resource, etc.) on the CPU side.
 */
class Buffer : public Resource {

    // Incremented and decremented in buffer constructors.
    static ContextMetricCount _bufferCPUCount;

    // Updated when buffer is resized or destroyed.
    static ContextMetricSize _bufferCPUMemSize;

public:
    using Flag = PageManager::Flag;

    // VKTODO: Make this independent of VkBufferUsageFlagBits.
    // Flags match VkBufferUsageFlagBits for convenience... do not modify
    enum Usage
    {
        // These values are unused in our API
        //TransferSrc = 0x0001,
        //TransferDst = 0x0002,
        //UniformTexelBuffer = 0x0004,
        //StorageTexelBuffer = 0x0008,
        UniformBuffer = 0x0010,
        ResourceBuffer = 0x0020,
        IndexBuffer = 0x0040,
        VertexBuffer = 0x0080,
        IndirectBuffer = 0x0100,
        AllFlags = 0x01F3
    };

    /**
     * Represents update to a buffer.
     * The updates are generated in render thread after the batch has been generated.
     * The updates are later applied before the frame is rendered.
     * They are also applied when the frame object was generated but it's being skipped instead of being rendered.
     */
    class Update {
    public:
        /**
         * @brief Generates an update object based on current dirty pages in the buffer.
         *
         * Called by `Batch::finishFrame` on the render thread when the frame is ready to be sent to the backend.
         * @param buffer Buffer to which this update will be applied to.
         */
        Update(const Buffer& buffer);

        /**
         * @brief Copy constructor for the Update object.
         * @param other Update object which will be copied.
         */
        Update(const Update& other);

        /**
         * @brief Move constructor for the Update object.
         * @param other Update object which will be moved.
         */
        Update(Update&& other);

        /**
         * @brief Applies the update to the buffer.
         *
         * Called before the frame gets rendered or dropped.
         * After the batches are generated changes to the buffer contents need to be applied. The reason for this is that
         * a frame is executed at the same time as another frame object is being generated.
         */
        void apply() const;

    private:
        // Reference to the buffer this update is for.
        const Buffer& buffer;

        // Each update has an index number that is incremented by one from the index of the previous update.
        // It's used to ensure that updates are applied in correct order and that none are skipped.
        size_t updateNumber;

        // It's equal to size of the all pages for the given buffer.
        Size size;

        // Indices of pages that need to be updated.
        PageManager::Pages dirtyPages;

        // Data for the pages that need to be updated.
        std::vector<uint8> dirtyData;
    };

    /**
     * @return Total count of currently existing buffers.
     */
    static uint32_t getBufferCPUCount();

    /**
     * @return Total size in bytes of currently existing buffers.
     */
    static Size getBufferCPUMemSize();

    // VKTODO: experiment with making buffer usage inferred from other commands on the batch.
    /**
     * @param usage A bit sum of `Buffer::Usage` flags describing this buffer.
     * @param pageSize Size of the page in bytes. Pages are the smallest parts of the buffer that can be updated.
     */
    Buffer(uint32_t usage, Size pageSize = PageManager::DEFAULT_PAGE_SIZE);

    /**
     * @brief Creates a buffer from std::vector.
     * @tparam T Vector type.
     * @param usage Planned usage of this buffer, bit sum of Buffer::Usage flags.
     * @param v Vector, which contents will be copied to the buffer.
     * @return Pointer to the newly created buffer.
     */
    template <typename T>
    static Buffer* createBuffer(uint32_t usage, const std::vector<T>& v) {
        return new Buffer(usage, sizeof(T) * v.size(), (const gpu::Byte*)v.data());
    }

    /**
     * @brief Creates buffer with a given content.
     * @param usage Planned usage of this buffer, bit sum of Buffer::Usage flags.
     * @param size Size in bytes.
     * @param bytes Pointer to the data.
     * @param pageSize Size of the page in bytes. Pages are the smallest parts of the buffer that can be updated.
     */
    Buffer(uint32_t usage, Size size, const Byte* bytes, Size pageSize = PageManager::DEFAULT_PAGE_SIZE);

    /**
     * @brief Copy constructor.
     *
     * Performs deep copy of the sysmem buffer.
     * @param buf Buffer to be copied.
     */
    Buffer(const Buffer& buf);

    /**
    * @brief Assignment constructor.
     *
     * Performs deep copy of the sysmem buffer.
     * @param buf Buffer to be copied.
     * @return Reference to the destination buffer.
     */
    Buffer& operator=(const Buffer& buf);

    /**
     * Buffer destructor updated statistics.
     */
    ~Buffer();

    /**
     * @return The size in bytes of data stored in the buffer
     */
    Size getSize() const override;

    /**
     * @tparam T Element type.
     * @return Number of elements of a given type in the buffer.
     */
    template <typename T>
    Size getNumTypedElements() const {
        return getSize() / sizeof(T);
    };

    /**
     * @return Constant pointer to buffer data.
     */
    const Byte* getData() const { return getSysmem().readData(); }

    /**
     * @return Returns usage, which is a bit sum of Buffer::Usage flags.
     */
    uint32_t getUsage() const { return _usage; }

    /**
     * @brief Resizes buffer.
     *
     * Keeps previous data [0 to min(pSize, mSize)].
     * @param pSize New size in bytes.
     * @return New size, same as input parameter.
     */
    Size resize(Size pSize);

    /**
     * @brief Assign data bytes and size (allocate for size, then copy bytes if exists).
     * @param size New size in bytes.
     * @param data Pointer to data of size specified in the previous parameter.
     * @return New size, same as input parameter.
     */
    Size setData(Size size, const Byte* data);

    // Assign data bytes and size (allocate for size, then copy bytes if exists)
    // \return
    /**
     * @brief Set a subset of buffer's data.
     *
     * Fails and returns 0 if buffer is not big enough.
     * @param offset Offset at which to start writing data.
     * @param size Size in bytes of data to write.
     * @param data Pointer to the data.
     * @return The number of bytes copied or zero if it was not possible to copy them.
     */
    Size setSubData(Size offset, Size size, const Byte* data);

    /**
     * @brief Set a subset of buffer's data using data of a given object.
     *
     * @tparam T Type of the object.
     * @param index Position in the buffer to which to write to. Offset in bytes is (index * sizeof(T)).
     * @param t Instance of the object that will be the source of data.
     * @return Number of bytes written.
     */
    template <typename T>
    Size setSubData(Size index, const T& t) {
        Size offset = index * sizeof(T);
        Size size = sizeof(T);
        return setSubData(offset, size, reinterpret_cast<const Byte*>(&t));
    }

    /**
     * @brief Set a subset of buffer's data using a vector of objects of a given type.
     *
     * @tparam T Object type.
     * @param index Position in the buffer to which to write to. Offset in bytes is (index * sizeof(T)).
     * @param t Vector of objects that will be the source of data.
     * @return Number of bytes written.
     */
    template <typename T>
    Size setSubData(Size index, const std::vector<T>& t) {
        if (t.empty()) {
            return 0;
        }
        Size offset = index * sizeof(T);
        Size size = t.size() * sizeof(T);
        return setSubData(offset, size, reinterpret_cast<const Byte*>(&t[0]));
    }

    /**
     * @brief Append new data at the end of the current buffer.
     *
     * Resizes buffer to (size + getSize) and copies the new data.
     * @param size Size of the data to append in bytes.
     * @param data Pointer to the data.
     * @return Number of bytes copied.
     */
    Size append(Size size, const Byte* data);

    /**
     * @brief Append new data at the end of the current buffer from an object of a given type.
     *
     * @tparam T Object type.
     * @param t Instance of the object that will be the source of data.
     * @return Number of bytes copied.
     */
    template <typename T>
    Size append(const T& t) {
        return append(sizeof(t), reinterpret_cast<const Byte*>(&t));
    }

    /**
     * @brief Append new data at the end of the current buffer using a vector of objects of a given type.
     *
     * @tparam T Object type.
     * @param t Vector of objects that will be the source of data.
     * @return Number of bytes copied.
     */
    template <typename T>
    Size append(const std::vector<T>& t) {
        if (t.empty()) {
            return _end;
        }
        return append(sizeof(T) * t.size(), reinterpret_cast<const Byte*>(&t[0]));
    }

    /**
     * Pointer to the renderer backend object associated with this object.
     * The renderer backend object will have different type depending on currently running backend, for example OpenGL or Vulkan.
     */
    const GPUObjectPointer gpuObject{};

    /**
     * @brief Access the sysmem object.
     *
     * Symsmem object contains buffer data on the CPU side. Rendering backends upload it to the GPU.
     * Intended only for Buffer and GPUObject derived classes.
     * @return Constant reference to the sysmem object.
     */
    const Sysmem& getSysmem() const { return _sysmem; }

    /**
     * @return true if there are updates for this buffer that were not applied yet.
     */
    bool isDirty() const { return _pages(PageManager::DIRTY); }

    /**
     * @brief Applies update to a buffer.
     *
     * After the batches are generated changes to the buffer contents need to be applied. The reason for this is that
     * a frame is executed at the same time as another frame object is being generated.
     * @param update Update object to apply.
     */
    void applyUpdate(const Update& update);

    // Main thread operation to say that the buffer is ready to be used as a frame
    /**
     * @brief Generate Update object from the current changes to the buffer.
     *
     * Should be called only if `isDirty` returns true.
     * @return Update object.
     */
    Update getUpdate() const;

protected:
    /**
     * @brief Instantly applies current changes to a buffer without creating an Update object.
     *
     * For use by the render thread to avoid the intermediate step of getUpdate/applyUpdate.
     */
    void flush() const;

    // FIXME don't maintain a second buffer continuously.  We should be able to apply updates
    // directly to the GL object and discard _renderSysmem and _renderPages

    /**
     * PageManager for second copy of the buffer (one ot which updates are applied).
     */
    mutable PageManager _renderPages;

    /**
     * Second copy of the buffer.
     * Update objects need to be generated from _sysmem and applying them copies them to _renderSysmem.
     */
    mutable Sysmem _renderSysmem;

    /**
     * Number of times a buffer update object was created.
     */
    mutable std::atomic<size_t> _getUpdateCount{ 0 };

    /**
     * Number of times a buffer update object was applied.
     */
    mutable std::atomic<size_t> _applyUpdateCount{ 0 };

    /**
     * @brief Marks pages on which specified data range is stored as dirty.
     *
     * Used to notify buffer update system that data was edited.
     * @param offset
     * @param bytes
     */
    void markDirty(Size offset, Size bytes);

    /**
     * @brief Marks pages on which data from a number of objects of a specified type range is stored as dirty.
     *
     * @tparam T Type of the object from which data is stored.
     * @param index Start position. Offset in bytes is index * sizeof(T).
     * @param count Number of objects to mark as dirty. Range in bytes is count * sizeof(T).
     */
    template <typename T>
    void markDirty(Size index, Size count = 1) {
        markDirty(sizeof(T) * index, sizeof(T) * count);
    }

    /**
     * @return Reference to sysmem object.
     */
    Sysmem& editSysmem() { return _sysmem; }

    /**
     * @brief Returns pointer to data to be edited, but data still needs to be flagged as dirty with a separate `makeDirty` call.
     * @return Pointer to buffer data.
     */
    Byte* editData() { return editSysmem().editData(); }

    /**
     * PageManager for _sysmem.
     * When buffer memory is edited, pages that were changed are flagged here and then Update object can be generated.
     * Later applying the update copies changes to _renderSysmem object.
     */
    mutable PageManager _pages;

    /**
     * Size of the buffer.
     */
    Size _end{ 0 };

    /**
     * Changes to the buffer are written here, and then pages corresponding to them in _pages are marked as dirty.
     * This is later used to create Update object that can be applied to _renderSysmem.
     */
    Sysmem _sysmem;

    /**
     * Usage of this buffer.
     * It's a bit sum of `Buffer::Usage`.
     */
    const uint32_t _usage{ 0 };

    friend class Serializer;
    friend class Deserializer;
    friend class BufferView;
    friend class Frame;
    friend class Batch;

    // FIXME find a more generic way to do this.
    friend class ::gpu::vk::VKBuffer;
    friend class ::gpu::vk::VKBackend;
    friend class gl::GLBackend;
    friend class gl::GLBuffer;
    friend class gl41::GL41Buffer;
    friend class gl45::GL45Buffer;
    friend class gles::GLESBuffer;
    friend class vk::VKBuffer;
};

using BufferUpdates = std::vector<Buffer::Update>;

typedef std::shared_ptr<Buffer> BufferPointer;
typedef std::vector<BufferPointer> Buffers;

/**
 *
 */
class BufferView {
protected:
    //
    static const Resource::Size DEFAULT_OFFSET{ 0 };

    //
    static const Element DEFAULT_ELEMENT;

public:
    using Size = Resource::Size;
    using Index = int32_t;

    /**
     * Shared pointer to the Buffer that this BufferView uses.
     */
    BufferPointer _buffer;

    /**
     * Offset in bytes at which given view starts in the buffer.
     */
    Size _offset{ 0 };

    /**
     * Size of the buffer view in bytes.
     */
    Size _size{ 0 };

    /**
     * Type of the element for this buffer view.
     */
    Element _element{ DEFAULT_ELEMENT };

    /**
     * Stride in bytes (offset between two consecutive elements) in the buffer view.
     */
    uint16 _stride{ 0 };

    BufferView(const BufferView& view) = default;
    BufferView& operator=(const BufferView& view) = default;

    /**
     * Constructs buffer view with empty shared pointer to the buffer and default element type.
     */
    BufferView();

    /**
     * Constructs buffer view with empty shared pointer to the buffer.
     * @param element Element tpye for the buffer.
     */
    BufferView(const Element& element);

    /**
     * @brief Constructs BufferView using a pointer to a new buffer.
     *
     * Shared pointer is created from the provided pointer, so that Buffer will be automatically deleted when it's not needed.
     * @param newBuffer Pointer to the new Buffer.
     * @param element Element type.
     */
    BufferView(Buffer* newBuffer, const Element& element = DEFAULT_ELEMENT);

    /**
     * @brief Constructs BufferView using an existing shader pointer to a Buffer.
     *
     * @param buffer Shared pointer to a buffer.
     * @param element Element type.
     */
    BufferView(const BufferPointer& buffer, const Element& element = DEFAULT_ELEMENT);

    /**
     * @brief Constructs BufferView using an existing shader pointer to a Buffer.
     *
     * @param buffer Shared pointer to a buffer.
     * @param offset Offset in bytes where the buffer view starts.
     * @param size Size in bytes.
     * @param element Element type.
     */
    BufferView(const BufferPointer& buffer, Size offset, Size size, const Element& element = DEFAULT_ELEMENT);

    /**
     *
     * @param buffer Shared pointer to a buffer.
     * @param offset Offset in bytes where the buffer view starts.
     * @param size Size in bytes.
     * @param stride Offset between two consecutive elements in bytes.
     * @param element Element type.
     */
    BufferView(const BufferPointer& buffer, Size offset, Size size, uint16 stride, const Element& element = DEFAULT_ELEMENT);

    /**
     * @return Number of elements in the buffer view.
     */
    Size getNumElements() const { return _size / _stride; }

    /**
     * @brief Template iterator with random access on the buffer sysmem.
     * @tparam T Type of the element, for example `glm::vec3`.
     */
    template <typename T>
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = Index;
        using pointer = const value_type*;
        using reference = const value_type&;

        Iterator(T* ptr = NULL, int stride = sizeof(T)): _ptr(ptr), _stride(stride) { }
        Iterator(const Iterator<T>& iterator) = default;
        ~Iterator() {}

        Iterator<T>& operator=(const Iterator<T>& iterator) = default;
        Iterator<T>& operator=(T* ptr) {
            _ptr = ptr;
            // stride is left unchanged
            return (*this);
        }

        operator bool() const {
            if (_ptr)
                return true;
            else
                return false;
        }

        bool operator==(const Iterator<T>& iterator) const { return (_ptr == iterator.getConstPtr()); }
        bool operator!=(const Iterator<T>& iterator) const { return (_ptr != iterator.getConstPtr()); }
        bool operator<(const Iterator<T>& iterator) const { return (_ptr < iterator.getConstPtr()); }
        bool operator>(const Iterator<T>& iterator) const { return (_ptr > iterator.getConstPtr()); }

        void movePtr(const Index& movement) {
            auto byteptr = ((Byte*)_ptr);
            byteptr += _stride * movement;
            _ptr = (T*)byteptr;
        }

        Iterator<T>& operator+=(const Index& movement) {
            movePtr(movement);
            return (*this);
        }
        Iterator<T>& operator-=(const Index& movement) {
            movePtr(-movement);
            return (*this);
        }
        Iterator<T>& operator++() {
            movePtr(1);
            return (*this);
        }
        Iterator<T>& operator--() {
            movePtr(-1);
            return (*this);
        }
        Iterator<T> operator++(Index) {
            auto temp(*this);
            movePtr(1);
            return temp;
        }
        Iterator<T> operator--(Index) {
            auto temp(*this);
            movePtr(-1);
            return temp;
        }
        Iterator<T> operator+(const Index& movement) {
            auto oldPtr = _ptr;
            movePtr(movement);
            auto temp(*this);
            _ptr = oldPtr;
            return temp;
        }
        Iterator<T> operator-(const Index& movement) {
            auto oldPtr = _ptr;
            movePtr(-movement);
            auto temp(*this);
            _ptr = oldPtr;
            return temp;
        }

        Index operator-(const Iterator<T>& iterator) { return (iterator.getPtr() - this->getPtr()) / sizeof(T); }

        T& operator*() { return *_ptr; }
        const T& operator*() const { return *_ptr; }
        T* operator->() { return _ptr; }

        T* getPtr() const { return _ptr; }
        const T* getConstPtr() const { return _ptr; }

    protected:
        T* _ptr;
        int _stride;
    };

#if 0
    // Direct memory access to the buffer contents is incompatible with the paging memory scheme
    template <typename T> Iterator<T> begin() { return Iterator<T>(&edit<T>(0), _stride); }
    template <typename T> Iterator<T> end() { return Iterator<T>(&edit<T>(getNum<T>()), _stride); }
#else
    template <typename T>
    Iterator<const T> begin() const {
        return Iterator<const T>(&get<T>(), _stride);
    }
    template <typename T>
    Iterator<const T> end() const {
        // reimplement get<T> without bounds checking
        Resource::Size elementOffset = getNum<T>() * _stride + _offset;
        return Iterator<const T>((reinterpret_cast<const T*>(_buffer->getData() + elementOffset)), _stride);
    }
#endif
    template <typename T>
    Iterator<const T> cbegin() const {
        return Iterator<const T>(&get<T>(), _stride);
    }
    template <typename T>
    Iterator<const T> cend() const {
        // reimplement get<T> without bounds checking
        Resource::Size elementOffset = getNum<T>() * _stride + _offset;
        return Iterator<const T>((reinterpret_cast<const T*>(_buffer->getData() + elementOffset)), _stride);
    }

    // the number of elements of the specified type fitting in the view size
    template <typename T>
    Index getNum() const {
        return Index(_size / _stride);
    }

    /**
     * @tparam T Type as which the data needs to be interpreted.
     * @return Read-only reference to BufferView data.
     */
    template <typename T>
    const T& get() const {
#if _DEBUG
        if (!_buffer) {
            qCDebug(gpulogging) << "Accessing null gpu::buffer!";
        }
        if (sizeof(T) > (_buffer->getSize() - _offset)) {
            qCDebug(gpulogging) << "Accessing buffer in non allocated memory, element size = " << sizeof(T)
                                << " available space in buffer at offset is = " << (_buffer->getSize() - _offset);
        }
        if (sizeof(T) > _size) {
            qCDebug(gpulogging) << "Accessing buffer outside the BufferView range, element size = " << sizeof(T)
                                << " when bufferView size = " << _size;
        }
#endif
        const T* t = (reinterpret_cast<const T*>(_buffer->getData() + _offset));
        return *(t);
    }

    /**
     * Get an editable reference to BufferView data and mark pages containing data as dirty.
     *
     * @tparam T Type as which the data needs to be interpreted.
     * @return Editable reference to BufferView data.
     */
    template <typename T>
    T& edit() {
#if _DEBUG
        if (!_buffer) {
            qCDebug(gpulogging) << "Accessing null gpu::buffer!";
        }
        if (sizeof(T) > (_buffer->getSize() - _offset)) {
            qCDebug(gpulogging) << "Accessing buffer in non allocated memory, element size = " << sizeof(T)
                                << " available space in buffer at offset is = " << (_buffer->getSize() - _offset);
        }
        if (sizeof(T) > _size) {
            qCDebug(gpulogging) << "Accessing buffer outside the BufferView range, element size = " << sizeof(T)
                                << " when bufferView size = " << _size;
        }
#endif
        _buffer->markDirty(_offset, sizeof(T));
        T* t = (reinterpret_cast<T*>(_buffer->editData() + _offset));
        return *(t);
    }

    /**
     * @tparam T Type as which the data needs to be interpreted.
     * @param index Index of the element to be retrieved.
     * @return Read-only reference to BufferView element at a given index.
     */
    template <typename T>
    const T& get(const Index index) const {
        Resource::Size elementOffset = index * _stride + _offset;
#if _DEBUG
        if (!_buffer) {
            qCDebug(gpulogging) << "Accessing null gpu::buffer!";
        }
        if (sizeof(T) > (_buffer->getSize() - elementOffset)) {
            qCDebug(gpulogging) << "Accessing buffer in non allocated memory, index = " << index
                                << ", element size = " << sizeof(T)
                                << " available space in buffer at offset is = " << (_buffer->getSize() - elementOffset);
        }
        if (index > getNum<T>()) {
            qCDebug(gpulogging) << "Accessing buffer outside the BufferView range, index = " << index
                                << " number elements = " << getNum<T>();
        }
#endif
        return *(reinterpret_cast<const T*>(_buffer->getData() + elementOffset));
    }

    /**
     * Get an editable reference to an element in BufferView with a given in data and mark pages containing data as dirty.
     *
     * @tparam T Type as which the data needs to be interpreted.
     * @param index Index of the element to be edited.
     * @return Editable reference to BufferView element at a given index.
     */
    template <typename T>
    T& edit(const Index index) const {
        Resource::Size elementOffset = index * _stride + _offset;
#if _DEBUG
        if (!_buffer) {
            qCDebug(gpulogging) << "Accessing null gpu::buffer!";
        }
        if (sizeof(T) > (_buffer->getSize() - elementOffset)) {
            qCDebug(gpulogging) << "Accessing buffer in non allocated memory, index = " << index
                                << ", element size = " << sizeof(T)
                                << " available space in buffer at offset is = " << (_buffer->getSize() - elementOffset);
        }
        if (index > getNum<T>()) {
            qCDebug(gpulogging) << "Accessing buffer outside the BufferView range, index = " << index
                                << " number elements = " << getNum<T>();
        }
#endif
        _buffer->markDirty(elementOffset, sizeof(T));
        return *(reinterpret_cast<T*>(_buffer->editData() + elementOffset));
    }
};

/**
 * @brief A buffer contaning a single structure.
 *
 * Typically used for uniforms.
 * @tparam T Type of the structure that the buffer contains.
 */
template <class T>
class StructBuffer : public gpu::BufferView {
public:
    /**
     * @brief Creates a new buffer containing a give structure.
     *
     * Typically used for uniforms.
     * @tparam U Type of the structure that the buffer contains.
     * @param usage A bit sum of `Buffer::Usage` flags describing this buffer.
     * @return Shared pointer to the created buffer.
     */
    template <class U>
    static BufferPointer makeBuffer(uint32_t usage = gpu::Buffer::UniformBuffer) {
        U t;
        return std::make_shared<gpu::Buffer>(usage, sizeof(U), (const gpu::Byte*)&t, sizeof(U));
    }
    ~StructBuffer(){};
    StructBuffer() : gpu::BufferView(makeBuffer<T>()) {}

    /**
     * @brief Mark data as dirty and get a writeable reference to the stored structure.
     *
     * @return Writeable reference to the stored structure.
     */
    T& edit() { return BufferView::edit<T>(0); }

    /**
     * @return Read-only reference to the stored structure.
     */
    const T& get() const { return BufferView::get<T>(0); }

    const T* operator->() const { return &get(); }
};
};  // namespace gpu

#endif
