//
//  Created by Bradley Austin Davis on 2017/01/25
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_TextureTable_h
#define hifi_gpu_TextureTable_h

#include "Forward.h"

#include <vector>

#define TEXTURE_TABLE_COUNT_1_LAYER_MATERIAL 8
#define TEXTURE_TABLE_COUNT_2_LAYER_MATERIAL 6
#define TEXTURE_TABLE_COUNT_3_LAYER_MATERIAL 4

namespace gpu {

class TextureTable {
public:
    using Vector = std::vector<TexturePointer>;

    /**
     * @param tableSize Number of textures in this table.
     */
    TextureTable(size_t tableSize);

    /**
     * @param textures Initializer list with shared pointers to textures.
     * @param tableSize Number of textures in this table.
     */
    TextureTable(const std::initializer_list<TexturePointer>& textures, size_t tableSize);

    /**
     * @param textures Vector with shared pointers to textures.
     * @param tableSize Number of textures in this table.
     */
    TextureTable(const Vector& textures, size_t tableSize);

    // Only for gpu::Context
    const GPUObjectPointer gpuObject{};

    /**
     * @brief Sets a texture on a given texture table index.
     *
     * Thread-safe.
     * Stamp is incremented if texture changes.
     * @param index Index of the texture to set. Must fit inside the texture table.
     * @param texturePointer Shared pointer of a texture.
     */
    void setTexture(size_t index, const TexturePointer& texturePointer);

    /**
     * @brief Sets a texture on a given texture table index.
     *
     * Thread-safe.
     * Stamp is incremented if the texture is changed.
     * @param index Index of the texture to set. Must fit inside the texture table.
     * @param texturePointer Reference to a texture view.
     */
    void setTexture(size_t index, const TextureView& texturePointer);

    /**
     * Thread-safe.
     * @return Vector of texture pointers.
     */
    Vector getTextures() const;

    /**
     * Stamp is incremented when one of the textures changes.
     * @return Current stamp.
     */
    Stamp getStamp() const { return _stamp; }

    /**
     * @brief Resizes the texture table.
     *
     * Stamp is not incremented.
     * @param tableSize New size of the texture table.
     */
    void resize(size_t tableSize) { _textures.resize(tableSize); };

private:
    mutable Mutex _mutex;
    Vector _textures;
    Stamp _stamp{ 1 };
};

}

#endif
