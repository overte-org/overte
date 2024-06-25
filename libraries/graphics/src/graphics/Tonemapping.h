//
//  Tonemapping.h
//  libraries/graphics/src/graphics
//
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_model_Tonemapping_h
#define hifi_model_Tonemapping_h

#include <memory>

#include <TonemappingCurve.h>

namespace graphics {
    class Tonemapping {
    public:
        Tonemapping() {}

        void setCurve(const TonemappingCurve curve) { _curve = curve; }
        void setExposure(const float exposure) { _exposure = exposure; }

        TonemappingCurve getCurve() const { return _curve; }
        float getExposure() const { return _exposure; }

    private:
        TonemappingCurve _curve { TonemappingCurve::SRGB };
        float _exposure { 0.0f };
    };
    using TonemappingPointer = std::shared_ptr<Tonemapping>;
}
#endif // hifi_model_Tonemapping_h
