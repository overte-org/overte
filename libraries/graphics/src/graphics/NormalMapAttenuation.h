//
//  NormalMapAttenuation.h
//  libraries/graphics/src/graphics
//
//  Created by HifiExperiments 7/3/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_model_NormalMapAttenuation_h
#define hifi_model_NormalMapAttenuation_h

#include <memory>

namespace graphics {
    class NormalMapAttenuation {
    public:
        NormalMapAttenuation() {}

        void setMin(const float min) { _min = min; }
        void setMax(const float max) { _max = max; }

        float getMin() const { return _min; }
        float getMax() const { return _max; }

    private:
        float _min { 30.0f };
        float _max { 100.0f };
    };
    using NormalMapAttenuationPointer = std::shared_ptr<NormalMapAttenuation>;
}
#endif // hifi_model_NormalMapAttenuation_h
