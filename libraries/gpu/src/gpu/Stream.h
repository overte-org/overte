//
//  Stream.h
//  interface/src/gpu
//
//  Created by Sam Gateau on 10/29/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Stream_h
#define hifi_gpu_Stream_h

#include <vector>
#include <map>
#include <array>
#include <string>

#include <assert.h>

#include "Resource.h"
#include "Format.h"

namespace gpu {

class Element;

// Stream namespace class
class Stream {
public:

    /// Possible input slots identifiers
    enum InputSlot {
        POSITION = 0,
        NORMAL = 1,
        COLOR = 2,
        TEXCOORD0 = 3,
        TEXCOORD = TEXCOORD0,
        TANGENT = 4,
        SKIN_CLUSTER_INDEX = 5,
        SKIN_CLUSTER_WEIGHT = 6,
        TEXCOORD1 = 7,
        TEXCOORD2 = 8,
        TEXCOORD3 = 9,
        TEXCOORD4 = 10,
        FADE1 = 7,
        FADE2 = 8,
        FADE3 = 9,
        FADE4 = 10,
        FADE5 = 11,
        FADE6 = 12,
        FADE7 = 13,

        NUM_INPUT_SLOTS,

        DRAW_CALL_INFO = 15, // Reserve last input slot for draw call infos
    };

    typedef uint8 Slot;

    static const std::array<Element, InputSlot::NUM_INPUT_SLOTS>& getDefaultElements();

    /// How often source of the data is incremented by stride.
    enum Frequency {
        PER_VERTEX = 0,
        PER_INSTANCE = 1,
    };

    /// The attribute description.
    /// Every thing that is needed to detail a stream attribute and how to interpret it.
    class Attribute {
    public:
        Attribute() {}

        /**
         * @param slot Slot on which data will be available to the shader.
         * @param channel Binding number which this attribute gets data from.
         * @param element Object describing element type.
         * @param offset Offset in bytes.
         * @param frequency When is current position incremented by stride.
         */
        Attribute(Slot slot, Slot channel, Element element, Offset offset = 0, Frequency frequency = PER_VERTEX) :
            _slot(slot),
            _channel(channel),
            _element(element),
            _offset(offset),
            _frequency(frequency)
        {}

        Slot _slot{ POSITION }; // Logical slot assigned to the attribute
        Slot _channel{ POSITION }; // index of the channel where to get the data from
        Element _element{ Element::VEC3F_XYZ };
        Offset _offset{ 0 };
        uint32 _frequency{ PER_VERTEX };

        /**
         * @return Size of the attribute in bytes.
         */
        uint32 getSize() const { return _element.getSize(); }

        /**
         * @brief Generate a string key describing the attribute uniquely.
         * @return A string key.
         */
        std::string getKey() const;
    };

    /// Stream Format is describing how to feed a list of attributes from a bunch of stream buffer channels
    class Format {
    public:
        typedef std::map< Slot, Attribute > AttributeMap;

        /// Contains information about single channel.
        class ChannelInfo {
        public:

            /// Which slots use this channel.
            std::vector< Slot > _slots;
            Offset _stride;

            /// Size of all attributes in this channel.
            uint32 _netSize;

            /// How often current position is incremented.
            uint32 _frequency{ PER_VERTEX };

            ChannelInfo() : _stride(0), _netSize(0) {}
        };
        typedef std::map< Slot, ChannelInfo > ChannelMap;

        /**
         * @return Number of attributes for this format.
         */
        size_t getNumAttributes() const { return _attributes.size(); }

        /**
         * @return Attributes mapped to slots.
         */
        const AttributeMap& getAttributes() const { return _attributes; }

        /**
         * @return Number of channels for this format.
         */
        size_t getNumChannels() const { return _channels.size(); }

        /**
         * @return Channels this format.
         */
        const ChannelMap& getChannels() const { return _channels; }

        /**
         * @param channel Binding number for a given data source.
         * @return Stride for a channel in a given binding location.
         */
        Offset getChannelStride(Slot channel) const { return _channels.at(channel)._stride; }

        /**
         * @return
         */
        size_t getElementTotalSize() const { return _elementTotalSize; }

        /**
         * @brief Sets attribute in a format.
         *
         * After adding attribute calls `evaluateCache()` to update channels map.
         * @param slot Slot at which data will be available to the shader.
         * @param channel Binding number for a given data source.
         * @param element Object containing information about the element type of this attribute.
         * @param offset Offset in bytes for the given element.
         * @param frequency How often data position is incremented. Must be the same for entire channel.
         * @return Always returns true currently.
         */
        bool setAttribute(Slot slot, Slot channel, Element element, Offset offset = 0, Frequency frequency = PER_VERTEX);

        /**
         * @param slot Slot index to check.
         * @return Returns true if there's an attribute on a given slot.
         */
        bool hasAttribute(Slot slot) const { return (_attributes.find(slot) != _attributes.end()); }

        /**
         * Attribute object with default constructor if there's no attribute at the given slot.
         * @param slot Slot index to get attribute from.
         * @return Attribute at a given slot.
         */
        Attribute getAttribute(Slot slot) const;

        /**
         * @return Gets a string uniquely representing this format.
         */
        const std::string& getKey() const { return _key; }

        /// GPU backend-specific object representing format.
        const GPUObjectPointer gpuObject{};

    protected:
        /// Attributes mapped to slots.
        AttributeMap _attributes;

        /// Channels mapped to binding indices.
        ChannelMap _channels;

        /// Sum of the size of all attributes in bytes.
        uint32 _elementTotalSize { 0 };

        /// String uniquely describing a format. Automatically updated when adding attributes.
        std::string _key;

        friend class Serializer;
        friend class Deserializer;

        /**
         * Updates key, channel map and total size.
         * Automatically called when adding attributes.
         */
        void evaluateCache();
    };

    typedef std::shared_ptr<Format> FormatPointer;
};

typedef std::vector< Offset > Offsets;

/// Buffer Stream is a container of N Buffers and their respective Offsets and Strides representing N consecutive channels.
/// A Buffer Stream can be assigned to the Batch to set several stream channels in one call.
class BufferStream {
public:
    using Strides = Offsets;

    /**
     * Clears all entries for the buffer stream.
     */
    void clear() { _buffers.clear(); _offsets.clear(); _strides.clear(); }

    /**
     * @param buffer Shared pointer to the buffer that will be added.
     * @param offset Offset in bytes where the data start.
     * @param stride Stride - size in bytes by which puffer position is incremented for consecutive elements.
     */
    void addBuffer(const BufferPointer& buffer, Offset offset, Offset stride);

    /**
     * @return Reference to a vector of pointers of buffers for this buffer stream.
     */
    const Buffers& getBuffers() const { return _buffers; }

    /**
     * @return Reference to a vector of offsets in bytes. Offsets have same indices as buffers from `getBuffers()`.
     */
    const Offsets& getOffsets() const { return _offsets; }

    /**
     * @return Reference to a vector of strides in bytes. Strides have same indices as buffers from `getBuffers()`.
     */
    const Strides& getStrides() const { return _strides; }

    /**
     * @return Returns number of buffers in this buffer stream.
     */
    size_t getNumBuffers() const { return _buffers.size(); }

    BufferStream& operator = (const BufferStream& src) = default;

protected:
    Buffers _buffers;
    Offsets _offsets;
    Strides _strides;
};
typedef std::shared_ptr<BufferStream> BufferStreamPointer;

};


#endif
