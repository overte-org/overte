//
//  RenderableProceduralParticleEffectEntityItem.h
//  interface/src/entities
//
//  Created by HifiExperiements on 11/19/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RenderableProceduralParticleEffectEntityItem_h
#define hifi_RenderableProceduralParticleEffectEntityItem_h

#include "RenderableEntityItem.h"
#include <ProceduralParticleEffectEntityItem.h>

#include <procedural/Procedural.h>

namespace render { namespace entities {

class ProceduralParticleEffectEntityRenderer : public TypedEntityRenderer<ProceduralParticleEffectEntityItem> {
    using Parent = TypedEntityRenderer<ProceduralParticleEffectEntityItem>;
    friend class EntityRenderer;

public:
    ProceduralParticleEffectEntityRenderer(const EntityItemPointer& entity);

    virtual void renderSimulate(RenderArgs* args) override;

protected:
    virtual void doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) override;
    virtual void doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) override;

    bool isTransparent() const override;
    virtual ItemKey getKey() override;
    virtual ShapeKey getShapeKey() override;
    virtual void doRender(RenderArgs* args) override;

private:
    using TexturePointer = gpu::TexturePointer;
    using FramebufferPointer = gpu::FramebufferPointer;

    void recreateParticles();

    QString _particleUpdateData;
    Procedural _updateProcedural;
    QString _particleRenderData;
    Procedural _renderProcedural { false }; // No AA on Particles

    size_t _numParticles { 0 };
    size_t _particlePropTextureDim { 128 }; // 2^ceil(log2(sqrt(10,000)))
    size_t _numTrianglesPerParticle { particle::DEFAULT_NUM_TRIS_PER };
    size_t _numUpdateProps { particle::DEFAULT_NUM_UPDATE_PROPS };
    bool _transparent { false };

    std::array<FramebufferPointer, 2> _particleBuffers;
    bool _evenPass { true };
};

} } // namespace

#endif // hifi_RenderableProceduralParticleEffectEntityItem_h
