//
//  Sampler.cpp
//  libraries/octree/src
//
//  Created by HifiExperiments on 8/21/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Sampler.h"

#include "QJsonObject"

QString wrapModeToString(Sampler::WrapMode mode) {
    switch (mode) {
        case Sampler::WrapMode::WRAP_REPEAT:
            return "repeat";
        case Sampler::WrapMode::WRAP_MIRROR:
            return "mirror";
        case Sampler::WrapMode::WRAP_CLAMP:
            return "clamp";
        case Sampler::WrapMode::WRAP_BORDER:
            return "border";
        case Sampler::WrapMode::WRAP_MIRROR_ONCE:
            return "mirrorOnce";
        default:
            return "";
    }
}

Sampler::WrapMode wrapModeFromString(const QString& string) {
    if (string == "repeat") {
        return Sampler::WrapMode::WRAP_REPEAT;
    } else if (string == "mirror") {
        return Sampler::WrapMode::WRAP_MIRROR;
    } else if (string == "clamp") {
        return Sampler::WrapMode::WRAP_CLAMP;
    } else if (string == "border") {
        return Sampler::WrapMode::WRAP_BORDER;
    } else if (string == "mirrorOnce") {
        return Sampler::WrapMode::WRAP_MIRROR_ONCE;
    }
    return Sampler::WrapMode::WRAP_REPEAT;
}

Sampler Sampler::parseSampler(const QJsonObject& object) {
    Sampler::Desc samplerDesc;

    auto filterItr = object.constFind("filter");
    if (filterItr != object.constEnd() && filterItr->isString()) {
        auto filterStr = filterItr->toString();
        if (filterStr == "point") {
            samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_POINT;
        } else if (filterStr == "linear") {
            samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_LINEAR;
        }
    } else {
        auto minFilterItr = object.constFind("minFilter");
        auto magFilterItr = object.constFind("magFilter");
        if (minFilterItr != object.constEnd() && minFilterItr->isString() && magFilterItr != object.constEnd() &&
            magFilterItr->isString()) {
            auto minFilterStr = minFilterItr->toString();
            auto magFilterStr = magFilterItr->toString();
            if (magFilterStr == "point") {
                if (minFilterStr == "point") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_POINT;
                } else if (minFilterStr == "linear") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_LINEAR_MAG_POINT;
                } else if (minFilterStr == "mipmapPoint") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_MIP_POINT;
                } else if (minFilterStr == "mipmapLinear") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_POINT_MIP_LINEAR;
                } else if (minFilterStr == "linearMipmapPoint") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_LINEAR_MAG_MIP_POINT;
                } else if (minFilterStr == "linearMipmapLinear") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                }
            } else if (magFilterStr == "linear") {
                if (minFilterStr == "point") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_POINT_MAG_LINEAR;
                } else if (minFilterStr == "linear") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_LINEAR;
                } else if (minFilterStr == "mipmapPoint") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                } else if (minFilterStr == "mipmapLinear") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_POINT_MAG_MIP_LINEAR;
                } else if (minFilterStr == "linearMipmapPoint") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_LINEAR_MIP_POINT;
                } else if (minFilterStr == "linearMipmapLinear") {
                    samplerDesc._filter = Sampler::Filter::FILTER_MIN_MAG_MIP_LINEAR;
                }
            }
        }
    }

    auto wrapItr = object.constFind("wrap");
    if (wrapItr != object.constEnd() && wrapItr->isString()) {
        auto wrapStr = wrapItr->toString();
        samplerDesc._wrapModeU = wrapModeFromString(wrapStr);
        samplerDesc._wrapModeV = samplerDesc._wrapModeU;
    } else {
        auto wrapSItr = object.constFind("wrapS");
        if (wrapSItr != object.constEnd() && wrapSItr->isString()) {
            auto wrapSStr = wrapSItr->toString();
            samplerDesc._wrapModeU = wrapModeFromString(wrapSStr);
        }

        auto wrapTItr = object.constFind("wrapT");
        if (wrapTItr != object.constEnd() && wrapTItr->isString()) {
            auto wrapTStr = wrapTItr->toString();
            samplerDesc._wrapModeV = wrapModeFromString(wrapTStr);
        }
    }

    auto borderColorItr = object.constFind("borderColor");
    if (borderColorItr != object.constEnd()) {
        samplerDesc._borderColor = vec4FromVariant(borderColorItr->toVariant());
    }

    return Sampler(samplerDesc);
}

QDebug& operator<<(QDebug& dbg, const Sampler& s) {
    QString result = "[ ";

    result += "borderColor: (";
    result += s.getBorderColor().r;
    result += ", ";
    result += s.getBorderColor().g;
    result += ", ";
    result += s.getBorderColor().b;
    result += ", ";
    result += s.getBorderColor().a;
    result += "), ";

    result += "maxAnistropy: ";
    result += s.getMaxAnisotropy();
    result += ", ";

    result += "filter: ";
    result += s.getFilter();
    result += ", ";

    result += "comparisonFunction: ";
    result += (uint8_t)s.getComparisonFunction();
    result += ", ";

    result += "wrap: (";
    result += s.getWrapModeU();
    result += ", ";
    result += s.getWrapModeV();
    result += ", ";
    result += s.getWrapModeW();
    result += "), ";

    result += "mipOffset: ";
    result += s.getMipOffset();
    result += ", ";

    result += "minMip: ";
    result += s.getMinMip();
    result += ", ";

    result += "maxMip: ";
    result += s.getMaxMip();
    result += ", ";

    result += "]";
    dbg.nospace() << result;
    return dbg;
}
