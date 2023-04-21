//
//  TextureMap.h
//  libraries/graphics/src/graphics
//
//  Created by Sam Gateau on 5/6/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_model_TextureMap_h
#define hifi_model_TextureMap_h

#include "gpu/Texture.h"

#include "Transform.h"
#include "MaterialMappingMode.h"

namespace graphics {

class TextureMap {
public:
    TextureMap() {}

    void setTextureSource(gpu::TextureSourcePointer& textureSource);
    gpu::TextureSourcePointer getTextureSource() const { return _textureSource; }

    bool isDefined() const;
    gpu::TextureView getTextureView() const;

    void setTextureTransform(const Transform& texcoordTransform);
    const Transform& getTextureTransform() const { return _texcoordTransform; }

    void setMappingMode(MaterialMappingMode mode) { _mappingMode = mode; }
    MaterialMappingMode getMappingMode() const { return _mappingMode; }

    void setRepeat(bool repeat) { _repeat = repeat; }
    bool getRepeat() const { return _repeat; }

    void setUseAlphaChannel(bool useAlpha) { _useAlphaChannel = useAlpha; }
    bool useAlphaChannel() const { return _useAlphaChannel; }

    void setLightmapOffsetScale(float offset, float scale);
    const glm::vec2& getLightmapOffsetScale() const { return _lightmapOffsetScale; }

protected:
    gpu::TextureSourcePointer _textureSource;

    Transform _texcoordTransform;
    glm::vec2 _lightmapOffsetScale{ 0.0f, 1.0f };
    MaterialMappingMode _mappingMode { MaterialMappingMode::UV };
    bool _repeat { true };

    bool _useAlphaChannel{ false };
};
typedef std::shared_ptr< TextureMap > TextureMapPointer;

};

#endif // hifi_model_TextureMap_h

