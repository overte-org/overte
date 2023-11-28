//
//  RenderableProceduralParticleEffectEntityItem.cpp
//  interface/src
//
//  Created by HifiExperiements on 11/19/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableProceduralParticleEffectEntityItem.h"

#include <procedural/ShaderConstants.h>
#include <shaders/Shaders.h>

using namespace render;
using namespace render::entities;

ProceduralParticleEffectEntityRenderer::ProceduralParticleEffectEntityRenderer(const EntityItemPointer& entity) :
    Parent(entity) {
    _updateProcedural._vertexSource = shader::Source::get(shader::gpu::vertex::DrawUnitQuadTexcoord);
    _updateProcedural._opaqueFragmentSource = shader::Source::get(shader::entities_renderer::fragment::proceduralParticleUpdate);
    _updateProcedural.setDoesFade(false);

    _renderProcedural._vertexSource = shader::Source::get(shader::entities_renderer::vertex::proceduralParticle);
    _renderProcedural._opaqueFragmentSource = shader::Source::get(shader::entities_renderer::fragment::proceduralParticle);
    _renderProcedural._transparentFragmentSource = shader::Source::get(shader::entities_renderer::fragment::proceduralParticle_translucent);
    _renderProcedural._transparentState->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
                                                          gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);
    _renderProcedural.setDoesFade(false);
}

void ProceduralParticleEffectEntityRenderer::doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) {
    void* key = (void*)this;
    AbstractViewStateInterface::instance()->pushPostUpdateLambda(key, [this, entity] {
        withWriteLock([&] {
            _renderTransform = getModelTransform();
            _renderTransform.postScale(entity->getScaledDimensions());
        });
    });
}

void ProceduralParticleEffectEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    bool needsUpdateDefines = false;
    bool needsRecreateParticles = false;

    uint32_t numParticles = entity->getNumParticles();
    if (_numParticles != numParticles) {
        _numParticles = numParticles;
        _particlePropTextureDim = pow(2, ceil(log2(sqrt(_numParticles))));
        needsUpdateDefines = true;
        needsRecreateParticles = true;
    }

    uint8_t numTrisPerParticle = entity->getNumTrianglesPerParticle();
    if (_numTrianglesPerParticle != numTrisPerParticle) {
        _numTrianglesPerParticle = numTrisPerParticle;
        needsUpdateDefines = true;
    }

    uint8_t numUpdateProps = entity->getNumUpdateProps();
    if (_numUpdateProps != numUpdateProps) {
        _numUpdateProps = numUpdateProps;
        needsUpdateDefines = true;
        needsRecreateParticles = true;
    }

    if (needsRecreateParticles) {
        recreateParticles();
    }

    bool particleTransparent = entity->getParticleTransparent();
    if (_transparent != particleTransparent) {
        _transparent = particleTransparent;
    }

    if (needsUpdateDefines) {
        std::unordered_map<std::string, std::string> replacements;

        static const std::string PROCEDURAL_PARTICLE_NUM_PARTICLES = "//PROCEDURAL_PARTICLE_NUM_PARTICLES";
        auto numParticlesDefine = "#undef NUM_PARTICLES\n#define NUM_PARTICLES " + std::to_string(_numParticles);
        replacements[PROCEDURAL_PARTICLE_NUM_PARTICLES] = numParticlesDefine;

        static const std::string PROCEDURAL_PARTICLE_NUM_UPDATE_PROPS = "//PROCEDURAL_PARTICLE_NUM_UPDATE_PROPS";
        auto numUpdatePropsDefine = "#undef NUM_UPDATE_PROPS\n#define NUM_UPDATE_PROPS " + std::to_string(_numUpdateProps);
        replacements[PROCEDURAL_PARTICLE_NUM_UPDATE_PROPS] = numUpdatePropsDefine;

        static const std::string PROCEDURAL_PARTICLE_NUM_TRIS_PER_PARTICLE = "//PROCEDURAL_PARTICLE_NUM_TRIS_PER_PARTICLE";
        auto numTrisPerParticleDefine = "#undef NUM_TRIS_PER_PARTICLE\n#define NUM_TRIS_PER_PARTICLE " + std::to_string(_numTrianglesPerParticle);
        replacements[PROCEDURAL_PARTICLE_NUM_TRIS_PER_PARTICLE] = numTrisPerParticleDefine;

        _updateProcedural.setFragmentReplacements(replacements);
        _renderProcedural.setFragmentReplacements(replacements);
        _renderProcedural.setVertexReplacements(replacements);
    }

    QString particleUpdateData = entity->getParticleUpdateData();
    if (_particleUpdateData != particleUpdateData) {
        _particleUpdateData = particleUpdateData;
        _updateProcedural.setProceduralData(ProceduralData::parse(particleUpdateData));
    }

    QString particleRenderData = entity->getParticleRenderData();
    if (_particleRenderData != particleRenderData) {
        _particleRenderData = particleRenderData;
        _renderProcedural.setProceduralData(ProceduralData::parse(particleRenderData));
    }
}

bool ProceduralParticleEffectEntityRenderer::isTransparent() const {
    return _transparent || Parent::isTransparent();
}

ItemKey ProceduralParticleEffectEntityRenderer::getKey() {
    ItemKey::Builder builder =
        ItemKey::Builder().withTypeShape().withTypeMeta().withTagBits(getTagMask()).withLayer(getHifiRenderLayer());

    if (isTransparent()) {
        builder.withTransparent();
    } else if (_canCastShadow) {
        builder.withShadowCaster();
    }

    if (_cullWithParent) {
        builder.withSubMetaCulled();
    }

    if (!_visible) {
        builder.withInvisible();
    }

    if (_numUpdateProps > 0) {
        builder.withSimulate();
    }

    return builder.build();
}

ShapeKey ProceduralParticleEffectEntityRenderer::getShapeKey() {
    auto builder = ShapeKey::Builder().withOwnPipeline();

    if (isTransparent()) {
        builder.withTranslucent();
    }

    if (_primitiveMode == PrimitiveMode::LINES) {
        builder.withWireframe();
    }

    return builder.build();
}

void ProceduralParticleEffectEntityRenderer::recreateParticles() {
    for (auto& buffer : _particleBuffers) {
        if (!buffer) {
            buffer = FramebufferPointer(gpu::Framebuffer::create(("RenderableProceduralParticleEffectEntity " + _entityID.toString()).toStdString()));
        }

        buffer->removeRenderBuffers();
        for (size_t i = 0; i < _numUpdateProps; i++) {
            TexturePointer texture = TexturePointer(gpu::Texture::createRenderBuffer(gpu::Element(gpu::VEC4, gpu::FLOAT, gpu::RGBA),
                (gpu::uint16)_particlePropTextureDim, (gpu::uint16)_particlePropTextureDim, gpu::Texture::SINGLE_MIP, gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_POINT)));
            texture->setSource(("RenderableProceduralParticleEffectEntity " + _entityID.toString() + " " + (char)i).toStdString());
            buffer->setRenderBuffer((gpu::uint32)i, texture);
        }
    }
}

void ProceduralParticleEffectEntityRenderer::renderSimulate(RenderArgs* args) {
    PerformanceTimer perfTimer("RenderableProceduralParticleEffectEntityItem::simulate");
    Q_ASSERT(args->_batch);
    gpu::Batch& batch = *args->_batch;

    if (!_visible || _numUpdateProps == 0 || !_updateProcedural.isReady()) {
        return;
    }

    _evenPass = !_evenPass;

    Transform transform;
    withReadLock([&] {
        transform = _renderTransform;
    });

    glm::ivec4 viewport = glm::ivec4(0, 0, _particleBuffers[!_evenPass]->getWidth(), _particleBuffers[!_evenPass]->getHeight());
    batch.setViewportTransform(viewport);
    batch.setFramebuffer(_particleBuffers[_evenPass]);

    for (size_t i = 0; i < _numUpdateProps; i++) {
        batch.setResourceTexture((gpu::uint32)(procedural::slot::texture::ParticleProp0 + i), _particleBuffers[!_evenPass]->getRenderBuffer((gpu::uint32)i));
    }

    _updateProcedural.prepare(batch, transform.getTranslation(), transform.getScale(), transform.getRotation(), _created, ProceduralProgramKey());
    batch.draw(gpu::TRIANGLE_STRIP, 4);
}

void ProceduralParticleEffectEntityRenderer::doRender(RenderArgs* args) {
    PerformanceTimer perfTimer("RenderableProceduralParticleEffectEntityItem::render");
    Q_ASSERT(args->_batch);
    gpu::Batch& batch = *args->_batch;

    if (!_visible || (_numUpdateProps > 0 && !_updateProcedural.isReady()) || !_renderProcedural.isReady()) {
        return;
    }

    Transform transform;
    withReadLock([&] {
        transform = _renderTransform;
    });

    for (size_t i = 0; i < _numUpdateProps; i++) {
        batch.setResourceTexture((gpu::uint32)(procedural::slot::texture::ParticleProp0 + i), _particleBuffers[_evenPass]->getRenderBuffer((gpu::uint32)i));
    }

    _renderProcedural.prepare(batch, transform.getTranslation(), transform.getScale(), transform.getRotation(), _created, ProceduralProgramKey(_transparent));

    static const size_t VERTEX_PER_TRIANGLE = 3;
    batch.drawInstanced((gpu::uint32)_numParticles, gpu::TRIANGLES, (gpu::uint32)(VERTEX_PER_TRIANGLE * _numTrianglesPerParticle));
}