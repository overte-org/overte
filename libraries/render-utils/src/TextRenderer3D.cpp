//
//  TextRenderer3D.cpp
//  interface/src/ui
//
//  Created by Andrzej Kapolka on 4/24/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TextRenderer3D.h"

#include "text/Font.h"

TextRenderer3D* TextRenderer3D::getInstance(const char* family) {
    return new TextRenderer3D(family);
}

TextRenderer3D::TextRenderer3D(const char* family) :
    _family(family),
    _font(Font::load(family)) {
    if (!_font) {
        qWarning() << "Unable to load font with family " << family;
        _font = Font::load(ROBOTO_FONT_FAMILY);
    }
}

glm::vec2 TextRenderer3D::computeExtent(const QString& str) const {
    if (_font) {
        return _font->computeExtent(str);
    }
    return glm::vec2(0.0f, 0.0f);
}

float TextRenderer3D::getFontHeight() const {
    if (_font) {
        return _font->getFontHeight();
    }
    return 0.0f;
}

void TextRenderer3D::draw(gpu::Batch& batch, const Font::DrawProps& props) {
    if (_font) {
        _font->drawString(batch, _drawInfo, props);
    }
}

void TextRenderer3D::draw(gpu::Batch& batch, const QString& font, const Font::DrawProps& props) {
    if (font != _family) {
        _family = font;
        _font = Font::load(_family);
    }
    if (_font) {
        _font->drawString(batch, _drawInfo, props);
    }
}