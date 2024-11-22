//
//  Created by Bradley Austin Davis on 2015/07/16
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Font_h
#define hifi_Font_h

#include <QObject>

#include "Glyph.h"
#include "TextEffect.h"
#include "TextAlignment.h"
#include "TextVerticalAlignment.h"
#include <gpu/Batch.h>
#include <gpu/Pipeline.h>

class Font : public QObject {
    Q_OBJECT

public:
    using Pointer = std::shared_ptr<Font>;

    Font(const QString& family);

    void read(QIODevice& path);

    struct DrawParams {
        vec4 bounds { 0.0f };
        vec4 color { 0.0f };

        vec2 unitRange { 1.0f };

        int effect { 0 };
        float effectThickness { 0.0f };

        vec3 effectColor { 0.0f };

#if defined(__clang__)
        __attribute__((unused))
#endif
        float _spare;
    };

    struct DrawInfo {
        gpu::BufferPointer verticesBuffer { nullptr };
        gpu::BufferPointer indicesBuffer { nullptr };
        gpu::BufferPointer paramsBuffer { nullptr };
        uint32_t indexCount;

        QString string;
        glm::vec2 origin;
        glm::vec2 bounds;
        DrawParams params;

        float scale { 0.0f };
        TextAlignment alignment { TextAlignment::LEFT };
        TextVerticalAlignment verticalAlignment { TextVerticalAlignment::TOP };
    };

    glm::vec2 computeExtent(const QString& str) const;
    float getFontHeight() const { return _fontHeight; }

    struct DrawProps {
        DrawProps(const QString& str, const glm::vec4& color, const glm::vec3& effectColor, const glm::vec2& origin, const glm::vec2& bounds,
                  float scale, float effectThickness, TextEffect effect, TextAlignment alignment, TextVerticalAlignment verticalAlignment, bool unlit,
                  bool forward, bool mirror) :
            str(str), color(color), effectColor(effectColor), origin(origin), bounds(bounds), scale(scale), effectThickness(effectThickness),
            effect(effect), alignment(alignment), verticalAlignment(verticalAlignment), unlit(unlit), forward(forward), mirror(mirror) {}
            
        DrawProps(const QString& str, const glm::vec4& color, const glm::vec2& origin, const glm::vec2& bounds, bool forward, TextAlignment alignment) :
            str(str), color(color), origin(origin), bounds(bounds), forward(forward), alignment(alignment) {}

        const QString& str;
        const glm::vec4& color;
        const glm::vec3& effectColor { glm::vec3(0.0f) };
        const glm::vec2& origin;
        const glm::vec2& bounds;
        float scale { 1.0f };
        float effectThickness { 0.0f };
        TextEffect effect { TextEffect::NO_EFFECT };
        TextAlignment alignment { TextAlignment::LEFT };
        TextVerticalAlignment verticalAlignment { TextVerticalAlignment::TOP };
        bool unlit = true;
        bool forward;
        bool mirror = false;
    };

    // Render string to batch
    void drawString(gpu::Batch& batch, DrawInfo& drawInfo, const DrawProps& props);

    static Pointer load(const QString& family);

    bool isLoaded() const { return _loaded; }

public slots:
    void handleFontNetworkReply();

private:
    static Pointer load(const QString& family, QIODevice& fontFile);
    QStringList tokenizeForWrapping(const QString& str) const;
    QStringList splitLines(const QString& str) const;
    float computeTokenWidth(const QString& str) const;

    const Glyph& getGlyph(const QChar& c) const;
    void buildVertices(DrawInfo& drawInfo, const QString& str, const glm::vec2& origin, const glm::vec2& bounds, float scale, bool enlargeForShadows,
                       TextAlignment alignment, TextVerticalAlignment verticalAlignment);

    void setupGPU();

    // maps characters to cached glyph info
    // HACK... the operator[] const for QHash returns a
    // copy of the value, not a const value reference, so
    // we declare the hash as mutable in order to avoid such
    // copies
    mutable QHash<QChar, Glyph> _glyphs;

    // Font characteristics
    QString _family;
    glm::vec2 _distanceRange { 1.0f };
    float _fontHeight { 0.0f };
    float _leading { 0.0f };
    float _spaceWidth { 0.0f };

    bool _loaded { false };
    bool _needsParamsUpdate { false };

    gpu::TexturePointer _texture;
    gpu::BufferStreamPointer _stream;

    static std::map<std::tuple<bool, bool, bool, bool>, gpu::PipelinePointer> _pipelines;
    static gpu::Stream::FormatPointer _format;
};

#endif

