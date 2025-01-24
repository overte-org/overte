//
//  HighlightStage.cpp
//
//  Created by Olivier Prat on 07/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "HighlightStage.h"

#include "Engine.h"

using namespace render;

template <>
std::string TypedStage<Highlight>::_name { "HIGHLIGHT_STAGE" };

HighlightStage::Index HighlightStage::addHighlight(const std::string& selectionName, const HighlightStyle& style) {
    Highlight outline { selectionName, style };
    return addElement(outline);
}

HighlightStage::Index HighlightStage::getHighlightIdBySelection(const std::string& selectionName) const {
    for (auto outlineId : _activeElementIDs) {
        const auto& outline = _elements.get(outlineId);
        if (outline._selectionName == selectionName) {
            return outlineId;
        }
    }
    return INVALID_INDEX;
}

const HighlightStyle& HighlightStageConfig::getStyle() const {
    auto styleIterator = _styles.find(_selectionName);
    if (styleIterator != _styles.end()) {
        return styleIterator->second;
    } else {
        auto insertion = _styles.insert(SelectionStyles::value_type{ _selectionName, HighlightStyle{} });
        return insertion.first->second;
    }
}

HighlightStyle& HighlightStageConfig::editStyle() {
    auto styleIterator = _styles.find(_selectionName);
    if (styleIterator != _styles.end()) {
        return styleIterator->second;
    } else {
        auto insertion = _styles.insert(SelectionStyles::value_type{ _selectionName, HighlightStyle{} });
        return insertion.first->second;
    }
}

void HighlightStageConfig::setSelectionName(const QString& name) {
    _selectionName = name.toStdString();
    emit dirty();
}

void HighlightStageConfig::setOutlineSmooth(bool isSmooth) {
    editStyle()._isOutlineSmooth = isSmooth;
    emit dirty();
}

void HighlightStageConfig::setColorRed(float value) {
    editStyle()._outlineUnoccluded.color.r = value;
    emit dirty();
}

void HighlightStageConfig::setColorGreen(float value) {
    editStyle()._outlineUnoccluded.color.g = value;
    emit dirty();
}

void HighlightStageConfig::setColorBlue(float value) {
    editStyle()._outlineUnoccluded.color.b = value;
    emit dirty();
}

void HighlightStageConfig::setOutlineWidth(float value) {
    editStyle()._outlineWidth = value;
    emit dirty();
}

void HighlightStageConfig::setOutlineIntensity(float value) {
    editStyle()._outlineUnoccluded.alpha = value;
    emit dirty();
}

void HighlightStageConfig::setUnoccludedFillOpacity(float value) {
    editStyle()._fillUnoccluded.alpha = value;
    emit dirty();
}

void HighlightStageConfig::setOccludedFillOpacity(float value) {
    editStyle()._fillOccluded.alpha = value;
    emit dirty();
}

void HighlightStageSetup::configure(const Config& config) {
    // Copy the styles here but update the stage with the new styles in run to be sure everything is
    // thread safe...
    _styles = config._styles;
}

void HighlightStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage<HighlightStage>(HighlightStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(HighlightStage::getName(), std::make_shared<HighlightStage>());
    }

    if (!_styles.empty()) {
        render::Transaction transaction;
        for (const auto& selection : _styles) {
            auto& selectionName = selection.first;
            auto& selectionStyle = selection.second;
            transaction.resetSelectionHighlight(selectionName, selectionStyle);
        }
        renderContext->_scene->enqueueTransaction(transaction);
        _styles.clear();
    }
}
