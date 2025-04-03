//
//  Created by Ada <ada@thingvellir.net> on 2025-02-24
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_RenderableCanvasEntityItem_h
#define hifi_RenderableCanvasEntityItem_h

#include "RenderableEntityItem.h"
#include <CanvasEntityItem.h>
#include <procedural/Procedural.h>

namespace render { namespace entities {

class CanvasEntityRenderer : public TypedEntityRenderer<CanvasEntityItem> {
    Q_OBJECT
    using Parent = TypedEntityRenderer<CanvasEntityItem>;
    friend class EntityRenderer;

public:
    CanvasEntityRenderer(const EntityItemPointer& entity);
    ~CanvasEntityRenderer();

    gpu::TexturePointer getTexture() override { return _texture; }

protected:
    virtual void doRender(RenderArgs* args) override;
    virtual bool isTransparent() const override { return true; }

    virtual void doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) override;
    virtual void doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) override;

private:
    std::shared_ptr<graphics::ProceduralMaterial> _material { std::make_shared<graphics::ProceduralMaterial>() };
    gpu::TexturePointer _texture;

    int _geometryId { 0 };
};

} }

#endif // hifi_RenderableCanvasEntityItem_h
