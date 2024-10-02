//
//  RenderableZoneEntityItem.cpp
//
//
//  Created by Clement on 4/22/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableZoneEntityItem.h"

#include <gpu/Batch.h>

#include <graphics/Stage.h>

#include <DependencyManager.h>
#include <GeometryCache.h>
#include <PerfStat.h>
#include <procedural/ProceduralSkybox.h>
#include <LightPayload.h>
#include <DeferredLightingEffect.h>

#include "EntityTreeRenderer.h"

// Sphere entities should fit inside a cube entity of the same size, so a sphere that has dimensions 1x1x1
// is a half unit sphere.  However, the geometry cache renders a UNIT sphere, so we need to scale down.
static const float SPHERE_ENTITY_SCALE = 0.5f;

using namespace render;
using namespace render::entities;

ZoneEntityRenderer::ZoneEntityRenderer(const EntityItemPointer& entity)
    : Parent(entity) {
    _background->setSkybox(std::make_shared<ProceduralSkybox>(entity->getCreated()));
}

void ZoneEntityRenderer::onRemoveFromSceneTyped(const TypedEntityPointer& entity) {
    if (_stage) {
        if (!LightStage::isIndexInvalid(_sunIndex)) {
            _stage->removeLight(_sunIndex);
            _sunIndex = INVALID_INDEX;
        }
        if (!LightStage::isIndexInvalid(_ambientIndex)) {
            _stage->removeLight(_ambientIndex);
            _ambientIndex = INVALID_INDEX;
        }
    }

    if (_backgroundStage) {
        if (!BackgroundStage::isIndexInvalid(_backgroundIndex)) {
            _backgroundStage->removeBackground(_backgroundIndex);
            _backgroundIndex = INVALID_INDEX;
        }
    }

    if (_hazeStage) {
        if (!HazeStage::isIndexInvalid(_hazeIndex)) {
            _hazeStage->removeHaze(_hazeIndex);
            _hazeIndex = INVALID_INDEX;
        }
    }

    if (_bloomStage) {
        if (!BloomStage::isIndexInvalid(_bloomIndex)) {
            _bloomStage->removeBloom(_bloomIndex);
            _bloomIndex = INVALID_INDEX;
        }
    }

    if (_tonemappingStage) {
        if (!TonemappingStage::isIndexInvalid(_tonemappingIndex)) {
            _tonemappingStage->removeTonemapping(_tonemappingIndex);
            _tonemappingIndex = INVALID_INDEX;
        }
    }

    if (_ambientOcclusionStage) {
        if (!AmbientOcclusionStage::isIndexInvalid(_ambientOcclusionIndex)) {
            _ambientOcclusionStage->removeAmbientOcclusion(_ambientOcclusionIndex);
            _ambientOcclusionIndex = INVALID_INDEX;
        }
    }
}

void ZoneEntityRenderer::doRender(RenderArgs* args) {
    // This is necessary so that zones can themselves be zone culled
    if (!passesZoneOcclusionTest(CullTest::_prevContainingZones)) {
        return;
    }

    if (!_stage) {
        _stage = args->_scene->getStage<LightStage>();
        assert(_stage);
    }

    if (!_backgroundStage) {
        _backgroundStage = args->_scene->getStage<BackgroundStage>();
        assert(_backgroundStage);
    }

    if (!_hazeStage) {
        _hazeStage = args->_scene->getStage<HazeStage>();
        assert(_hazeStage);
    }

    if (!_bloomStage) {
        _bloomStage = args->_scene->getStage<BloomStage>();
        assert(_bloomStage);
    }

    if (!_tonemappingStage) {
        _tonemappingStage = args->_scene->getStage<TonemappingStage>();
        assert(_tonemappingStage);
    }

    if (!_ambientOcclusionStage) {
        _ambientOcclusionStage = args->_scene->getStage<AmbientOcclusionStage>();
        assert(_ambientOcclusionStage);
    }

    { // Sun 
        if (_needSunUpdate) {
            if (LightStage::isIndexInvalid(_sunIndex)) {
                _sunIndex = _stage->addLight(_sunLight);
            } else {
                _stage->updateLightArrayBuffer(_sunIndex);
            }
            _needSunUpdate = false;
        }
    }

    { // Ambient
        updateAmbientMap();

        if (_needAmbientUpdate) {
            if (LightStage::isIndexInvalid(_ambientIndex)) {
                _ambientIndex = _stage->addLight(_ambientLight);
            } else {
                _stage->updateLightArrayBuffer(_ambientIndex);
            }
            _needAmbientUpdate = false;
        }
    }

    { // Skybox
        updateSkyboxMap();

        if (_needBackgroundUpdate) {
            if (BackgroundStage::isIndexInvalid(_backgroundIndex)) {
                _backgroundIndex = _backgroundStage->addBackground(_background);
            }
            _needBackgroundUpdate = false;
        }
    }

    {
        if (_needHazeUpdate) {
            if (HazeStage::isIndexInvalid(_hazeIndex)) {
                _hazeIndex = _hazeStage->addHaze(_haze);
            }
            _needHazeUpdate = false;
        }
    }

    {
        if (_needBloomUpdate) {
            if (BloomStage::isIndexInvalid(_bloomIndex)) {
                _bloomIndex = _bloomStage->addBloom(_bloom);
            }
            _needBloomUpdate = false;
        }
    }

    {
        if (_needTonemappingUpdate) {
            if (TonemappingStage::isIndexInvalid(_tonemappingIndex)) {
                _tonemappingIndex = _tonemappingStage->addTonemapping(_tonemapping);
            }
            _needTonemappingUpdate = false;
        }
    }

    {
        if (_needAmbientOcclusionUpdate) {
            if (AmbientOcclusionStage::isIndexInvalid(_ambientOcclusionIndex)) {
                _ambientOcclusionIndex = _ambientOcclusionStage->addAmbientOcclusion(_ambientOcclusion);
            }
            _needAmbientOcclusionUpdate = false;
        }
    }

    if (_visible) {
        // Finally, push the lights visible in the frame
        //
        // If component is disabled then push component off state
        // else if component is enabled then push current state
        // (else mode is inherit, the value from the parent zone will be used
        //
        if (_keyLightMode == COMPONENT_MODE_DISABLED) {
            _stage->_currentFrame.pushSunLight(_stage->getSunOffLight());
        } else if (_keyLightMode == COMPONENT_MODE_ENABLED) {
            _stage->_currentFrame.pushSunLight(_sunIndex);
        }

        if (_skyboxMode == COMPONENT_MODE_DISABLED) {
            _backgroundStage->_currentFrame.pushBackground(INVALID_INDEX);
        } else if (_skyboxMode == COMPONENT_MODE_ENABLED) {
            _backgroundStage->_currentFrame.pushBackground(_backgroundIndex);
        }

        if (_ambientLightMode == COMPONENT_MODE_DISABLED) {
            _stage->_currentFrame.pushAmbientLight(_stage->getAmbientOffLight());
        } else if (_ambientLightMode == COMPONENT_MODE_ENABLED) {
            _stage->_currentFrame.pushAmbientLight(_ambientIndex);
        }

        // Haze only if the mode is not inherit, as the model deals with on/off
        if (_hazeMode != COMPONENT_MODE_INHERIT) {
            _hazeStage->_currentFrame.pushHaze(_hazeIndex);
        }

        if (_bloomMode == COMPONENT_MODE_DISABLED) {
            _bloomStage->_currentFrame.pushBloom(INVALID_INDEX);
        } else if (_bloomMode == COMPONENT_MODE_ENABLED) {
            _bloomStage->_currentFrame.pushBloom(_bloomIndex);
        }

        if (_tonemappingMode == COMPONENT_MODE_DISABLED) {
            _tonemappingStage->_currentFrame.pushTonemapping(0); // Use the fallback tonemapping for "off"
        } else if (_tonemappingMode == COMPONENT_MODE_ENABLED) {
            _tonemappingStage->_currentFrame.pushTonemapping(_tonemappingIndex);
        }

        if (_ambientOcclusionMode == COMPONENT_MODE_DISABLED) {
            _ambientOcclusionStage->_currentFrame.pushAmbientOcclusion(INVALID_INDEX);
        } else if (_ambientOcclusionMode == COMPONENT_MODE_ENABLED) {
            _ambientOcclusionStage->_currentFrame.pushAmbientOcclusion(_ambientOcclusionIndex);
        }
    }

    CullTest::_containingZones.insert(_entityID);
}

void ZoneEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    auto position = entity->getWorldPosition();
    auto rotation = entity->getWorldOrientation();
    auto dimensions = entity->getScaledDimensions();
    bool rotationChanged = rotation != _lastRotation;
    bool transformChanged = rotationChanged || position != _lastPosition || dimensions != _lastDimensions;

    auto visible = entity->getVisible();
    if (transformChanged || visible != _lastVisible) {
        _lastVisible = visible;
        void* key = (void*)this;
        EntityItemID id = entity->getID();
        AbstractViewStateInterface::instance()->pushPostUpdateLambda(key, [id] {
            DependencyManager::get<EntityTreeRenderer>()->updateZone(id);
        });
    }

    auto proceduralUserData = entity->getUserData();
    bool proceduralUserDataChanged = _proceduralUserData != proceduralUserData;

    // FIXME one of the bools here could become true between being fetched and being reset, 
    // resulting in a lost update
    bool keyLightChanged = entity->keyLightPropertiesChanged() || rotationChanged;
    bool ambientLightChanged = entity->ambientLightPropertiesChanged() || transformChanged;
    bool skyboxChanged = entity->skyboxPropertiesChanged() || proceduralUserDataChanged;
    bool hazeChanged = entity->hazePropertiesChanged();
    bool bloomChanged = entity->bloomPropertiesChanged();
    bool tonemappingChanged = entity->tonemappingPropertiesChanged();
    bool ambientOcclusionChanged = entity->ambientOcclusionPropertiesChanged();
    entity->resetRenderingPropertiesChanged();

    if (transformChanged) {
        _lastPosition = entity->getWorldPosition();
        _lastRotation = entity->getWorldOrientation();
        _lastDimensions = entity->getScaledDimensions();
    }

    if (proceduralUserDataChanged) {
        _proceduralUserData = entity->getUserData();
    }

    updateKeyZoneItemFromEntity(entity);

    if (keyLightChanged) {
        _keyLightProperties = entity->getKeyLightProperties();
        updateKeySunFromEntity(entity);
    }

    if (ambientLightChanged) {
        _ambientLightProperties = entity->getAmbientLightProperties();
        updateAmbientLightFromEntity(entity);
    }

    if (skyboxChanged) {
        _skyboxProperties = entity->getSkyboxProperties();
        updateKeyBackgroundFromEntity(entity);
    }

    if (hazeChanged) {
        _hazeProperties = entity->getHazeProperties();
        updateHazeFromEntity(entity);
    }

    if (bloomChanged) {
        _bloomProperties = entity->getBloomProperties();
        updateBloomFromEntity(entity);
    }

    if (tonemappingChanged) {
        _tonemappingProperties = entity->getTonemappingProperties();
        updateTonemappingFromEntity(entity);
    }

    if (ambientOcclusionChanged) {
        _ambientOcclusionProperties = entity->getAmbientOcclusionProperties();
        updateAmbientOcclusionFromEntity(entity);
    }

    bool visuallyReady = true;
    uint32_t skyboxMode = entity->getSkyboxMode();
    if (skyboxMode == COMPONENT_MODE_ENABLED && !_skyboxTextureURL.isEmpty()) {
        bool skyboxLoadedOrFailed = (_skyboxTexture && (_skyboxTexture->isLoaded() || _skyboxTexture->isFailed()));

        visuallyReady = skyboxLoadedOrFailed;
    }

    entity->setVisuallyReady(visuallyReady);
}

ItemKey ZoneEntityRenderer::getKey() {
    return ItemKey::Builder().withTypeMeta().withTagBits(getTagMask()).build();
}

bool ZoneEntityRenderer::needsRenderUpdateFromTypedEntity(const TypedEntityPointer& entity) const {
    if (entity->keyLightPropertiesChanged() ||
        entity->ambientLightPropertiesChanged() ||
        entity->hazePropertiesChanged() ||
        entity->bloomPropertiesChanged() ||
        entity->skyboxPropertiesChanged() ||
        entity->tonemappingPropertiesChanged() ||
        entity->ambientOcclusionPropertiesChanged()) {

        return true;
    }

    return false;
}

void ZoneEntityRenderer::updateKeySunFromEntity(const TypedEntityPointer& entity) {
    _keyLightMode = (ComponentMode)entity->getKeyLightMode();

    const auto& sunLight = editSunLight();
    sunLight->setType(graphics::Light::SUN);
    sunLight->setPosition(_lastPosition);
    sunLight->setOrientation(_lastRotation);

    // Set the keylight
    sunLight->setColor(ColorUtils::toVec3(_keyLightProperties.getColor()));
    sunLight->setIntensity(_keyLightProperties.getIntensity());
    sunLight->setDirection(_lastRotation * _keyLightProperties.getDirection());
    sunLight->setCastShadows(_keyLightProperties.getCastShadows());
    sunLight->setShadowBias(_keyLightProperties.getShadowBias());
    sunLight->setShadowsMaxDistance(_keyLightProperties.getShadowMaxDistance());
}

void ZoneEntityRenderer::updateAmbientLightFromEntity(const TypedEntityPointer& entity) {
    _ambientLightMode = (ComponentMode)entity->getAmbientLightMode();

    const auto& ambientLight = editAmbientLight();
    ambientLight->setType(graphics::Light::AMBIENT);
    ambientLight->setPosition(_lastPosition);
    ambientLight->setOrientation(_lastRotation);

    // Set the ambient light
    ambientLight->setAmbientColor(ColorUtils::toVec3(_ambientLightProperties.getAmbientColor()));
    ambientLight->setAmbientIntensity(_ambientLightProperties.getAmbientIntensity());

    if (_ambientLightProperties.getAmbientURL().isEmpty()) {
        setAmbientURL(_skyboxProperties.getUrl());
    } else {
        setAmbientURL(_ambientLightProperties.getAmbientURL());
    }

    ambientLight->setTransform(entity->getTransform().getInverseMatrix());
}

void ZoneEntityRenderer::updateHazeFromEntity(const TypedEntityPointer& entity) {
    const uint32_t hazeMode = entity->getHazeMode();

    _hazeMode = (ComponentMode)hazeMode;

    const auto& haze = editHaze();

    haze->setHazeActive(hazeMode == COMPONENT_MODE_ENABLED);
    haze->setAltitudeBased(_hazeProperties.getHazeAltitudeEffect());

    haze->setHazeRangeFactor(graphics::Haze::convertHazeRangeToHazeRangeFactor(_hazeProperties.getHazeRange()));
    glm::u8vec3 hazeColor = _hazeProperties.getHazeColor();
    haze->setHazeColor(toGlm(hazeColor));
    glm::u8vec3 hazeGlareColor = _hazeProperties.getHazeGlareColor();
    haze->setHazeGlareColor(toGlm(hazeGlareColor));
    haze->setHazeEnableGlare(_hazeProperties.getHazeEnableGlare());
    haze->setHazeGlareBlend(graphics::Haze::convertGlareAngleToPower(_hazeProperties.getHazeGlareAngle()));

    float hazeAltitude = _hazeProperties.getHazeCeiling() - _hazeProperties.getHazeBaseRef();
    haze->setHazeAltitudeFactor(graphics::Haze::convertHazeAltitudeToHazeAltitudeFactor(hazeAltitude));
    haze->setHazeBaseReference(_hazeProperties.getHazeBaseRef());

    haze->setHazeBackgroundBlend(_hazeProperties.getHazeBackgroundBlend());

    haze->setHazeAttenuateKeyLight(_hazeProperties.getHazeAttenuateKeyLight());
    haze->setHazeKeyLightRangeFactor(graphics::Haze::convertHazeRangeToHazeRangeFactor(_hazeProperties.getHazeKeyLightRange()));
    haze->setHazeKeyLightAltitudeFactor(graphics::Haze::convertHazeAltitudeToHazeAltitudeFactor(_hazeProperties.getHazeKeyLightAltitude()));
}

void ZoneEntityRenderer::updateBloomFromEntity(const TypedEntityPointer& entity) {
    _bloomMode = (ComponentMode)entity->getBloomMode();

    const auto& bloom = editBloom();

    bloom->setBloomIntensity(_bloomProperties.getBloomIntensity());
    bloom->setBloomThreshold(_bloomProperties.getBloomThreshold());
    bloom->setBloomSize(_bloomProperties.getBloomSize());
}

void ZoneEntityRenderer::updateTonemappingFromEntity(const TypedEntityPointer& entity) {
    _tonemappingMode = (ComponentMode)entity->getTonemappingMode();

    const auto& tonemapping = editTonemapping();

    tonemapping->setCurve(_tonemappingProperties.getCurve());
    tonemapping->setExposure(_tonemappingProperties.getExposure());
}

void ZoneEntityRenderer::updateAmbientOcclusionFromEntity(const TypedEntityPointer& entity) {
    _ambientOcclusionMode = (ComponentMode)entity->getAmbientOcclusionMode();

    const auto& ambientOcclusion = editAmbientOcclusion();

    ambientOcclusion->setTechnique(_ambientOcclusionProperties.getTechnique());
    ambientOcclusion->setJitter(_ambientOcclusionProperties.getJitter());
    ambientOcclusion->setResolutionLevel(_ambientOcclusionProperties.getResolutionLevel());
    ambientOcclusion->setEdgeSharpness(_ambientOcclusionProperties.getEdgeSharpness());
    ambientOcclusion->setBlurRadius(_ambientOcclusionProperties.getBlurRadius());
    ambientOcclusion->setAORadius(_ambientOcclusionProperties.getAoRadius());
    ambientOcclusion->setAOObscuranceLevel(_ambientOcclusionProperties.getAoObscuranceLevel());
    ambientOcclusion->setAOFalloffAngle(_ambientOcclusionProperties.getAoFalloffAngle());
    ambientOcclusion->setAOSamplingAmount(_ambientOcclusionProperties.getAoSamplingAmount());
    ambientOcclusion->setSSAONumSpiralTurns(_ambientOcclusionProperties.getSsaoNumSpiralTurns());
}

void ZoneEntityRenderer::updateKeyBackgroundFromEntity(const TypedEntityPointer& entity) {
    _skyboxMode = (ComponentMode)entity->getSkyboxMode();

    editBackground();
    setSkyboxColor(toGlm(_skyboxProperties.getColor()));
    setProceduralUserData(_proceduralUserData);
    setSkyboxURL(_skyboxProperties.getUrl());
}

void ZoneEntityRenderer::updateKeyZoneItemFromEntity(const TypedEntityPointer& entity) {
    // Update rotation values
    editSkybox()->setOrientation(_lastRotation);

    /* TODO: Implement the sun model behavior / Keep this code here for reference, this is how we
    {
    // Set the stage
    bool isSunModelEnabled = this->getStageProperties().getSunModelEnabled();
    sceneStage->setSunModelEnable(isSunModelEnabled);
    if (isSunModelEnabled) {
    sceneStage->setLocation(this->getStageProperties().getLongitude(),
    this->getStageProperties().getLatitude(),
    this->getStageProperties().getAltitude());

    auto sceneTime = sceneStage->getTime();
    sceneTime->setHour(this->getStageProperties().calculateHour());
    sceneTime->setDay(this->getStageProperties().calculateDay());
    }
    }*/
}

void ZoneEntityRenderer::setAmbientURL(const QString& ambientUrl) {
    if (_ambientTextureURL == ambientUrl) {
        return;
    }
    _ambientTextureURL = ambientUrl;

    if (_ambientTextureURL.isEmpty()) {
        _pendingAmbientTexture = false;
        _ambientTexture.clear();

        _ambientLight->setAmbientMap(nullptr);
        _ambientLight->setAmbientSpherePreset(gpu::SphericalHarmonics::BREEZEWAY);
    } else {
        _pendingAmbientTexture = true;
        auto textureCache = DependencyManager::get<TextureCache>();
        _ambientTexture = textureCache->getTexture(_ambientTextureURL, image::TextureUsage::AMBIENT_TEXTURE);
    }
}

void ZoneEntityRenderer::updateAmbientMap() {
    if (_pendingAmbientTexture) {
        if (_ambientTexture && _ambientTexture->isLoaded()) {
            _pendingAmbientTexture = false;

            auto texture = _ambientTexture->getGPUTexture();
            if (texture) {
                if (texture->getIrradiance()) {
                    _ambientLight->setAmbientSphere(*texture->getIrradiance());
                } else {
                    _ambientLight->setAmbientSpherePreset(gpu::SphericalHarmonics::BREEZEWAY);
                }
                editAmbientLight()->setAmbientMap(texture);
            } else {
                qCDebug(entitiesrenderer) << "Failed to load ambient texture:" << _ambientTexture->getURL();
            }
        }
    }
}

void ZoneEntityRenderer::setSkyboxURL(const QString& skyboxUrl) {
    if (_skyboxTextureURL == skyboxUrl) {
        return;
    }
    _skyboxTextureURL = skyboxUrl;

    if (_skyboxTextureURL.isEmpty()) {
        _pendingSkyboxTexture = false;
        _skyboxTexture.clear();

        editSkybox()->setCubemap(nullptr);
    } else {
        _pendingSkyboxTexture = true;
        auto textureCache = DependencyManager::get<TextureCache>();
        _skyboxTexture = textureCache->getTexture(_skyboxTextureURL, image::TextureUsage::SKY_TEXTURE);
    }
}

void ZoneEntityRenderer::updateSkyboxMap() {
    if (_pendingSkyboxTexture) {
        if (_skyboxTexture && _skyboxTexture->isLoaded()) {
            _pendingSkyboxTexture = false;

            auto texture = _skyboxTexture->getGPUTexture();
            if (texture) {
                editSkybox()->setCubemap(texture);
            } else {
                qCDebug(entitiesrenderer) << "Failed to load Skybox texture:" << _skyboxTexture->getURL();
            }
        }
    }
}

void ZoneEntityRenderer::setSkyboxColor(const glm::vec3& color) {
    editSkybox()->setColor(color);
}

void ZoneEntityRenderer::setProceduralUserData(const QString& userData) {
    std::dynamic_pointer_cast<ProceduralSkybox>(editSkybox())->parse(userData);
}

