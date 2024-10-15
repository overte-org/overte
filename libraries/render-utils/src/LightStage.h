//
//  LightStage.h
//  render-utils/src
//
//  Created by Zach Pomerantz on 1/14/2015.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_LightStage_h
#define hifi_render_utils_LightStage_h

#include <gpu/Framebuffer.h>
#include <graphics/Light.h>
#include <render/Engine.h>
#include <render/Stage.h>

class ViewFrustum;

class LightFrame {
public:
    LightFrame() {}

    using Index = render::indexed_container::Index;

    void clear() { _pointLights.clear(); _spotLights.clear(); _sunLights.clear(); _ambientLights.clear(); }
    void pushLight(Index index, graphics::Light::Type type) {
        switch (type) {
            case graphics::Light::POINT: { pushPointLight(index); break; }
            case graphics::Light::SPOT: { pushSpotLight(index); break; }
            case graphics::Light::SUN: { pushSunLight(index); break; }
            case graphics::Light::AMBIENT: { pushAmbientLight(index); break; }
            default: { break; }
        }
    }
    void pushPointLight(Index index) { _pointLights.emplace_back(index); }
    void pushSpotLight(Index index) { _spotLights.emplace_back(index); }
    void pushSunLight(Index index) { _sunLights.emplace_back(index); }
    void pushAmbientLight(Index index) { _ambientLights.emplace_back(index); }

    render::ElementIndices _pointLights;
    render::ElementIndices _spotLights;
    render::ElementIndices _sunLights;
    render::ElementIndices _ambientLights;
};

// Light stage to set up light-related rendering tasks
class LightStage : public render::PointerStage<graphics::Light, graphics::LightPointer, LightFrame> {
public:
    using LightPointer = graphics::LightPointer;

    class Shadow {
    public:
        using UniformBufferView = gpu::BufferView;
        static const int MAP_SIZE;

        class Cascade {
            friend Shadow;
        public:

            Cascade();

            gpu::FramebufferPointer framebuffer;

            const std::shared_ptr<ViewFrustum>& getFrustum() const { return _frustum; }

            const glm::mat4& getView() const;
            const glm::mat4& getProjection() const;

            void setMinDistance(float value) { _minDistance = value; }
            void setMaxDistance(float value) { _maxDistance = value; }
            float getMinDistance() const { return _minDistance; }
            float getMaxDistance() const { return _maxDistance; }

        private:

            std::shared_ptr<ViewFrustum> _frustum;
            float _minDistance;
            float _maxDistance;

            float computeFarDistance(const ViewFrustum& viewFrustum, const Transform& shadowViewInverse,
                                     float left, float right, float bottom, float top, float viewMaxShadowDistance) const;
        };

        Shadow(LightPointer light, unsigned int cascadeCount = 1);

        void setLight(LightPointer light);

        void setKeylightFrustum(const ViewFrustum& viewFrustum,
                                float nearDepth = 1.0f, float farDepth = 1000.0f);
        void setKeylightCascadeFrustum(unsigned int cascadeIndex, const ViewFrustum& viewFrustum,
                                float nearDepth = 1.0f, float farDepth = 1000.0f);
        void setKeylightCascadeBias(unsigned int cascadeIndex, float constantBias, float slopeBias);
        void setCascadeFrustum(unsigned int cascadeIndex, const ViewFrustum& shadowFrustum);

        const UniformBufferView& getBuffer() const { return _schemaBuffer; }

        unsigned int getCascadeCount() const { return (unsigned int)_cascades.size(); }
        const Cascade& getCascade(unsigned int index) const { return _cascades[index]; }

        float getMaxDistance() const { return _maxDistance; }
        void setMaxDistance(float value);

        const LightPointer& getLight() const { return _light; }

        gpu::TexturePointer map;
#include "Shadows_shared.slh"
        class Schema : public ShadowParameters {
        public:
            Schema();
        };

    protected:

        using Cascades = std::vector<Cascade>;

        static const glm::mat4 _biasMatrix;

        LightPointer _light;
        float _maxDistance{ 0.0f };
        Cascades _cascades;

        UniformBufferView _schemaBuffer = nullptr;
    };
    using ShadowPointer = std::shared_ptr<Shadow>;

    Index addElement(const LightPointer& light) override { return addElement(light, false); }
    Index addElement(const LightPointer& light, const bool shouldSetAsDefault);
    LightPointer removeElement(Index index) override;

    Index getDefaultLight() { return _defaultLightId; }

    LightStage();

    gpu::BufferPointer getLightArrayBuffer() const { return _lightArrayBuffer; }
    void updateLightArrayBuffer(Index lightId);

    class ShadowFrame {
    public:
        ShadowFrame() {}

        void clear() {}

        using Object = ShadowPointer;
        using Objects = std::vector<Object>;

        void pushShadow(const ShadowPointer& shadow) {
            _objects.emplace_back(shadow);
        }

        Objects _objects;
    };
    using ShadowFramePointer = std::shared_ptr<ShadowFrame>;

    Index getAmbientOffLight() { return _ambientOffLightId; }
    Index getPointOffLight() { return _pointOffLightId; }
    Index getSpotOffLight() { return _spotOffLightId; }
    Index getSunOffLight() { return _sunOffLightId; }

    LightPointer getCurrentKeyLight(const LightFrame& frame) const;
    LightPointer getCurrentAmbientLight(const LightFrame& frame) const;

protected:

    struct Desc {
        Index shadowId{ INVALID_INDEX };
    };
    using Descs = std::vector<Desc>;

    gpu::BufferPointer _lightArrayBuffer;

    Descs _descs;

    // define off lights
    Index _ambientOffLightId;
    Index _pointOffLightId;
    Index _spotOffLightId;
    Index _sunOffLightId;

    Index _defaultLightId;

};
using LightStagePointer = std::shared_ptr<LightStage>;


class LightStageSetup {
public:
    using JobModel = render::Job::Model<LightStageSetup>;

    LightStageSetup();
    void run(const render::RenderContextPointer& renderContext);
};


#endif
