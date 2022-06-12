//
//  SerDes.h
//
//
//  Created by Dale Glass on 5/6/2022
//  Copyright 2022 Dale Glass
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
 * @brief Data serializer/deserializer
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
 * SerDes ser;
 * ser << version;
 * ser << count;
 * ser << pos;
 *
 * // Serialized data is in ser.buffer(), ser.length() long.
 * @endcode
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
 * SerDes des(buffer, sizeof(buffer));
 * des >> version;
 * des >> count;
 * des >> pos;
 * @endcode
 *
 * This object should be modified directly to add support for any primitive and common datatypes in the code. To support serializing/deserializing
 * classes and structures, implement a `operator<<` and `operator>>` functions for that object, eg:
 *
 * @code {.cpp}
 * SerDes &operator<<(SerDes &ser, const Object &o) {
 *  ser << o._borderColor;
 *  ser << o._maxAnisotropy;
 *  ser << o._filter;
 *  return ser;
 * }
 *
 * SerDes &operator>>(SerDes &des, Object &o) {
 *  des >> o._borderColor;
 *  des >> o._maxAnisotropy;
 *  des >> o._filter;
 *  return des;
 * }
 *
 * @endcode
 *
 */
class SerDes {
    public:
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
        SerDes() {
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
        SerDes(char *externalStore, size_t storeLength) {
            _capacity = storeLength;
            _length = storeLength;
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
        SerDes(uint8_t *externalStore, size_t storeLength) : SerDes((char*)externalStore, storeLength) {

        }

        SerDes(const SerDes &) = delete;
        SerDes &operator=(const SerDes &) = delete;



        ~SerDes() {
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
            if (!extendBy(bytes)) {
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
        SerDes &operator<<(uint8_t c) {
            return *this << int8_t(c);
        }

        /**
         * @brief Add an int8_t to the output
         *
         * @param c  Character to add
         * @return SerDes& This object
         */
        SerDes &operator<<(int8_t c) {
            if (!extendBy(1)) {
                return *this;
            }

            _store[_pos++] = c;
            return *this;
        }

        /**
         * @brief Read an uint8_t from the buffer
         *
         * @param c Character to read
         * @return SerDes& This object
         */
        SerDes &operator>>(uint8_t &c) {
            return *this >> reinterpret_cast<int8_t&>(c);
        }

        /**
         * @brief Read an int8_t from the buffer
         *
         * @param c Character to read
         * @return SerDes& This object
         */
        SerDes &operator>>(int8_t &c) {
            if ( _pos < _length ) {
                c = _store[_pos++];
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading 8 bits from position " << _pos << ", length " << _length;
            }

            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an uint16_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        SerDes &operator<<(uint16_t val) {
            return *this << int16_t(val);
        }

        /**
         * @brief Add an int16_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        SerDes &operator<<(int16_t val) {
            if (!extendBy(sizeof(val))) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        /**
         * @brief Read an uint16_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(uint16_t &val) {
            return *this >> reinterpret_cast<int16_t&>(val);
        }

        /**
         * @brief Read an int16_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(int16_t &val) {
            if ( _pos + sizeof(val) <= _length ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading 16 bits from position " << _pos << ", length " << _length;
            }

            return *this;
        }

        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an uint32_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        SerDes &operator<<(uint32_t val) {
            return *this << int32_t(val);
        }

        /**
         * @brief Add an int32_t to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        SerDes &operator<<(int32_t val) {
            if (!extendBy(sizeof(val))) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        /**
         * @brief Read an uint32_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(uint32_t &val) {
            return *this >> reinterpret_cast<int32_t&>(val);
        }

        /**
         * @brief Read an int32_t from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(int32_t &val) {
            if ( _pos + sizeof(val) <= _length ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading 32 bits from position " << _pos << ", length " << _length;
            }
            return *this;
        }


        ///////////////////////////////////////////////////////////

        /**
         * @brief Add an float to the output
         *
         * @param val  Value to add
         * @return SerDes& This object
         */
        SerDes &operator<<(float val) {
            if (!extendBy(sizeof(val))) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        /**
         * @brief Read an float from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(float &val) {
            if ( _pos + sizeof(val) <= _length ) {
                memcpy((char*)&val, &_store[_pos], sizeof(val));
                _pos += sizeof(val);
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading float from position " << _pos << ", length " << _length;
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
        SerDes &operator<<(glm::vec3 val) {
            size_t sz = sizeof(val.x);
            if (!extendBy(sz*3)) {
                return *this;
            }

            memcpy(&_store[_pos       ], (char*)&val.x, sz);
            memcpy(&_store[_pos + sz  ], (char*)&val.y, sz);
            memcpy(&_store[_pos + sz*2], (char*)&val.z, sz);

            _pos += sz*3;
            return *this;
        }

        /**
         * @brief Read a glm::vec3 from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(glm::vec3 &val) {
            size_t sz = sizeof(val.x);

            if ( _pos + sz*3 <= _length ) {
                memcpy((char*)&val.x, &_store[_pos       ], sz);
                memcpy((char*)&val.y, &_store[_pos + sz  ], sz);
                memcpy((char*)&val.z, &_store[_pos + sz*2], sz);

                _pos += sz*3;
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading glm::vec3 from position " << _pos << ", length " << _length;
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
        SerDes &operator<<(glm::vec4 val) {
            size_t sz = sizeof(val.x);
            if (!extendBy(sz*4)) {
                return *this;
            }

            memcpy(&_store[_pos       ], (char*)&val.x, sz);
            memcpy(&_store[_pos + sz  ], (char*)&val.y, sz);
            memcpy(&_store[_pos + sz*2], (char*)&val.z, sz);
            memcpy(&_store[_pos + sz*3], (char*)&val.w, sz);

            _pos += sz*4;
            return *this;
        }

        /**
         * @brief Read a glm::vec4 from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(glm::vec4 &val) {
            size_t sz = sizeof(val.x);

            if ( _pos + sz*4 <= _length ) {
                memcpy((char*)&val.x, &_store[_pos       ], sz);
                memcpy((char*)&val.y, &_store[_pos + sz  ], sz);
                memcpy((char*)&val.z, &_store[_pos + sz*2], sz);
                memcpy((char*)&val.w, &_store[_pos + sz*3], sz);

                _pos += sz*4;
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading glm::vec3 from position " << _pos << ", length " << _length;
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
        SerDes &operator<<(glm::ivec2 val) {
            size_t sz = sizeof(val.x);
            if (!extendBy(sz*2)) {
                return *this;
            }

            memcpy(&_store[_pos       ], (char*)&val.x, sz);
            memcpy(&_store[_pos + sz  ], (char*)&val.y, sz);

            _pos += sz*2;
            return *this;
        }

        /**
         * @brief Read a glm::ivec2 from the buffer
         *
         * @param val Value to read
         * @return SerDes& This object
         */
        SerDes &operator>>(glm::ivec2 &val) {
            size_t sz = sizeof(val.x);

            if ( _pos + sz*2 <= _length ) {
                memcpy((char*)&val.x, &_store[_pos       ], sz);
                memcpy((char*)&val.y, &_store[_pos + sz  ], sz);

                _pos += sz*2;
            } else {
                _overflow = true;
                qCritical() << "Deserializer trying to read past end of input, reading glm::ivec2 from position " << _pos << ", length " << _length;
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
        SerDes &operator<<(const char *val) {
            size_t len = strlen(val)+1;
            extendBy(len);
            memcpy(&_store[_pos], val, len);
            _pos += len;
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
        SerDes &operator<<(const QString &val) {
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
        void rewind() { _pos = 0; _overflow = false; }

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
        friend QDebug operator<<(QDebug debug, const SerDes &ds);

    private:
        bool extendBy(size_t bytes) {
            //qDebug() << "Extend by" << bytes;

            if ( _capacity < _length + bytes) {
                if ( _storeIsExternal ) {
                    _overflow = true;
                    return false;
                }

                changeAllocation(_length + bytes);
            }

            _length += bytes;
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
};
