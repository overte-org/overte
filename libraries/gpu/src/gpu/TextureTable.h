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
    TextureTable(size_t tableSize);
    TextureTable(const std::initializer_list<TexturePointer>& textures, size_t tableSize);
    TextureTable(const Vector& textures, size_t tableSize);

    // Only for gpu::Context
    const GPUObjectPointer gpuObject{};

    void setTexture(size_t index, const TexturePointer& texturePointer);
    void setTexture(size_t index, const TextureView& texturePointer);

    Vector getTextures() const;
    Stamp getStamp() const { return _stamp; }

    void resize(size_t tableSize) { _textures.resize(tableSize); };

private:
    mutable Mutex _mutex;
    Vector _textures;
    Stamp _stamp{ 1 };
};

}

#endif
