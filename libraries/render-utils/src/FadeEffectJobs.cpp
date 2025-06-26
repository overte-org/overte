//
//  FadeEffectJobs.cpp

//  Created by Olivier Prat on 07/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "FadeEffectJobs.h"
#include "render/Logging.h"
#include "render/TransitionStage.h"

#include "FadeObjectParams.shared.slh"

#include <NumericalConstants.h>
#include <Interpolate.h>
#include <gpu/Context.h>

#include <QJsonArray>

#include <PathUtils.h>

#define FADE_MIN_SCALE  0.001
#define FADE_MAX_SCALE  10000.0
#define FADE_MAX_SPEED  50.f

inline float parameterToValuePow(float parameter, const double minValue, const double maxOverMinValue) {
    return (float)(minValue * pow(maxOverMinValue, double(parameter)));
}

inline float valueToParameterPow(float value, const double minValue, const double maxOverMinValue) {
    return (float)(log(double(value) / minValue) / log(maxOverMinValue));
}

void FadeEditJob::configure(const Config& config) {
    _isEditEnabled = config.editFade;
    _editedCategory =  std::min<int>((int)TransitionType::TYPE_COUNT, config.editedCategory);
}

void FadeEditJob::run(const render::RenderContextPointer& renderContext, Output& output) {
    auto scene = renderContext->_scene;
    output = render::Item::INVALID_ITEM_ID;

    if (_isEditEnabled) {
        static const std::string selectionName("TransitionEdit");
        auto scene = renderContext->_scene;
        if (!scene->isSelectionEmpty(selectionName)) {
            auto selection = scene->getSelection(selectionName);
            auto editedItem = selection.getItems().front();
            output = editedItem;
            render::Transaction transaction;
            bool hasTransaction{ false };

            if (editedItem != _editedItem && render::Item::isValidID(_editedItem)) {
                // Remove transition from previously edited item as we've changed edited item
                hasTransaction = true;
                transaction.removeTransitionFromItem(_editedItem);
            }
            _editedItem = editedItem;

            if (render::Item::isValidID(_editedItem)) {
                auto transitionType = (TransitionType)_editedCategory;
                transaction.queryTransitionOnItem(_editedItem, [transitionType, scene](render::ItemID id, const render::Transition* transition) {
                    if (transition == nullptr || transition->isFinished || transition->eventType != transitionType) {
                        // Relaunch transition
                        render::Transaction transaction;
                        transaction.resetTransitionOnItem(id, transitionType);
                        scene->enqueueTransaction(transaction);
                    }
                });
                hasTransaction = true;
            }

            if (hasTransaction) {
                scene->enqueueTransaction(transaction);
            }
        } else if (render::Item::isValidID(_editedItem)) {
            // Remove transition from previously edited item as we've disabled fade edition
            render::Transaction transaction;
            transaction.removeTransitionFromItem(_editedItem);
            scene->enqueueTransaction(transaction);
            _editedItem = render::Item::INVALID_ITEM_ID;
        }
    }
    else if (render::Item::isValidID(_editedItem)) {
        // Remove transition from previously edited item as we've disabled fade edition
        render::Transaction transaction;
        transaction.removeTransitionFromItem(_editedItem);
        scene->enqueueTransaction(transaction);
        _editedItem = render::Item::INVALID_ITEM_ID;
    }
}

FadeConfig::FadeConfig() {}

void FadeConfig::setDuration(float value) {
    props.duration = value;
    emit dirty();
}

float FadeConfig::getDuration() const { 
    return props.duration;
}

void FadeConfig::setBaseSizeX(float value) {
    props.baseSizeInv.x = 1.0f / parameterToValuePow(value, FADE_MIN_SCALE, FADE_MAX_SCALE/ FADE_MIN_SCALE);
    emit dirty();
}

float FadeConfig::getBaseSizeX() const { 
    return valueToParameterPow(1.0f / props.baseSizeInv.x, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
}

void FadeConfig::setBaseSizeY(float value) {
    props.baseSizeInv.y = 1.0f / parameterToValuePow(value, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
    emit dirty();
}

float FadeConfig::getBaseSizeY() const {
    return valueToParameterPow(1.0f / props.baseSizeInv.y, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
}

void FadeConfig::setBaseSizeZ(float value) {
    props.baseSizeInv.z = 1.0f / parameterToValuePow(value, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
    emit dirty();
}

float FadeConfig::getBaseSizeZ() const {
    return valueToParameterPow(1.0f / props.baseSizeInv.z, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
}

void FadeConfig::setBaseLevel(float value) {
    props.baseLevel = value;
    emit dirty();
}

void FadeConfig::setInverted(bool value) {
    props.inverted = value;
    emit dirty();
}

bool FadeConfig::isInverted() const { 
    return props.inverted;
}

void FadeConfig::setNoiseSizeX(float value) {
    props.noiseSizeInv.x = 1.0f / parameterToValuePow(value, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
    emit dirty();
}

float FadeConfig::getNoiseSizeX() const {
    return valueToParameterPow(1.0f / props.noiseSizeInv.x, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
}

void FadeConfig::setNoiseSizeY(float value) {
    props.noiseSizeInv.y = 1.0f / parameterToValuePow(value, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
    emit dirty();
}

float FadeConfig::getNoiseSizeY() const {
    return valueToParameterPow(1.0f / props.noiseSizeInv.y, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
}

void FadeConfig::setNoiseSizeZ(float value) {
    props.noiseSizeInv.z = 1.0f / parameterToValuePow(value, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
    emit dirty();
}

float FadeConfig::getNoiseSizeZ() const {
    return valueToParameterPow(1.0f / props.noiseSizeInv.z, FADE_MIN_SCALE, FADE_MAX_SCALE / FADE_MIN_SCALE);
}

void FadeConfig::setNoiseLevel(float value) {
    props.noiseLevel = value;
    emit dirty();
}

void FadeConfig::setNoiseSpeedX(float value) {
    props.noiseSpeed.x = powf(value, 3.f)*FADE_MAX_SPEED;
    emit dirty();
}

float FadeConfig::getNoiseSpeedX() const {
    return powf(props.noiseSpeed.x / FADE_MAX_SPEED, 1.f / 3.f);
}

void FadeConfig::setNoiseSpeedY(float value) {
    props.noiseSpeed.y = powf(value, 3.f)*FADE_MAX_SPEED;
    emit dirty();
}

float FadeConfig::getNoiseSpeedY() const {
    return powf(props.noiseSpeed.y / FADE_MAX_SPEED, 1.f / 3.f);
}

void FadeConfig::setNoiseSpeedZ(float value) {
    props.noiseSpeed.z = powf(value, 3.f)*FADE_MAX_SPEED;
    emit dirty();
}

float FadeConfig::getNoiseSpeedZ() const {
    return powf(props.noiseSpeed.z / FADE_MAX_SPEED, 1.f / 3.f);
}

void FadeConfig::setEdgeWidth(float value) {
    props.edgeWidth = value * value;
    emit dirty();
}

float FadeConfig::getEdgeWidth() const { 
    return sqrtf(props.edgeWidth);
}

void FadeConfig::setEdgeInnerColor(const QColor& value) {
    props.innerEdgeColor.r = value.redF();
    props.innerEdgeColor.g = value.greenF();
    props.innerEdgeColor.b = value.blueF();
    emit dirty();
}

QColor FadeConfig::getEdgeInnerColor() const {
    QColor color;
    color.setRedF(props.innerEdgeColor.r);
    color.setGreenF(props.innerEdgeColor.g);
    color.setBlueF(props.innerEdgeColor.b);
    color.setAlphaF(1.0f);
    return color;
}

void FadeConfig::setEdgeInnerIntensity(float value) {
    props.innerEdgeColor.a = value;
    emit dirty();
}

void FadeConfig::setEdgeOuterColor(const QColor& value) {
    props.outerEdgeColor.r = value.redF();
    props.outerEdgeColor.g = value.greenF();
    props.outerEdgeColor.b = value.blueF();
    emit dirty();
}

QColor FadeConfig::getEdgeOuterColor() const {
    QColor color;
    color.setRedF(props.outerEdgeColor.r);
    color.setGreenF(props.outerEdgeColor.g);
    color.setBlueF(props.outerEdgeColor.b);
    color.setAlphaF(1.0f);
    return color;
}

void FadeConfig::setEdgeOuterIntensity(float value) {
    props.outerEdgeColor.a = value;
    emit dirty();
}

void FadeConfig::setTiming(int value) {
    assert(value < TIMING_COUNT);
    props.timing = (FadeTiming)value;
    emit dirty();
}

FadeJob::FadeJob() {
    _previousTime = usecTimestampNow();
}

void FadeJob::configure(const Config& config) {
    _editedFadeProperties = config.props;
}

void FadeJob::run(const render::RenderContextPointer& renderContext, const Input& input) {
    Config* jobConfig = static_cast<Config*>(renderContext->jobConfig.get());

    auto scene = renderContext->args->_scene;
    auto transitionStage = scene->getStage<render::TransitionStage>();
    uint64_t now = usecTimestampNow();
    const double deltaTime = (int64_t(now) - int64_t(_previousTime)) / double(USECS_PER_SECOND);
    render::Transaction transaction;
    bool isFirstItem = true;
    bool hasTransaction = false;

    // And now update fade effect
    for (auto transitionId : *transitionStage) {
        auto& state = transitionStage->editElement(transitionId);
#ifdef DEBUG
        auto& item = scene->getItem(state.itemId);
        assert(item.getTransitionId() == transitionId);
#endif
        if (update(renderContext->args, *jobConfig, scene, transaction, state, deltaTime, input)) {
            hasTransaction = true;
        }
        if (isFirstItem && (state.threshold != jobConfig->threshold)) {
            jobConfig->setProperty("threshold", state.threshold);
            isFirstItem = false;
        }
    }
    _previousTime = now;
    if (hasTransaction) {
        scene->enqueueTransaction(transaction);
    }
}

bool FadeJob::update(RenderArgs* args,
                     const Config& config,
                     const render::ScenePointer& scene,
                     render::Transaction& transaction,
                     render::Transition& transition,
                     const double deltaTime,
                     const render::ItemID editedItemID) const {
    auto item = scene->getItemSafe(transition.itemId);
    if (!item.exist()) {
        return false;
    }

    bool hasParamsBuffer = transition.paramsBuffer._size == sizeof(FadeObjectParams);
    FadeProperties fadeProperties;
    if (transition.itemId == editedItemID) {
        fadeProperties = _editedFadeProperties;
    } else {
        fadeProperties = hasParamsBuffer ? transition.fadeProperties : item.getFadeProperties(transition.eventType);
    }
    const double eventDuration = (double)fadeProperties.duration;
    const FadeTiming timing = fadeProperties.timing;

    auto aabb = item.getBound(args);
    if (render::Item::isValidID(transition.boundItemId)) {
        auto boundItem = scene->getItemSafe(transition.boundItemId);
        if (boundItem.exist()) {
            aabb = boundItem.getBound(args);
        }
    }
    auto& dimensions = aabb.getDimensions();

    assert(timing < FadeConfig::TIMING_COUNT);

    transition.noiseOffset = aabb.calcCenter();
    transition.baseInvSize = fadeProperties.baseSizeInv;

    switch (transition.eventType) {
    case TransitionType::ELEMENT_ENTER_DOMAIN:
    case TransitionType::ELEMENT_LEAVE_DOMAIN:
    {
        transition.threshold = computeElementEnterRatio(transition.time, eventDuration, timing);
        transition.baseOffset = transition.noiseOffset;
        transition.isFinished += (transition.threshold >= 1.0f) & 1;
        if (transition.eventType == TransitionType::ELEMENT_ENTER_DOMAIN) {
            transition.threshold = 1.0f - transition.threshold;
        }
    }
    break;

    case TransitionType::BUBBLE_ISECT_OWNER:
    {
        transition.threshold = 0.5f;
        transition.baseOffset = transition.noiseOffset;
    }
    break;

    case TransitionType::BUBBLE_ISECT_TRESPASSER:
    {
        transition.threshold = 0.5f;
        transition.baseOffset = transition.noiseOffset;
    }
    break;

    case TransitionType::USER_ENTER_DOMAIN:
    case TransitionType::USER_LEAVE_DOMAIN:
    {
        transition.threshold = computeElementEnterRatio(transition.time, eventDuration, timing);
        transition.baseOffset = transition.noiseOffset - dimensions.y / 2.f;
        transition.baseInvSize.y = 1.f / dimensions.y;
        transition.isFinished += (transition.threshold >= 1.0f) & 1;
        if (transition.eventType == TransitionType::USER_LEAVE_DOMAIN) {
            transition.threshold = 1.f - transition.threshold;
        }
    }
    break;

    case TransitionType::AVATAR_CHANGE:
        break;

    default:
        assert(false);
    }

    transition.noiseOffset += fadeProperties.noiseSpeed * (float)transition.time;
    if (config.manualFade) {
        transition.threshold = config.manualThreshold;
    }
    transition.threshold = std::max(0.f, std::min(1.f, transition.threshold));
    transition.threshold = (transition.threshold - 0.5f);
    transition.threshold *= 1.0f + (fadeProperties.edgeWidth + std::max(0.0f, (fadeProperties.noiseLevel + fadeProperties.baseLevel) * 0.5f - 0.5f));
    transition.threshold += 0.5f;
    transition.time += deltaTime;

    if (!hasParamsBuffer) {
        static_assert(sizeof(transition.paramsBuffer) == sizeof(gpu::StructBuffer<FadeObjectParams>),
                      "Assuming gpu::StructBuffer is a helper class for gpu::BufferView");
        transition.paramsBuffer = gpu::StructBuffer<FadeObjectParams>();
    }

    auto& params = static_cast<gpu::StructBuffer<FadeObjectParams>&>(transition.paramsBuffer).edit();
    if (!hasParamsBuffer) {
        params.noiseOffsetAndInverted.w = fadeProperties.inverted;
        params.baseInvSizeAndLevel.w = fadeProperties.baseLevel;
        params.noiseInvSizeAndLevel = vec4(fadeProperties.noiseSizeInv, fadeProperties.noiseLevel);
        params.innerEdgeColor = fadeProperties.innerEdgeColor;
        params.outerEdgeColor = fadeProperties.outerEdgeColor;
        params.edgeWidthInv = 1.0f / fadeProperties.edgeWidth;

        transition.fadeProperties = fadeProperties;
    }

    params.noiseOffsetAndInverted = vec4(transition.noiseOffset, params.noiseOffsetAndInverted.w);
    params.baseOffsetAndThreshold = vec4(transition.baseOffset, transition.threshold);
    params.baseInvSizeAndLevel = vec4(transition.baseInvSize, params.baseInvSizeAndLevel.w);

    // If the transition is finished for more than a number of frames (here 1), garbage collect it.
    if (transition.isFinished > 1) {
        transaction.removeTransitionFromItem(transition.itemId);
        return true;
    }
    return false;
}

float FadeJob::computeElementEnterRatio(double time, const double period, FadeTiming timing) {
    assert(period > 0.0);
    float fadeAlpha = 1.0f;
    const double INV_FADE_PERIOD = 1.0 / period;
    double fraction = time * INV_FADE_PERIOD;
    fraction = std::max(fraction, 0.0);
    if (fraction < 1.0) {
        switch (timing) {
        default:
            fadeAlpha = (float)fraction;
            break;
        case FadeTiming::EASE_IN:
            fadeAlpha = (float)(fraction*fraction*fraction);
            break;
        case FadeTiming::EASE_OUT:
            fadeAlpha = 1.f - (float)fraction;
            fadeAlpha = 1.f- fadeAlpha*fadeAlpha*fadeAlpha;
            break;
        case FadeTiming::EASE_IN_OUT:
            fadeAlpha = (float)(fraction*fraction*fraction*(fraction*(fraction * 6 - 15) + 10));
            break;
        }
    }
    return fadeAlpha;
}
