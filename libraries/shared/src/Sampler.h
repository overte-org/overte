//
//  Sampler.h
//  libraries/shared/src
//
//  Created by HifiExperiments on 8/21/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Sampler_h
#define hifi_Sampler_h

#include <glm/glm.hpp>

#include "RegisteredMetaTypes.h"
#include "SerDes.h"

enum class ComparisonFunction {
    NEVER = 0,
    LESS,
    EQUAL,
    LESS_EQUAL,
    GREATER,
    NOT_EQUAL,
    GREATER_EQUAL,
    ALWAYS,

    NUM_COMPARISON_FUNCS,
};

class Sampler {
public:

    enum Filter {
        FILTER_MIN_MAG_POINT, // top mip only
        FILTER_MIN_POINT_MAG_LINEAR, // top mip only
        FILTER_MIN_LINEAR_MAG_POINT, // top mip only
        FILTER_MIN_MAG_LINEAR, // top mip only

        FILTER_MIN_MAG_MIP_POINT,
        FILTER_MIN_MAG_POINT_MIP_LINEAR,
        FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
        FILTER_MIN_POINT_MAG_MIP_LINEAR,
        FILTER_MIN_LINEAR_MAG_MIP_POINT,
        FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        FILTER_MIN_MAG_LINEAR_MIP_POINT,
        FILTER_MIN_MAG_MIP_LINEAR,
        FILTER_ANISOTROPIC,

        NUM_FILTERS,
    };

    enum WrapMode {
        WRAP_REPEAT = 0,
        WRAP_MIRROR,
        WRAP_CLAMP,
        WRAP_BORDER,
        WRAP_MIRROR_ONCE,

        NUM_WRAP_MODES
    };

    static const uint8_t MAX_MIP_LEVEL = 0xFF;

    class Desc {
    public:
        glm::vec4 _borderColor{ 1.0f };
        uint32_t _maxAnisotropy = 16;

        uint8_t _filter = FILTER_MIN_MAG_MIP_LINEAR;
        uint8_t _comparisonFunc = (uint8_t)ComparisonFunction::ALWAYS;

        uint8_t _wrapModeU = WRAP_REPEAT;
        uint8_t _wrapModeV = WRAP_REPEAT;
        uint8_t _wrapModeW = WRAP_REPEAT;

        uint8_t _mipOffset = 0;
        uint8_t _minMip = 0;
        uint8_t _maxMip = MAX_MIP_LEVEL;

        Desc() {}
        Desc(const Filter filter, const WrapMode wrap = WRAP_REPEAT) : _filter(filter), _wrapModeU(wrap), _wrapModeV(wrap), _wrapModeW(wrap) {}

        bool operator==(const Desc& other) const {
            return _borderColor == other._borderColor &&
                _maxAnisotropy == other._maxAnisotropy &&
                _filter == other._filter &&
                _comparisonFunc == other._comparisonFunc &&
                _wrapModeU == other._wrapModeU &&
                _wrapModeV == other._wrapModeV &&
                _wrapModeW == other._wrapModeW &&
                _mipOffset == other._mipOffset &&
                _minMip == other._minMip &&
                _maxMip == other._maxMip;
        }
    };
    static_assert(sizeof(Desc) == 28, "Sampler::Desc size doesn't match.");

    Sampler() {}
    Sampler(const Filter filter, const WrapMode wrap = WRAP_REPEAT) : _desc(filter, wrap) {}
    Sampler(const Desc& desc) : _desc(desc) {}
    ~Sampler() {}

    const glm::vec4& getBorderColor() const { return _desc._borderColor; }

    uint32_t getMaxAnisotropy() const { return _desc._maxAnisotropy; }

    WrapMode getWrapModeU() const { return WrapMode(_desc._wrapModeU); }
    WrapMode getWrapModeV() const { return WrapMode(_desc._wrapModeV); }
    WrapMode getWrapModeW() const { return WrapMode(_desc._wrapModeW); }

    Filter getFilter() const { return Filter(_desc._filter); }
    ComparisonFunction getComparisonFunction() const { return ComparisonFunction(_desc._comparisonFunc); }
    bool doComparison() const { return getComparisonFunction() != ComparisonFunction::ALWAYS; }

    uint8_t getMipOffset() const { return _desc._mipOffset; }
    uint8_t getMinMip() const { return _desc._minMip; }
    uint8_t getMaxMip() const { return _desc._maxMip; }

    const Desc& getDesc() const { return _desc; }

    bool operator==(const Sampler& other) const {
        return _desc == other._desc;
    }
    bool operator!=(const Sampler& other) const {
        return !(*this == other);
    }

    static Sampler parseSampler(const QJsonObject& object);

protected:
    Desc _desc;
};

inline DataSerializer& operator<<(DataSerializer& ser, const Sampler::Desc& d) {
    DataSerializer::SizeTracker tracker(ser);
    ser << d._borderColor;
    ser << d._maxAnisotropy;
    ser << d._filter;
    ser << d._comparisonFunc;
    ser << d._wrapModeU;
    ser << d._wrapModeV;
    ser << d._wrapModeW;
    ser << d._mipOffset;
    ser << d._minMip;
    ser << d._maxMip;
    return ser;
}

inline DataDeserializer& operator>>(DataDeserializer& dsr, Sampler::Desc& d) {
    DataDeserializer::SizeTracker tracker(dsr);
    dsr >> d._borderColor;
    dsr >> d._maxAnisotropy;
    dsr >> d._filter;
    dsr >> d._comparisonFunc;
    dsr >> d._wrapModeU;
    dsr >> d._wrapModeV;
    dsr >> d._wrapModeW;
    dsr >> d._mipOffset;
    dsr >> d._minMip;
    dsr >> d._maxMip;
    return dsr;
}

namespace std {
    template<> struct hash<Sampler> {
        size_t operator()(const Sampler& sampler) const noexcept {
            size_t result = 0;
            const auto& desc = sampler.getDesc();
            hash_combine(result, desc._comparisonFunc, desc._filter, desc._maxAnisotropy, desc._maxMip, desc._minMip, desc._wrapModeU, desc._wrapModeV, desc._wrapModeW);
            return result;
        }
    };
}

QDebug& operator<<(QDebug& dbg, const Sampler& s);

QString wrapModeToString(Sampler::WrapMode mode);
Sampler::WrapMode wrapModeFromString(const QString& string);

#endif // hifi_Sampler_h
