//
//  SerDes.h
//
//
//  Created by Dale Glass on 5/6/2022
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#include <string>
#include <cstring>
#include <QtCore/QByteArray>
#include <QDebug>
#include <glm/glm.hpp>

/**
 * @brief Data serializer
 *
 * When encoding, this class takes in data and encodes it into a buffer. No attempt is made to store version numbers, lengths,
 * or any other metadata. It's entirely up to the user to use the class in such a way that the process can be
 * correctly reversed if variable-length or optional fields are used.
 *
 * It can operate both on an internal, dynamically-allocated buffer, or an externally provided, fixed-size one.
 * If an external store is used, the class will refuse to add data once capacity is reached and set the overflow flag.
 * When decoding, this class operates on a fixed size buffer. If an attempt to read past the end is made, the read fails,
 * and the overflow flag is set.
 *
 * The class was written for the maximum simplicity possible and inline friendliness.
 *
 * Example of encoding:
 *
 * @code {.cpp}
 * uint8_t version = 1;
 * uint16_t count = 1;
 * glm::vec3 pos{1.5, 2.0, 9.0};
 *
 * Serializer ser;
 * ser << version;
 * ser << count;
 * ser << pos;
 *
 * // Serialized data is in ser.buffer(), ser.length() long.
 * @endcode
 *
 * This object should be modified directly to add support for any primitive and common datatypes in the code. To support serializing/deserializing
 * classes and structures, implement a `operator<<` and `operator>>` functions for that object, eg:
 *
 * @code {.cpp}
 * DataSerializer &operator<<(DataSerializer &ser, const Object &o) {
 *  ser << o._borderColor;
 *  ser << o._maxAnisotropy;
 *  ser << o._filter;
 *  return ser;
 * }
 * @endcode
 *
 */
class DataSerializer {
    public:

        /**
         * @brief RAII tracker of advance position
         *
         * When a custom operator<< is implemented for DataSserializer,
         * this class allows to easily keep track of how much data has been added and
         * adjust the parent's lastAdvance() count on this class' destruction.
         *
         * @code {.cpp}
         * DataSerializer &operator<<(DataSerializer &ser, const Object &o) {
         *  DataSerializer::SizeTracker tracker(ser);
         *
         *  ser << o._borderColor;
         *  ser << o._maxAnisotropy;
         *  ser << o._filter;
         *  return ser;
         * }
         * @endcode
         */
        class SizeTracker {
            public:
            SizeTracker(DataSerializer &parent) : _parent(parent) {
                _start_pos = _parent.pos();
            }

            ~SizeTracker() {
                size_t cur_pos = _parent.pos();

                if ( cur_pos >= _start_pos ) {
                    _parent._lastAdvance = cur_pos - _start_pos;
                } else {
                    _parent._lastAdvance = 0;
                }
            }

            private:
            DataSerializer &_parent;
            size_t _start_pos = 0;
        };

        /**
         * @brief Default size for a dynamically allocated buffer.
         *
         * Since this is mostly intended to be used for networking, we default to the largest probable MTU here.
         */
        static const int  DEFAULT_SIZE = 1500;

        /**
         * @brief Character to use for padding.
         *
         * Padding should be ignored, so it doesn't matter what we go with here, but it can be useful to set it
         * to something that would be distinctive in a dump.
         */
        static const char PADDING_CHAR = 0xAA;

        /**
         * @brief Construct a dynamically allocated serializer
         *
         * If constructed this way, an internal buffer will be dynamically allocated and grown as needed.
         *
         * The buffer is SerDes::DEFAULT_SIZE bytes by default, and doubles in size every time the limit is reached.
         */
        DataSerializer() {
            _capacity = DEFAULT_SIZE;
            _pos = 0;
            _length = 0;
            _store = new char[_capacity];
        }

        /**
         * @brief Construct a statically allocated serializer
         *
         * If constructed this way, the external buffer will be used to store data. The class will refuse to
         * keep adding data if the maximum length is reached, write a critical message to the log, and set
         * the overflow flag.
         *
         * The flag can be read with isOverflow()
         *
         * @param externalStore External data store
         * @param storeLength Length of the data store
         */
        DataSerializer(char *externalStore, size_t storeLength) {
            _capacity = storeLength;
            _length = 0;
            _pos = 0;
            _storeIsExternal = true;
            _store = externalStore;
        }

        /**
         * @brief Construct a statically allocated serializer
         *
         * If constructed this way, the external buffer will be used to store data. The class will refuse to
         * keep adding data if the maximum length is reached, and set the overflow flag.
         *
         * The flag can be read with isOverflow()
         *
         * @param externalStore External data store
         * @param storeLength Length of the data store
         */
        DataSerializer(uint8_t *externalStore, size_t storeLength) : DataSerializer((char*)externalStore, storeLength) {

        }

        DataSerializer(const DataSerializer &) = delete;
        DataSerializer &operator=(const DataSerializer &) = delete;



        ~DataSerializer() {
            if (!_storeIsExternal) {
                delete[] _store;
            }
        }

        /**
         * @brief Adds padding to the output
         *
         * The bytes will be set to SerDes::PADDING_CHAR, which is a constant in the source code.
         * Since padding isn't supposed to be read, it can be any value and is intended to
         * be set to something that can be easily recognized in a dump.
         *
         * @param bytes Number of bytes to add
         */
        void addPadding(size_t bytes) {
            if (!extendBy(bytes, "padding")) {
                return;
            }

            // Fill padding with something recognizable. Will keep valgrind happier.
            memset(&_store[_pos], PADDING_CHAR, bytes);
            _pos += bytes;
        }

        /**
         * @brief Add an uint8_t to the output
         *
         * @param c  Character to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(uint8_t c) {
            return *this << int8_t(c);
        }

        /**
         * @brief Add an int8_t to the output
         *
         * @param c  Character to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(int8_t c) {
            if (!extendBy(1, "int8_t")) {
                return *this;
            }

            _store[_pos++] = c;
            return *this;
        }


        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an uint16_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(uint16_t val) {
            return *this << int16_t(val);
        }

        /**
         * @brief Add an int16_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(int16_t val) {
            if (!extendBy(sizeof(val), "int16_t")) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }



        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an uint32_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(uint32_t val) {
            return *this << int32_t(val);
        }

        /**
         * @brief Add an int32_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(int32_t val) {
            if (!extendBy(sizeof(val), "int32_t")) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an uint64_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(uint64_t val) {
            return *this << int64_t(val);
        }

        /**
         * @brief Add an int64_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(int64_t val) {
            if (!extendBy(sizeof(val), "int64_t")) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an float to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(float val) {
            if (extendBy(sizeof(val), "float")) {
                memcpy(&_store[_pos], (char*)&val, sizeof(val));
                _pos += sizeof(val);
            }
            return *this;
        }


        ///////////////////////////////////////////////////////////


        /**
         * @brief Add an glm::vec3 to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(glm::vec3 val) {
            size_t sz = sizeof(val.x);
            if (extendBy(sz*3, "glm::vec3")) {
                memcpy(&_store[_pos       ], (char*)&val.x, sz);
                memcpy(&_store[_pos + sz  ], (char*)&val.y, sz);
                memcpy(&_store[_pos + sz*2], (char*)&val.z, sz);

                _pos += sz*3;
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Add a glm::vec4 to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(glm::vec4 val) {
            size_t sz = sizeof(val.x);
            if (extendBy(sz*4, "glm::vec4")) {
                memcpy(&_store[_pos       ], (char*)&val.x, sz);
                memcpy(&_store[_pos + sz  ], (char*)&val.y, sz);
                memcpy(&_store[_pos + sz*2], (char*)&val.z, sz);
                memcpy(&_store[_pos + sz*3], (char*)&val.w, sz);

                _pos += sz*4;
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Add a glm::ivec2 to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        DataSerializer &operator<<(glm::ivec2 val) {
            size_t sz = sizeof(val.x);
            if (extendBy(sz*2, "glm::ivec2")) {
                memcpy(&_store[_pos       ], (char*)&val.x, sz);
                memcpy(&_store[_pos + sz  ], (char*)&val.y, sz);

                _pos += sz*2;
            }
            return *this;
        }


        ///////////////////////////////////////////////////////////

        /**
         * @brief Write a null-terminated string into the buffer
         *
         * The `\0` at the end of the string is also written.
         *
         * @param val Value to write
         * @return SerDes& This object
         */
        DataSerializer &operator<<(const char *val) {
            size_t len = strlen(val)+1;
            if (extendBy(len, "string")) {
                memcpy(&_store[_pos], val, len);
                _pos += len;
            }
            return *this;
        }

        /**
         * @brief Write a QString into the buffer
         *
         * The string is encoded in UTF-8 and the `\0` at the end of the string is also written.
         *
         * @param val Value to write
         * @return SerDes& This object
         */
        DataSerializer &operator<<(const QString &val) {
            return *this << val.toUtf8().constData();
        }


        ///////////////////////////////////////////////////////////

        /**
         * @brief Pointer to the start of the internal buffer.
         *
         * The allocated amount can be found with capacity().
         *
         * The end of the stored data can be found with length().
         *
         * @return Pointer to buffer
         */
        char *buffer() const { return _store; }

        /**
         * @brief Current position in the buffer. Starts at 0.
         *
         * @return size_t
         */
        size_t pos() const { return _pos; }

        /**
         * @brief Last position that was written to in the buffer. Starts at 0.
         *
         * @return size_t
         */
        size_t length() const { return _length; }

        /**
         * @brief Current capacity of the buffer.
         *
         * If the buffer is dynamically allocated, it can grow.
         *
         * If the buffer is static, this is a fixed limit.
         *
         * @return size_t
         */
        size_t capacity() const { return _capacity; }

        /**
         * @brief Whether there's any data in the buffer
         *
         * @return true Something has been written
         * @return false The buffer is empty
         */
        bool isEmpty() const { return _length == 0; }

        /**
         * @brief The buffer size limit has been reached
         *
         * This can only return true for a statically allocated buffer.
         *
         * @return true Limit reached
         * @return false There is still room
         */
        bool isOverflow() const { return _overflow; }

        /**
         * @brief Reset the serializer to the start, clear overflow bit.
         *
         */
        void rewind() { _pos = 0; _overflow = false; _lastAdvance = 0; }


        /**
         * @brief Size of the last advance
         *
         * This can be used to get how many bytes were added in the last operation.
         * It is reset on rewind()
         *
         * @return size_t
         */
        size_t lastAdvance() const { return _lastAdvance; }

        /**
         * @brief Dump the contents of this object into QDebug
         *
         * This produces a dump of the internal state, and an ASCII/hex dump of
         * the contents, for debugging.
         *
         * @param debug Qt QDebug stream
         * @param ds  This object
         * @return QDebug
         */
        friend QDebug operator<<(QDebug debug, const DataSerializer &ds);

    private:
        bool extendBy(size_t bytes, const QString &type_name) {
            //qDebug() << "Extend by" << bytes;

            if ( _capacity < _length + bytes) {
                if ( _storeIsExternal ) {
                    qCritical() << "Serializer trying to write past end of output buffer of" << _capacity << "bytes. Error writing" << bytes << "bytes for" << type_name << " from position " << _pos << ", length " << _length;
                    _overflow = true;
                    return false;
                }

                changeAllocation(_length + bytes);
            }

            _length += bytes;
            _lastAdvance = bytes;
            return true;
        }

        // This is split up here to try to make the class as inline-friendly as possible.
        void changeAllocation(size_t new_size);

        char *_store;
        bool _storeIsExternal = false;
        bool _overflow = false;
        size_t _capacity = 0;
        size_t _length = 0;
        size_t _pos = 0;
        size_t _lastAdvance = 0;
};

/**
 * @brief Data deserializer
 *
 * This class operates on a fixed size buffer. If an attempt to read past the end is made, the read fails,
 * and the overflow flag is set.
 *
 * The class was written for the maximum simplicity possible and inline friendliness.
 *
 * Example of decoding:
 *
 * @code {.cpp}
 * // Incoming data has been placed in:
 * // char buffer[1024];
 *
 * uint8_t version;
 * uint16_t count;
 * glm::vec3 pos;
 *
 * DataDeserializer des(buffer, sizeof(buffer));
 * des >> version;
 * des >> count;
 * des >> pos;
 * @endcode
 *
 * This object should be modified directly to add support for any primitive and common datatypes in the code. To support deserializing
 * classes and structures, implement an `operator>>` function for that object, eg:
 *
 * @code {.cpp}
 * DataDeserializer &operator>>(DataDeserializer &des, Object &o) {
 *  des >> o._borderColor;
 *  des >> o._maxAnisotropy;
 *  des >> o._filter;
 *  return des;
 * }
 * @endcode
 *
 */
class DataDeserializer {
    public:

        /**
         * @brief RAII tracker of advance position
         *
         * When a custom operator>> is implemented for DataDeserializer,
         * this class allows to easily keep track of how much data has been added and
         * adjust the parent's lastAdvance() count on this class' destruction.
         *
         * @code {.cpp}
         * DataDeserializer &operator>>(Deserializer &des, Object &o) {
         *  DataDeserializer::SizeTracker tracker(des);
         *
         *  des >> o._borderColor;
         *  des >> o._maxAnisotropy;
         *  des >> o._filter;
         *  return des;
         * }
         * @endcode
         */
        class SizeTracker {
            public:
            SizeTracker(DataDeserializer &parent) : _parent(parent) {
                _start_pos = _parent.pos();
            }

            ~SizeTracker() {
                size_t cur_pos = _parent.pos();

                if ( cur_pos >= _start_pos ) {
                    _parent._lastAdvance = cur_pos - _start_pos;
                } else {
                    _parent._lastAdvance = 0;
                }
            }

            private:
            DataDeserializer &_parent;
            size_t _start_pos = 0;
        };

        /**
         * @brief Construct a Deserializer
         *         *
         * @param externalStore External data store
         * @param storeLength Length of the data store
         */
        DataDeserializer(const char *externalStore, size_t storeLength) {
            _length = storeLength;
            _pos = 0;
            _store = externalStore;
            _lastAdvance = 0;
        }

        /**
         * @brief Construct a Deserializer
         *
         * @param externalStore External data store
         * @param storeLength Length of the data store
         */
        DataDeserializer(const uint8_t *externalStore, size_t storeLength) : DataDeserializer((const char*)externalStore, storeLength) {

        }

        /**
         * @brief Construct a new Deserializer reading data from a Serializer
         *
         * This is a convenience function for testing.
         *
         * @param serializer Serializer with data
         */
        DataDeserializer(const DataSerializer &serializer) : DataDeserializer(serializer.buffer(), serializer.length()) {

        }

        /**
         * @brief Skips padding in the input
         *
         * @param bytes Number of bytes to skip
         */
        void skipPadding(size_t bytes) {
            if (!canAdvanceBy(bytes, "padding")) {
                return;
            }

            _pos += bytes;
            _lastAdvance = bytes;
        }


        /**
         * @brief Read an uint8_t from the buffer
         *
         * @param c Character to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(uint8_t &c) {
            return *this >> reinterpret_cast<int8_t&>(c);
        }

        /**
         * @brief Read an int8_t from the buffer
         *
         * @param c Character to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(int8_t &c) {
            if ( canAdvanceBy(1, "int8_t") ) {
                c = _store[_pos++];
                _lastAdvance = sizeof(c);
            }

            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Read an uint16_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(uint16_t &val) {
            return *this >> reinterpret_cast<int16_t&>(val);
        }

        /**
         * @brief Read an int16_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(int16_t &val) {
            if ( canAdvanceBy(sizeof(val), "int16_t") ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
                _lastAdvance = sizeof(val);
            }

            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Read an uint32_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(uint32_t &val) {
            return *this >> reinterpret_cast<int32_t&>(val);
        }

        /**
         * @brief Read an int32_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(int32_t &val) {
            if ( canAdvanceBy(sizeof(val), "int32_t") ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
                _lastAdvance = sizeof(val);
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Read an uint64_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(uint64_t &val) {
            return *this >> reinterpret_cast<int64_t&>(val);
        }

        /**
         * @brief Read an int64_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(int64_t &val) {
            if ( canAdvanceBy(sizeof(val), "int64_t") ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
                _lastAdvance = sizeof(val);
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////


        /**
         * @brief Read an float from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(float &val) {
            if ( canAdvanceBy(sizeof(val), "float") ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
                _lastAdvance = sizeof(val);
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////




        /**
         * @brief Read a glm::vec3 from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(glm::vec3 &val) {
            size_t sz = sizeof(val.x);

            if ( canAdvanceBy(sz*3, "glm::vec3") ) {
                memcpy((char*)&val.x, &_store[_pos       ], sz);
                memcpy((char*)&val.y, &_store[_pos + sz  ], sz);
                memcpy((char*)&val.z, &_store[_pos + sz*2], sz);

                _pos += sz*3;
                _lastAdvance = sz * 3;
            }

            return *this;
        }

        ///////////////////////////////////////////////////////////


        /**
         * @brief Read a glm::vec4 from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(glm::vec4 &val) {
            size_t sz = sizeof(val.x);

            if ( canAdvanceBy(sz*4, "glm::vec4")) {
                memcpy((char*)&val.x, &_store[_pos       ], sz);
                memcpy((char*)&val.y, &_store[_pos + sz  ], sz);
                memcpy((char*)&val.z, &_store[_pos + sz*2], sz);
                memcpy((char*)&val.w, &_store[_pos + sz*3], sz);

                _pos += sz*4;
                _lastAdvance = sz*4;
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////


        /**
         * @brief Read a glm::ivec2 from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        DataDeserializer &operator>>(glm::ivec2 &val) {
            size_t sz = sizeof(val.x);

            if ( canAdvanceBy(sz*2, "glm::ivec2") ) {
                memcpy((char*)&val.x, &_store[_pos       ], sz);
                memcpy((char*)&val.y, &_store[_pos + sz  ], sz);

                _pos += sz*2;
                _lastAdvance = sz * 2;
            }

            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Pointer to the start of the internal buffer.
         *
         * The allocated amount can be found with capacity().
         *
         * The end of the stored data can be found with length().
         *
         * @return Pointer to buffer
         */
        const char *buffer() const { return _store; }

        /**
         * @brief Current position in the buffer. Starts at 0.
         *
         * @return size_t
         */
        size_t pos() const { return _pos; }

        /**
         * @brief Last position that was written to in the buffer. Starts at 0.
         *
         * @return size_t
         */
        size_t length() const { return _length; }

        /**
         * @brief Whether there's any data in the buffer
         *
         * @return true Something has been written
         * @return false The buffer is empty
         */
        bool isEmpty() const { return _length == 0; }

        /**
         * @brief The buffer size limit has been reached
         *
         * This can only return true for a statically allocated buffer.
         *
         * @return true Limit reached
         * @return false There is still room
         */
        bool isOverflow() const { return _overflow; }

        /**
         * @brief Reset the serializer to the start, clear overflow bit.
         *
         */
        void rewind() { _pos = 0; _overflow = false; _lastAdvance = 0; }

        /**
         * @brief Size of the last advance
         *
         * This can be used to get how many bytes were added in the last operation.
         * It is reset on rewind()
         *
         * @return size_t
         */
        size_t lastAdvance() const { return _lastAdvance; }

        /**
         * @brief Dump the contents of this object into QDebug
         *
         * This produces a dump of the internal state, and an ASCII/hex dump of
         * the contents, for debugging.
         *
         * @param debug Qt QDebug stream
         * @param ds  This object
         * @return QDebug
         */
        friend QDebug operator<<(QDebug debug, const DataDeserializer &ds);

    private:
        bool canAdvanceBy(size_t bytes, const QString &type_name) {
            //qDebug() << "Checking advance by" << bytes;

            if ( _length < _pos + bytes) {
                qCritical() << "Deserializer trying to read past end of input buffer of" << _length << "bytes, reading" << bytes << "bytes for" << type_name << "from position " << _pos;
                _overflow = true;
                return false;
            }

            return true;
        }

        const char *_store;
        bool _overflow = false;
        size_t _length = 0;
        size_t _pos = 0;
        size_t _lastAdvance = 0;
};
