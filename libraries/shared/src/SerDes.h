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
 *
 * If an external store is used, the class will refuse to add data once capacity is reached and set the overflow flag.
 *
 * When decoding, this class operates on a fixed size buffer. If an attempt to read past the end is made, the read fails,
 * and the overflow flag is set.
 *
 * The class was written for the maximum simplicity possible and inline friendliness.
 */
class SerDes {
    public:
        // This class is aimed at network serialization, so we assume we're going to deal
        // with something MTU-sized by default.
        static const int  DEFAULT_SIZE = 1500;
        static const char PADDING_CHAR = 0xAA;

        /**
         * @brief Construct a dynamically allocated serializer
         *
         * If constructed this way, an internal buffer will be dynamically allocated and grown as needed.
         *
         * The default buffer size is 1500 bytes, based on the assumption that it will be used to construct
         * network packets.
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
         * keep adding data if the maximum length is reached, and set the overflow flag.
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

        SerDes(uint8_t *externalStore, size_t storeLength) : SerDes((char*)externalStore, storeLength) {

        }

        SerDes(const SerDes &) = delete;
        SerDes &operator=(const SerDes &) = delete;



        ~SerDes() {
            if (!_storeIsExternal) {
                delete[] _store;
            }
        }

        void addPadding(size_t bytes) {
            if (!extendBy(bytes)) {
                return;
            }

            // Fill padding with something recognizable. Will keep valgrind happier.
            memset(&_store[_pos], PADDING_CHAR, bytes);
            _pos += bytes;
        }

        SerDes &operator<<(uint8_t c) {
            return *this << int8_t(c);
        }

        SerDes &operator<<(int8_t c) {
            if (!extendBy(1)) {
                return *this;
            }

            _store[_pos++] = c;
            return *this;
        }

        SerDes &operator>>(uint8_t &c) {
            return *this >> reinterpret_cast<int8_t&>(c);
        }

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

        SerDes &operator<<(uint16_t val) {
            return *this << int16_t(val);
        }

        SerDes &operator<<(int16_t val) {
            if (!extendBy(sizeof(val))) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        SerDes &operator>>(uint16_t &val) {
            return *this >> reinterpret_cast<int16_t&>(val);
        }

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

        SerDes &operator<<(uint32_t val) {
            return *this << int32_t(val);
        }

        SerDes &operator<<(int32_t val) {
            if (!extendBy(sizeof(val))) {
                return *this;
            }

            memcpy(&_store[_pos], (char*)&val, sizeof(val));
            _pos += sizeof(val);
            return *this;
        }

        SerDes &operator>>(uint32_t &val) {
            return *this >> reinterpret_cast<int32_t&>(val);
        }

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

        SerDes &operator<<(const char *val) {
            size_t len = strlen(val)+1;
            extendBy(len);
            memcpy(&_store[_pos], val, len);
            _pos += len;
            return *this;
        }

        SerDes &operator<<(const QString &val) {
            return *this << val.toUtf8().constData();
        }


        ///////////////////////////////////////////////////////////

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
