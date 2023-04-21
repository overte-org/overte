//
//  LightStage.cpp
//  render-utils/src
//
//  Created by Zach Pomerantz on 1/14/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LightStage.h"

#include <cmath>

#include "ViewFrustum.h"

std::string LightStage::_stageName { "LIGHT_STAGE"};
// The bias matrix goes from homogeneous coordinates to UV coords (see http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/#basic-shader)
const glm::mat4 LightStage::Shadow::_biasMatrix {
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0 };
const int LightStage::Shadow::MAP_SIZE = 1024;

const LightStage::Index LightStage::INVALID_INDEX { render::indexed_container::INVALID_INDEX };

LightStage::LightStage() {
    // Add off lights
    const LightPointer ambientOffLight { std::make_shared<graphics::Light>() };
    ambientOffLight->setAmbientIntensity(0.0f);
    ambientOffLight->setColor(graphics::Vec3(0.0));
    ambientOffLight->setIntensity(0.0f);
    ambientOffLight->setType(graphics::Light::Type::AMBIENT);
    _ambientOffLightId = addLight(ambientOffLight);

    const LightPointer pointOffLight { std::make_shared<graphics::Light>() };
    pointOffLight->setAmbientIntensity(0.0f);
    pointOffLight->setColor(graphics::Vec3(0.0));
    pointOffLight->setIntensity(0.0f);
    pointOffLight->setType(graphics::Light::Type::POINT);
    _pointOffLightId = addLight(pointOffLight);

    const LightPointer spotOffLight { std::make_shared<graphics::Light>() };
    spotOffLight->setAmbientIntensity(0.0f);
    spotOffLight->setColor(graphics::Vec3(0.0));
    spotOffLight->setIntensity(0.0f);
    spotOffLight->setType(graphics::Light::Type::SPOT);
    _spotOffLightId = addLight(spotOffLight);

    const LightPointer sunOffLight { std::make_shared<graphics::Light>() };
    sunOffLight->setAmbientIntensity(0.0f);
    sunOffLight->setColor(graphics::Vec3(0.0));
    sunOffLight->setIntensity(0.0f);
    sunOffLight->setType(graphics::Light::Type::SUN);
    _sunOffLightId = addLight(sunOffLight);

    // Set default light to the off ambient light (until changed)
    _defaultLightId = _ambientOffLightId;
}

LightStage::Shadow::Schema::Schema() {
    ShadowTransform defaultTransform = {};
    defaultTransform.reprojection = mat4();
    defaultTransform.fixedBias = 0.005f;
    std::fill(cascades, cascades + SHADOW_CASCADE_MAX_COUNT, defaultTransform);
    invMapSize = 1.0f / MAP_SIZE;
    cascadeCount = 1;
    invCascadeBlendWidth = 1.0f / 0.2f;
    invFalloffDistance = 1.0f / 2.0f;
    maxDistance = 20.0f;
}

LightStage::Shadow::Cascade::Cascade() : 
    _frustum{ std::make_shared<ViewFrustum>() },
    _minDistance{ 0.0f },
    _maxDistance{ 20.0f } {
}

const glm::mat4& LightStage::Shadow::Cascade::getView() const {
    return _frustum->getView();
}

const glm::mat4& LightStage::Shadow::Cascade::getProjection() const {
    return _frustum->getProjection();
}

float LightStage::Shadow::Cascade::computeFarDistance(const ViewFrustum& viewFrustum, const Transform& shadowViewInverse,
                                                      float left, float right, float bottom, float top, float viewMaxShadowDistance) const {
    // Far distance should be extended to the intersection of the infinitely extruded shadow frustum 
    // with the view frustum side planes. To do so, we generate 10 triangles in shadow space which are the result of
    // tesselating the side and far faces of the view frustum and clip them with the 4 side planes of the
    // shadow frustum. The resulting clipped triangle vertices with the farthest Z gives the desired
    // shadow frustum far distance.
    std::array<Triangle, 10> viewFrustumTriangles;
    Plane shadowClipPlanes[4] = {
        Plane(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, top, 0.0f)),
        Plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, bottom, 0.0f)),
        Plane(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(left, 0.0f, 0.0f)),
        Plane(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(right, 0.0f, 0.0f))
    };

    viewFrustum.tesselateSidesAndFar(shadowViewInverse, viewFrustumTriangles.data(), viewMaxShadowDistance);

    static const int MAX_TRIANGLE_COUNT = 16;
    auto far = 0.0f;

    for (auto& triangle : viewFrustumTriangles) {
        Triangle clippedTriangles[MAX_TRIANGLE_COUNT];
        auto clippedTriangleCount = clipTriangleWithPlanes(triangle, shadowClipPlanes, 4, clippedTriangles, MAX_TRIANGLE_COUNT);

        for (auto i = 0; i < clippedTriangleCount; i++) {
            const auto& clippedTriangle = clippedTriangles[i];
            far = glm::max(far, -clippedTriangle.v0.z);
            far = glm::max(far, -clippedTriangle.v1.z);
            far = glm::max(far, -clippedTriangle.v2.z);
        }
    }

    return far;
}

LightStage::Shadow::Shadow(graphics::LightPointer light, unsigned int cascadeCount) : 
    _light{ light } {
    cascadeCount = std::min(cascadeCount, (unsigned int)SHADOW_CASCADE_MAX_COUNT);
    Schema schema;
    schema.cascadeCount = cascadeCount;
    _schemaBuffer = std::make_shared<gpu::Buffer>(sizeof(Schema), (const gpu::Byte*) &schema);

    // Create shadow cascade texture array
    auto depthFormat = gpu::Element(gpu::SCALAR, gpu::FLOAT, gpu::DEPTH);  // Depth32 texel format
    map = gpu::TexturePointer(gpu::Texture::createRenderBufferArray(depthFormat, MAP_SIZE, MAP_SIZE, cascadeCount));
    gpu::Sampler::Desc samplerDesc;
    samplerDesc._borderColor = glm::vec4(1.0f);
    samplerDesc._wrapModeU = gpu::Sampler::WRAP_BORDER;
    samplerDesc._wrapModeV = gpu::Sampler::WRAP_BORDER;
    samplerDesc._filter = gpu::Sampler::FILTER_MIN_MAG_LINEAR;
    samplerDesc._comparisonFunc = gpu::LESS;

    map->setSampler(gpu::Sampler(samplerDesc));

    _cascades.resize(cascadeCount);

    for (uint cascadeIndex=0; cascadeIndex < cascadeCount; cascadeIndex++) {
        auto& cascade = _cascades[cascadeIndex];
        std::string name = "Shadowmap Cascade ";
        name += '0' + cascadeIndex;
        cascade.framebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create(name));
        cascade.framebuffer->setDepthBuffer(map, depthFormat, cascadeIndex);
    }

    if (light) {
        setMaxDistance(light->getShadowsMaxDistance());
    }
}

void LightStage::Shadow::setLight(graphics::LightPointer light) {
    _light = light;
    if (light) {
        setMaxDistance(light->getShadowsMaxDistance());
    }
}

void LightStage::Shadow::setMaxDistance(float value) {
    static const auto MINIMUM_MAXDISTANCE = 1e-3f;

    value = std::max(MINIMUM_MAXDISTANCE, value);
    if (value != _maxDistance) {
        // This overlaping factor isn't really used directly for blending of shadow cascades. It's
        // just there to be sure the cascades do overlap. The blending width used is relative
        // to the UV space and is set in the Schema with invCascadeBlendWidth.
        static const auto OVERLAP_FACTOR = 1.0f / 5.0f;

        _maxDistance = value;

        if (_cascades.size() == 1) {
            _cascades.front().setMinDistance(0.0f);
            _cascades.front().setMaxDistance(_maxDistance);
        } else {
            // Distribute the cascades along that distance
            // TODO : these parameters should be exposed to the user as part of the light entity parameters, no?
            static const auto LOW_MAX_DISTANCE = 2.0f;
            static const auto MAX_RESOLUTION_LOSS = 0.6f; // Between 0 and 1, 0 giving tighter distributions

            // The max cascade distance is computed by multiplying the previous cascade's max distance by a certain
            // factor. There is a "user" factor that is computed from a desired max resolution loss in the shadow
            // and an optimal one based on the global min and max shadow distance, all cascades considered. The final
            // distance is a gradual blend between the two
            const auto userDistanceScale = 1.0f / (1.0f - MAX_RESOLUTION_LOSS);
            const auto optimalDistanceScale = powf(_maxDistance / LOW_MAX_DISTANCE, 1.0f / (_cascades.size() - 1));

            float maxCascadeUserDistance = LOW_MAX_DISTANCE;
            float maxCascadeOptimalDistance = LOW_MAX_DISTANCE;
            float minCascadeDistance = 0.0f;

            for (size_t cascadeIndex = 0; cascadeIndex < _cascades.size(); ++cascadeIndex) {
                float blendFactor = cascadeIndex / float(_cascades.size() - 1);
                float maxCascadeDistance;

                if (cascadeIndex == size_t(_cascades.size() - 1)) {
                    maxCascadeDistance = _maxDistance;
                } else {
                    maxCascadeDistance = maxCascadeUserDistance + (maxCascadeOptimalDistance - maxCascadeUserDistance)*blendFactor*blendFactor;
                }

                float shadowOverlapDistance = maxCascadeDistance * OVERLAP_FACTOR;

                _cascades[cascadeIndex].setMinDistance(minCascadeDistance);
                _cascades[cascadeIndex].setMaxDistance(maxCascadeDistance + shadowOverlapDistance);

                // Compute distances for next cascade
                minCascadeDistance = maxCascadeDistance;
                maxCascadeUserDistance = maxCascadeUserDistance * userDistanceScale;
                maxCascadeOptimalDistance = maxCascadeOptimalDistance * optimalDistanceScale;
                maxCascadeUserDistance = std::min(maxCascadeUserDistance, _maxDistance);
            }
        }

        // Update the buffer
        const auto& lastCascade = _cascades.back();
        auto& schema = _schemaBuffer.edit<Schema>();
        schema.maxDistance = _maxDistance;
        schema.invFalloffDistance = 1.0f / (OVERLAP_FACTOR*lastCascade.getMaxDistance());
    }
}

void LightStage::Shadow::setKeylightFrustum(const ViewFrustum& viewFrustum,
                                            float nearDepth, float farDepth) {
    assert(nearDepth < farDepth);
    // Orient the keylight frustum
    auto lightDirection = glm::normalize(_light->getDirection());
    glm::quat orientation;
    if (lightDirection == IDENTITY_UP) {
        orientation = glm::quat(glm::mat3(-IDENTITY_RIGHT, IDENTITY_FORWARD, -IDENTITY_UP));
    } else if (lightDirection == -IDENTITY_UP) {
        orientation = glm::quat(glm::mat3(IDENTITY_RIGHT, IDENTITY_FORWARD, IDENTITY_UP));
    } else {
        auto side = glm::normalize(glm::cross(lightDirection, IDENTITY_UP));
        auto up = glm::normalize(glm::cross(side, lightDirection));
        orientation = glm::quat_cast(glm::mat3(side, up, -lightDirection));
    }

    // Position the keylight frustum
    auto position = viewFrustum.getPosition() - (nearDepth + farDepth)*lightDirection;
    for (auto& cascade : _cascades) {
        cascade._frustum->setOrientation(orientation);
        cascade._frustum->setPosition(position);
    }
}

void LightStage::Shadow::setKeylightCascadeFrustum(unsigned int cascadeIndex, const ViewFrustum& viewFrustum,
                                            float nearDepth, float farDepth) {
    assert(nearDepth < farDepth);
    assert(cascadeIndex < _cascades.size());

    auto& cascade = _cascades[cascadeIndex];
    const auto viewMinCascadeShadowDistance = std::max(viewFrustum.getNearClip(), cascade.getMinDistance());
    const auto viewMaxCascadeShadowDistance = std::min(viewFrustum.getFarClip(), cascade.getMaxDistance());
    const auto viewMaxShadowDistance = _cascades.back().getMaxDistance();

    const Transform shadowView{ cascade._frustum->getView()};
    const Transform shadowViewInverse{ shadowView.getInverseMatrix() };

    auto nearCorners = viewFrustum.getCorners(viewMinCascadeShadowDistance);
    auto farCorners = viewFrustum.getCorners(viewMaxCascadeShadowDistance);

    vec3 min{ shadowViewInverse.transform(nearCorners.bottomLeft) };
    vec3 max{ min };
    // Expand keylight frustum  to fit view frustum
    auto fitFrustum = [&min, &max, &shadowViewInverse](const vec3& viewCorner) {
        const auto corner = shadowViewInverse.transform(viewCorner);

        min.x = glm::min(min.x, corner.x);
        min.y = glm::min(min.y, corner.y);
        min.z = glm::min(min.z, corner.z);

        max.x = glm::max(max.x, corner.x);
        max.y = glm::max(max.y, corner.y);
        max.z = glm::max(max.z, corner.z);
    };
    fitFrustum(nearCorners.bottomRight);
    fitFrustum(nearCorners.topLeft);
    fitFrustum(nearCorners.topRight);
    fitFrustum(farCorners.bottomLeft);
    fitFrustum(farCorners.bottomRight);
    fitFrustum(farCorners.topLeft);
    fitFrustum(farCorners.topRight);

    // Re-adjust near and far shadow distance
    auto near = glm::min(-max.z, nearDepth);
    auto far = cascade.computeFarDistance(viewFrustum, shadowViewInverse, min.x, max.x, min.y, max.y, viewMaxShadowDistance);

    glm::mat4 ortho = glm::ortho<float>(min.x, max.x, min.y, max.y, near, far);
    cascade._frustum->setProjection(ortho);

    // Calculate the frustum's internal state
    cascade._frustum->calculate();

    // Update the buffer
    auto& schema = _schemaBuffer.edit<Schema>();
    auto& schemaCascade = schema.cascades[cascadeIndex];
    schemaCascade.reprojection = _biasMatrix * ortho * shadowViewInverse.getMatrix();
}

void LightStage::Shadow::setKeylightCascadeBias(unsigned int cascadeIndex, float constantBias, float slopeBias) {
    auto& schema = _schemaBuffer.edit<Schema>();
    auto& schemaCascade = schema.cascades[cascadeIndex];
    schemaCascade.fixedBias = constantBias;
    schemaCascade.slopeBias = slopeBias;
}

void LightStage::Shadow::setCascadeFrustum(unsigned int cascadeIndex, const ViewFrustum& shadowFrustum) {
    assert(cascadeIndex < _cascades.size());
    const Transform view{ shadowFrustum.getView() };
    const Transform viewInverse{ view.getInverseMatrix() };
    auto& cascade = _cascades[cascadeIndex];

    *cascade._frustum = shadowFrustum;
    // Update the buffer
    auto& schema = _schemaBuffer.edit<Schema>();
    auto& schemaCascade = schema.cascades[cascadeIndex];
    schemaCascade.reprojection = _biasMatrix * shadowFrustum.getProjection() * viewInverse.getMatrix();
}

LightStage::Index LightStage::findLight(const LightPointer& light) const {
    auto found = _lightMap.find(light);
    if (found != _lightMap.end()) {
        return INVALID_INDEX;
    } else {
        return (*found).second;
    }
}

LightStage::Index LightStage::addLight(const LightPointer& light, const bool shouldSetAsDefault) {
    Index lightId;

    auto found = _lightMap.find(light);
    if (found == _lightMap.end()) {
        lightId = _lights.newElement(light);
        // Avoid failing to allocate a light, just pass
        if (lightId != INVALID_INDEX) {

            // Allocate the matching Desc to the light
            if (lightId >= (Index) _descs.size()) {
                _descs.emplace_back(Desc());
            } else {
                assert(_descs[lightId].shadowId == INVALID_INDEX);
                _descs[lightId] = Desc();
            }

            // INsert the light and its index in the reverese map
            _lightMap.insert(LightMap::value_type(light, lightId));

            updateLightArrayBuffer(lightId);
        }
    } else {
        lightId = (*found).second;
    }

    if (shouldSetAsDefault) {
        _defaultLightId = lightId;
    }

    return lightId;
}

LightStage::LightPointer LightStage::removeLight(Index index) {
    LightPointer removedLight = _lights.freeElement(index);
    if (removedLight) {
        _lightMap.erase(removedLight);
        _descs[index] = Desc();
    }
    assert(_descs.size() <= (size_t)index || _descs[index].shadowId == INVALID_INDEX);
    return removedLight;
}

LightStage::LightPointer LightStage::getCurrentKeyLight(const LightStage::Frame& frame) const {
    Index keyLightId { _defaultLightId };
    if (!frame._sunLights.empty()) {
        keyLightId = frame._sunLights.front();
    }
    return _lights.get(keyLightId);
}

LightStage::LightPointer LightStage::getCurrentAmbientLight(const LightStage::Frame& frame) const {
    Index keyLightId { _defaultLightId };
    if (!frame._ambientLights.empty()) {
        keyLightId = frame._ambientLights.front();
    }
    return _lights.get(keyLightId);
}

void LightStage::updateLightArrayBuffer(Index lightId) {
    auto lightSize = sizeof(graphics::Light::LightSchema);
    if (!_lightArrayBuffer) {
        _lightArrayBuffer = std::make_shared<gpu::Buffer>(lightSize);
    }

    assert(checkLightId(lightId));

    if (lightId > (Index)_lightArrayBuffer->getNumTypedElements<graphics::Light::LightSchema>()) {
        _lightArrayBuffer->resize(lightSize * (lightId + 10));
    }

    // lightArray is big enough so we can remap
    auto light = _lights._elements[lightId];
    if (light) {
        const auto& lightSchema = light->getLightSchemaBuffer().get();
        _lightArrayBuffer->setSubData<graphics::Light::LightSchema>(lightId, lightSchema);
    } else {
        // this should not happen ?
    }
}

LightStageSetup::LightStageSetup() {
}

void LightStageSetup::run(const render::RenderContextPointer& renderContext) {
    if (renderContext->_scene) {
        auto stage = renderContext->_scene->getStage(LightStage::getName());
        if (!stage) {
            stage = std::make_shared<LightStage>();
            renderContext->_scene->resetStage(LightStage::getName(), stage);
        }
    }
}

