//
//  AmbientOcclusion.h
//  libraries/graphics/src/graphics
//
//  Created by HifiExperiments 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_model_AmbientOcclusion_h
#define hifi_model_AmbientOcclusion_h

#include <memory>

#include <AmbientOcclusionTechnique.h>

namespace graphics {
    class AmbientOcclusion {
    public:
        AmbientOcclusion() {}

        void setTechnique(const AmbientOcclusionTechnique technique) { _technique = technique; }
        void setJitter(const bool jitter) { _jitter = jitter; }
        void setResolutionLevel(const uint8_t resolutionLevel) { _resolutionLevel = resolutionLevel; }
        void setEdgeSharpness(const float edgeSharpness) { _edgeSharpness = edgeSharpness; }
        void setBlurRadius(const uint8_t blurRadius) { _blurRadius = blurRadius; }
        void setAORadius(const float aoRadius) { _aoRadius = aoRadius; }
        void setAOObscuranceLevel(const float aoObscuranceLevel) { _aoObscuranceLevel = aoObscuranceLevel; }
        void setAOFalloffAngle(const float aoFalloffAngle) { _aoFalloffAngle = aoFalloffAngle; }
        void setAOSamplingAmount(const float aoSamplingAmount) { _aoSamplingAmount = aoSamplingAmount; }
        void setSSAONumSpiralTurns(const float ssaoNumSpiralTurns) { _ssaoNumSpiralTurns = ssaoNumSpiralTurns; }

        AmbientOcclusionTechnique getTechnique() const { return _technique; }
        bool getJitter() const { return _jitter; }
        uint8_t getResolutionLevel() const { return _resolutionLevel; }
        float getEdgeSharpness() const { return _edgeSharpness; }
        uint8_t getBlurRadius() const { return _blurRadius; }
        float getAORadius() const { return _aoRadius; }
        float getAOObscuranceLevel() const { return _aoObscuranceLevel; }
        float getAOFalloffAngle() const { return _aoFalloffAngle; }
        float getAOSamplingAmount() const { return _aoSamplingAmount; }
        float getSSAONumSpiralTurns() const { return _ssaoNumSpiralTurns; }

    private:
        AmbientOcclusionTechnique _technique { AmbientOcclusionTechnique::SSAO };
        bool _jitter { false };
        uint8_t _resolutionLevel { 2 };
        float _edgeSharpness { 1.0f };
        uint8_t _blurRadius { 4 };
        float _aoRadius { 1.0f };
        float _aoObscuranceLevel { 0.5f };
        float _aoFalloffAngle { 0.25f };
        float _aoSamplingAmount { 0.5f };
        float _ssaoNumSpiralTurns { 7.0f };
    };
    using AmbientOcclusionPointer = std::shared_ptr<AmbientOcclusion>;
}
#endif // hifi_model_AmbientOcclusion_h
