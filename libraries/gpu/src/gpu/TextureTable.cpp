//
//  Created by Bradley Austin Davis on 2017/01/25
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "TextureTable.h"
#include "Texture.h"

#include <shared/GlobalAppProperties.h>
using namespace gpu;


TextureTable::TextureTable(size_t tableSize) {
    _textures.resize(tableSize);
}

TextureTable::TextureTable(const std::initializer_list<TexturePointer>& textures, size_t tableSize) {
    _textures.resize(tableSize);
    auto max = std::min<size_t>(_textures.size(), textures.size());
    auto itr = textures.begin();
    size_t index = 0;
    while (itr != textures.end() && index < max) {
        setTexture(index, *itr);
        ++index;
    }
}

TextureTable::TextureTable(const Vector& textures, size_t tableSize) : _textures(textures) {
    _textures.resize(tableSize);
}

void TextureTable::setTexture(size_t index, const TexturePointer& texturePointer) {
    if (index >= _textures.size() || _textures[index] == texturePointer) {
        return;
    }
    {
        Lock lock(_mutex);
        ++_stamp;
        _textures[index] = texturePointer;
    }
}

void TextureTable::setTexture(size_t index, const TextureView& textureView) {
    setTexture(index, textureView._texture);
}

TextureTable::Vector TextureTable::getTextures() const {
     Vector result;
     {
         Lock lock(_mutex);
         result = _textures;
     }
     return result; 
}
