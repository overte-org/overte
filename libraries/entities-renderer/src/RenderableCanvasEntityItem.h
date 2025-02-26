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

#include <CanvasEntityItem.h>
#include "RenderableEntityItem.h"

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
    virtual void doRender(RenderArgs* args) override { }
    virtual bool isTransparent() const override { return false; }
    virtual bool wantsHandControllerPointerEvents() const override { return false; }
    virtual bool wantsKeyboardFocus() const override { return false; }

    virtual void doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) override;

private:
    gpu::TexturePointer _texture;
};

} }

#endif // hifi_RenderableCanvasEntityItem_h
