//
//  Copyright 2013-2019 High Fidelity, Inc.
//  Copyright 2020-2021 Vircadia contributors
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Font.h"

#include <QFile>
#include <QImage>
#include <QNetworkReply>
#include <QThreadStorage>

#include "artery-font/artery-font.h"
#include "artery-font/std-artery-font.h"

#include <ColorUtils.h>

#include <StreamHelpers.h>
#include <shaders/Shaders.h>

#include "../render-utils/ShaderConstants.h"
#include "../RenderUtilsLogging.h"
#include "FontFamilies.h"
#include "../StencilMaskPass.h"

#include "NetworkAccessManager.h"
#include "NetworkingConstants.h"

static std::mutex fontMutex;

std::map<std::tuple<bool, bool, bool, bool>, gpu::PipelinePointer> Font::_pipelines;
gpu::Stream::FormatPointer Font::_format;

struct TextureVertex {
    glm::vec2 pos;
    glm::vec2 tex;
    glm::vec4 bounds;
    TextureVertex() {}
    TextureVertex(const glm::vec2& pos, const glm::vec2& tex, const glm::vec4& bounds) : pos(pos), tex(tex), bounds(bounds) {}
};

static const int NUMBER_OF_INDICES_PER_QUAD = 6;  // 1 quad = 2 triangles
static const int VERTICES_PER_QUAD = 4;           // 1 quad = 4 vertices (must match value in sdf_text3D.slv)
const float DOUBLE_MAX_OFFSET_PIXELS = 20.0f;     // must match value in sdf_text3D.slh

struct QuadBuilder {
    TextureVertex vertices[VERTICES_PER_QUAD];

    QuadBuilder(const Glyph& glyph, const glm::vec2& offset, float scale, bool enlargeForShadows) {
        glm::vec2 min = offset + glm::vec2(glyph.offset.x, glyph.offset.y - glyph.size.y);
        glm::vec2 size = glyph.size;
        glm::vec2 texMin = glyph.texOffset;
        glm::vec2 texSize = glyph.texSize;

        // We need the pre-adjustment bounds for clamping
        glm::vec4 bounds = glm::vec4(texMin, texSize);
        if (enlargeForShadows) {
            glm::vec2 imageSize = glyph.size / glyph.texSize;
            glm::vec2 sizeDelta = 0.5f * DOUBLE_MAX_OFFSET_PIXELS * scale * imageSize;
            glm::vec2 oldSize = size;
            size += sizeDelta;
            min.y -= sizeDelta.y;

            texSize = texSize * (size / oldSize);
        }

        // min = bottomLeft
        vertices[0] = TextureVertex(min,
                                    texMin + glm::vec2(0.0f, texSize.y), bounds);
        vertices[1] = TextureVertex(min + glm::vec2(size.x, 0.0f),
                                    texMin + texSize, bounds);
        vertices[2] = TextureVertex(min + glm::vec2(0.0f, size.y),
                                    texMin, bounds);
        vertices[3] = TextureVertex(min + size,
                                    texMin + glm::vec2(texSize.x, 0.0f), bounds);
    }
};

Font::Pointer Font::load(const QString& family, QIODevice& fontFile) {
    Pointer font = std::make_shared<Font>(family);
    font->read(fontFile);
    return font;
}

void Font::handleFontNetworkReply() {
    auto requestReply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(requestReply != nullptr);

    if (requestReply->error() == QNetworkReply::NoError) {
        read(*requestReply);
    } else {
        qDebug() << "Error downloading " << requestReply->url() << " - " << requestReply->errorString();
    }
}

QThreadStorage<size_t> _readOffset;
QThreadStorage<size_t> _readMax;
int readHelper(void* dst, int length, void* data) {
    if (_readOffset.localData() + length > _readMax.localData()) {
        return -1;
    }
    memcpy(dst, (char *)data + _readOffset.localData(), length);
    _readOffset.setLocalData(_readOffset.localData() + length);
    return length;
};

void Font::read(QIODevice& in) {
    _loaded = false;

    QByteArray data = in.readAll();
    _readOffset.setLocalData(0);
    _readMax.setLocalData(data.length());
    artery_font::StdArteryFont<float> arteryFont;
    bool success = artery_font::decode<&readHelper, float, artery_font::StdList, artery_font::StdByteArray, artery_font::StdString>(arteryFont, (void *)data.data());

    if (!success) {
        qDebug() << "Font" << _family << "failed to decode.";
        return;
    }

    if (arteryFont.variants.length() == 0) {
        qDebug() << "Font" << _family << "has 0 variants.";
        return;
    }

    _distanceRange = glm::vec2(arteryFont.variants[0].metrics.distanceRange);
    _fontHeight = arteryFont.variants[0].metrics.ascender + fabs(arteryFont.variants[0].metrics.descender);
    _leading = arteryFont.variants[0].metrics.lineHeight;
    _spaceWidth = 0.5f * arteryFont.variants[0].metrics.emSize; // We use half the emSize as a first guess for _spaceWidth

    if (arteryFont.variants[0].glyphs.length() == 0) {
        qDebug() << "Font" << _family << "has 0 glyphs.";
        return;
    }

    QVector<Glyph> glyphs;
    glyphs.reserve(arteryFont.variants[0].glyphs.length());
    for (int i = 0; i < arteryFont.variants[0].glyphs.length(); i++) {
        auto& g = arteryFont.variants[0].glyphs[i];

        Glyph glyph;
        glyph.c = g.codepoint;
        glyph.texOffset = glm::vec2(g.imageBounds.l, g.imageBounds.b);
        glyph.texSize = glm::vec2(g.imageBounds.r, g.imageBounds.t) - glyph.texOffset;
        glyph.offset = glm::vec2(g.planeBounds.l, g.planeBounds.b);
        glyph.size = glm::vec2(g.planeBounds.r, g.planeBounds.t) - glyph.offset;
        glyph.d = g.advance.h;
        glyphs.push_back(glyph);

        // If we find the space character, we save its size in _spaceWidth for later
        if (glyph.c == ' ') {
            _spaceWidth = glyph.d;
        }
    }

    if (arteryFont.images.length() == 0) {
        qDebug() << "Font" << _family << "has 0 images.";
        return;
    }

    if (arteryFont.images[0].imageType != artery_font::ImageType::IMAGE_MTSDF) {
        qDebug() << "Font" << _family << "has the wrong image type.  Expected MTSDF (7), got" << arteryFont.images[0].imageType;
        return;
    }

    if (arteryFont.images[0].encoding != artery_font::ImageEncoding::IMAGE_PNG) {
        qDebug() << "Font" << _family << "has the wrong encoding.  Expected PNG (8), got" << arteryFont.images[0].encoding;
        return;
    }

    if (arteryFont.images[0].pixelFormat != artery_font::PixelFormat::PIXEL_UNSIGNED8) {
        qDebug() << "Font" << _family << "has the wrong pixel format.  Expected unsigned char (8), got" << arteryFont.images[0].pixelFormat;
        return;
    }

    if (arteryFont.images[0].width == 0 || arteryFont.images[0].height == 0) {
        qDebug() << "Font" << _family << "has image with width or height of 0.  Width:" << arteryFont.images[0].width << ", height:"<< arteryFont.images[0].height;
        return;
    }

    // read image data
    QImage image;
    if (!image.loadFromData((const unsigned char*)arteryFont.images[0].data, arteryFont.images[0].data.length(), "PNG")) {
        qDebug() << "Failed to read image for font" << _family;
        return;
    }

    _glyphs.clear();
    glm::vec2 imageSize = toGlm(image.size());
    _distanceRange /= imageSize;
    foreach(Glyph g, glyphs) {
        // Adjust the pixel texture coordinates into UV coordinates,
        g.texSize /= imageSize;
        g.texOffset /= imageSize;
        // Y flip
        g.texOffset.y = 1.0f - (g.texOffset.y + g.texSize.y);
        g.offset.y = -(1.0f - (g.offset.y + g.size.y));
        // store in the character to glyph hash
        _glyphs[g.c] = g;
    };

    image = image.convertToFormat(QImage::Format_RGBA8888);

    gpu::Element formatGPU = gpu::Element(gpu::VEC3, gpu::NUINT8, gpu::RGB);
    gpu::Element formatMip = gpu::Element(gpu::VEC3, gpu::NUINT8, gpu::RGB);
    if (image.hasAlphaChannel()) {
        formatGPU = gpu::Element(gpu::VEC4, gpu::NUINT8, gpu::RGBA);
        formatMip = gpu::Element(gpu::VEC4, gpu::NUINT8, gpu::BGRA);
    }
    // FIXME: We're forcing this to use only one mip, and then manually doing anisotropic filtering in the shader,
    // and also calling textureLod.  Shouldn't this just use anisotropic filtering and auto-generate mips?
    // We should also use smoothstep for anti-aliasing, as explained here: https://github.com/libgdx/libgdx/wiki/Distance-field-fonts
    _texture = gpu::Texture::create2D(formatGPU, image.width(), image.height(), gpu::Texture::SINGLE_MIP,
                                      gpu::Sampler(gpu::Sampler::FILTER_MIN_POINT_MAG_LINEAR));
    _texture->setStoredMipFormat(formatMip);
    _texture->assignStoredMip(0, image.sizeInBytes(), image.constBits());
    _texture->setImportant(true);

    _loaded = true;
    _needsParamsUpdate = true;
}

static QHash<QString, Font::Pointer> LOADED_FONTS;

Font::Pointer Font::load(const QString& family) {
    std::lock_guard<std::mutex> lock(fontMutex);
    if (!LOADED_FONTS.contains(family)) {
        QString loadFilename;

        if (family == ROBOTO_FONT_FAMILY) {
            loadFilename = ":/Roboto.arfont";
        } else if (family == INCONSOLATA_FONT_FAMILY) {
            loadFilename = ":/InconsolataMedium.arfont";
        } else if (family == COURIER_FONT_FAMILY) {
            loadFilename = ":/CourierPrime.arfont";
        } else if (family == TIMELESS_FONT_FAMILY) {
            loadFilename = ":/Timeless.arfont";
        } else if (family.startsWith("http")) {
            auto loadingFont = std::make_shared<Font>(family);
            LOADED_FONTS[family] = loadingFont;

            auto& networkAccessManager = NetworkAccessManager::getInstance();

            QNetworkRequest networkRequest;
            networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
            networkRequest.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
            networkRequest.setUrl(family);

            auto networkReply = networkAccessManager.get(networkRequest);
            connect(networkReply, &QNetworkReply::finished, loadingFont.get(), &Font::handleFontNetworkReply);
        } else if (!LOADED_FONTS.contains(ROBOTO_FONT_FAMILY)) {
            // Unrecognized font and we haven't loaded Roboto yet
            loadFilename = ":/Roboto.arfont";
        } else {
            // Unrecognized font but we've already loaded Roboto
            LOADED_FONTS[family] = LOADED_FONTS[ROBOTO_FONT_FAMILY];
        }

        if (!loadFilename.isEmpty()) {
            QFile fontFile(loadFilename);
            fontFile.open(QIODevice::ReadOnly);

            qCDebug(renderutils) << "Loaded font" << loadFilename << "from Qt Resource System.";

            LOADED_FONTS[family] = load(family, fontFile);
        }
    }
    return LOADED_FONTS[family];
}

Font::Font(const QString& family) : _family(family) {
    static std::once_flag once;
    std::call_once(once, []{
        Q_INIT_RESOURCE(fonts);
    });
}

// NERD RAGE: why doesn't QHash have a 'const T & operator[] const' member
const Glyph& Font::getGlyph(const QChar& c) const {
    if (!_glyphs.contains(c)) {
        return _glyphs[QChar('?')];
    }
    return _glyphs[c];
}

QStringList Font::splitLines(const QString& str) const {
    return str.split('\n');
}

QStringList Font::tokenizeForWrapping(const QString& str) const {
    QStringList tokens;
    for(auto line : splitLines(str)) {
        if (!tokens.empty()) {
            tokens << QString('\n');
        }
        tokens << line.split(' ');
    }
    return tokens;
}

float Font::computeTokenWidth(const QString& token) const {
    float advance = 0.0f;
    foreach(QChar c, token) {
        Q_ASSERT(c != '\n');
        advance += (c == ' ') ? _spaceWidth : getGlyph(c).d;
    }
    return advance;
}

glm::vec2 Font::computeExtent(const QString& str) const {
    glm::vec2 extent = glm::vec2(0.0f, 0.0f);

    QStringList lines = splitLines(str);
    if (!lines.empty()) {
        for(const auto& line : lines) {
            float tokenWidth = computeTokenWidth(line);
            extent.x = std::max(tokenWidth, extent.x);
        }
        extent.y = lines.count() * _fontHeight;
    }
    return extent;
}

void Font::setupGPU() {
    if (_pipelines.empty()) {
        using namespace shader::render_utils::program;

        // transparent, unlit, forward
        static const std::vector<std::tuple<bool, bool, bool, uint32_t>> keys = {
            std::make_tuple(false, false, false, sdf_text3D), std::make_tuple(true, false, false, sdf_text3D_translucent),
            std::make_tuple(false, true, false, sdf_text3D_unlit), std::make_tuple(true, true, false, sdf_text3D_translucent_unlit),
            std::make_tuple(false, false, true, sdf_text3D_forward), std::make_tuple(true, false, true, sdf_text3D_forward/*sdf_text3D_translucent_forward*/),
            std::make_tuple(false, true, true, sdf_text3D_translucent_unlit/*sdf_text3D_unlit_forward*/), std::make_tuple(true, true, true, sdf_text3D_translucent_unlit/*sdf_text3D_translucent_unlit_forward*/)
        };
        for (auto& key : keys) {
            bool transparent = std::get<0>(key);
            bool unlit = std::get<1>(key);
            bool forward = std::get<2>(key);

            auto state = std::make_shared<gpu::State>();
            state->setCullMode(gpu::State::CULL_BACK);
            state->setDepthTest(true, !transparent, gpu::LESS_EQUAL);
            state->setBlendFunction(transparent,
                gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA,
                gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);
            if (transparent) {
                PrepareStencil::testMask(*state);
            } else {
                PrepareStencil::testMaskDrawShape(*state);
            }
            _pipelines[std::make_tuple(transparent, unlit, forward, false)] = gpu::Pipeline::create(gpu::Shader::createProgram(std::get<3>(key)), state);
            _pipelines[std::make_tuple(transparent, unlit, forward, true)] = gpu::Pipeline::create(gpu::Shader::createProgram(forward ? sdf_text3D_forward_mirror : sdf_text3D_mirror), state);
        }

        // Sanity checks
        static const int TEX_COORD_OFFSET = offsetof(TextureVertex, tex);
        static const int TEX_BOUNDS_OFFSET = offsetof(TextureVertex, bounds);
        assert(TEX_COORD_OFFSET == sizeof(glm::vec2));
        assert(sizeof(TextureVertex) == 2 * sizeof(glm::vec2) + sizeof(glm::vec4));
        assert(sizeof(QuadBuilder) == 4 * sizeof(TextureVertex));

        // Setup rendering structures
        _format = std::make_shared<gpu::Stream::Format>();
        _format->setAttribute(gpu::Stream::POSITION, 0, gpu::Element(gpu::VEC2, gpu::FLOAT, gpu::XYZ), 0);
        _format->setAttribute(gpu::Stream::TEXCOORD, 0, gpu::Element(gpu::VEC2, gpu::FLOAT, gpu::UV), TEX_COORD_OFFSET);
        _format->setAttribute(gpu::Stream::TEXCOORD1, 0, gpu::Element(gpu::VEC4, gpu::FLOAT, gpu::XYZW), TEX_BOUNDS_OFFSET);
    }
}

inline QuadBuilder adjustedQuadBuilderForAlignmentMode(const Glyph& glyph, glm::vec2 advance, float scale, float enlargeForShadows,
                                                TextAlignment alignment, float rightSpacing, TextVerticalAlignment verticalAlignment, float bottomSpacing) {
    if (alignment == TextAlignment::RIGHT) {
        advance.x += rightSpacing;
    } else if (alignment == TextAlignment::CENTER) {
        advance.x += 0.5f * rightSpacing;
    }
    if (verticalAlignment == TextVerticalAlignment::BOTTOM) {
        advance.y += bottomSpacing;
    } else if (verticalAlignment == TextVerticalAlignment::CENTER) {
        advance.y += 0.5f * bottomSpacing;
    }
    return QuadBuilder(glyph, advance, scale, enlargeForShadows);
}

void Font::buildVertices(Font::DrawInfo& drawInfo, const QString& str, const glm::vec2& origin, const glm::vec2& bounds, float scale, bool enlargeForShadows,
                         TextAlignment alignment, TextVerticalAlignment verticalAlignment) {
    drawInfo.verticesBuffer = std::make_shared<gpu::Buffer>();
    drawInfo.indicesBuffer = std::make_shared<gpu::Buffer>();
    drawInfo.indexCount = 0;
    int numVertices = 0;

    drawInfo.string = str;
    drawInfo.bounds = bounds;
    drawInfo.origin = origin;

    float rightEdge = origin.x + bounds.x;
    float bottomEdge = origin.y - bounds.y;

    // Top left of text
    bool firstTokenOfLine = true;
    glm::vec2 advance = origin;
    std::vector<std::pair<Glyph, vec2>> glyphsAndCorners;
    const QStringList tokens = tokenizeForWrapping(str);
    for (int i = 0; i < tokens.length(); i++) {
        const QString& token = tokens[i];

        if ((bounds.y != -1) && (advance.y < bottomEdge)) {
            // We are out of the y bound, stop drawing
            break;
        }

        bool isNewLine = (token == QString('\n'));
        bool forceNewLine = false;

        // Handle wrapping
        if (!isNewLine && (bounds.x != -1) && (advance.x + computeExtent(token).x > rightEdge)) {
            // We are out of the x bound, force new line
            forceNewLine = true;
        }

        if (isNewLine || (forceNewLine && !firstTokenOfLine)) {
            if (forceNewLine && !firstTokenOfLine) {
                // We want to try this token again on the new line
                i--;
            }

            // Character return, move the advance to a new line
            advance = glm::vec2(origin.x, advance.y - _leading);
            firstTokenOfLine = true;
            // No need to draw anything, go directly to next token
            continue;
        }

        // Draw the token
        for (const QChar& c : token) {
            if (bounds.x != -1 && advance.x > rightEdge) {
                break;
            }
            const Glyph& glyph = _glyphs[c];

            glyphsAndCorners.emplace_back(glyph, advance);

            // Advance by glyph size
            advance.x += glyph.d;
        }

        if (forceNewLine && firstTokenOfLine) {
            // If the first word of a line didn't fit, we draw as many characters as we could, now go to the next line
            // Character return, move the advance to a new line
            advance = glm::vec2(origin.x, advance.y - _leading);
            firstTokenOfLine = true;
        } else {
            // Add space after all non return tokens
            advance.x += _spaceWidth;
            // Our token fits in the x direction!  Any subsequent tokens won't be the first for this line.
            firstTokenOfLine = false;
        }
    }

    std::vector<QuadBuilder> quadBuilders;
    quadBuilders.reserve(glyphsAndCorners.size());
    {
        float bottomSpacing = -FLT_MAX;
        bool foundBottomSpacing = false;
        if (verticalAlignment != TextVerticalAlignment::TOP) {
            int i = (int)glyphsAndCorners.size() - 1;
            while (!foundBottomSpacing && i >= 0) {
                auto* nextGlyphAndCorner = &glyphsAndCorners[i];
                bottomSpacing = std::max(bottomSpacing, bottomEdge - (nextGlyphAndCorner->second.y + (nextGlyphAndCorner->first.offset.y - nextGlyphAndCorner->first.size.y)));
                i--;
                while (i >= 0) {
                    auto& prevGlyphAndCorner = glyphsAndCorners[i];
                    // We're to the right of the last character we checked, which means we're on a previous line, so we can stop
                    if (prevGlyphAndCorner.second.x >= nextGlyphAndCorner->second.x) {
                        foundBottomSpacing = true;
                        break;
                    }
                    nextGlyphAndCorner = &prevGlyphAndCorner;
                    bottomSpacing = std::max(bottomSpacing, bottomEdge - (nextGlyphAndCorner->second.y + (nextGlyphAndCorner->first.offset.y - nextGlyphAndCorner->first.size.y)));
                    i--;
                }
            }
        }

        int i = (int)glyphsAndCorners.size() - 1;
        while (i >= 0) {
            auto* nextGlyphAndCorner = &glyphsAndCorners[i];
            float rightSpacing = rightEdge - (nextGlyphAndCorner->second.x + nextGlyphAndCorner->first.d);
            quadBuilders.push_back(adjustedQuadBuilderForAlignmentMode(nextGlyphAndCorner->first, nextGlyphAndCorner->second, scale, enlargeForShadows,
                                                                       alignment, rightSpacing, verticalAlignment, bottomSpacing));
            i--;
            while (i >= 0) {
                auto& prevGlyphAndCorner = glyphsAndCorners[i];
                // We're to the right of the last character we checked, which means we're on a previous line, so we need to
                // recalculate the spacing, so we exit this loop
                if (prevGlyphAndCorner.second.x >= nextGlyphAndCorner->second.x) {
                    break;
                }

                quadBuilders.push_back(adjustedQuadBuilderForAlignmentMode(prevGlyphAndCorner.first, prevGlyphAndCorner.second, scale, enlargeForShadows,
                                                                           alignment, rightSpacing, verticalAlignment, bottomSpacing));

                nextGlyphAndCorner = &prevGlyphAndCorner;
                i--;
            }
        }
    }

    // The quadBuilders is backwards now because we looped over the glyphs backwards to adjust their alignment
    for (int i = (int)quadBuilders.size() - 1; i >= 0; i--) {
        quint16 verticesOffset = numVertices;
        drawInfo.verticesBuffer->append(quadBuilders[i]);
        numVertices += VERTICES_PER_QUAD;

        // Sam's recommended triangle slices
        // Triangle tri1 = { v0, v1, v3 };
        // Triangle tri2 = { v1, v2, v3 };
        // NOTE: Random guy on the internet's recommended triangle slices
        // Triangle tri1 = { v0, v1, v2 };
        // Triangle tri2 = { v2, v3, v0 };

        // The problem here being that the 4 vertices are { ll, lr, ul, ur }, a Z pattern
        // Additionally, you want to ensure that the shared side vertices are used sequentially
        // to improve cache locality
        //
        //  2 -- 3
        //  |    |
        //  |    |
        //  0 -- 1
        //
        //  { 0, 1, 2 } -> { 2, 1, 3 }
        quint16 indices[NUMBER_OF_INDICES_PER_QUAD];
        indices[0] = verticesOffset + 0;
        indices[1] = verticesOffset + 1;
        indices[2] = verticesOffset + 2;
        indices[3] = verticesOffset + 2;
        indices[4] = verticesOffset + 1;
        indices[5] = verticesOffset + 3;
        drawInfo.indicesBuffer->append(sizeof(indices), (const gpu::Byte*)indices);
        drawInfo.indexCount += NUMBER_OF_INDICES_PER_QUAD;
    }
}

void Font::drawString(gpu::Batch& batch, Font::DrawInfo& drawInfo, const DrawProps& props) {
    if (!_loaded || props.str == "") {
        return;
    }

    int textEffect = (int)props.effect;
    const int SHADOW_EFFECT = (int)TextEffect::SHADOW_EFFECT;

    const bool boundsChanged = props.bounds != drawInfo.bounds || props.origin != drawInfo.origin;

    // If we're switching to or from shadow effect mode, we need to rebuild the vertices
    if (props.str != drawInfo.string || boundsChanged || props.alignment != drawInfo.alignment || props.verticalAlignment != drawInfo.verticalAlignment ||
            (drawInfo.params.effect != textEffect && (textEffect == SHADOW_EFFECT || drawInfo.params.effect == SHADOW_EFFECT)) ||
            (textEffect == SHADOW_EFFECT && props.scale != drawInfo.scale)) {
        drawInfo.scale = props.scale;
        drawInfo.alignment = props.alignment;
        drawInfo.verticalAlignment = props.verticalAlignment;
        buildVertices(drawInfo, props.str, props.origin, props.bounds, props.scale, textEffect == SHADOW_EFFECT, drawInfo.alignment, drawInfo.verticalAlignment);
    }

    setupGPU();

    if (!drawInfo.paramsBuffer || boundsChanged || _needsParamsUpdate || drawInfo.params.color != props.color ||
            drawInfo.params.effectColor != props.effectColor || drawInfo.params.effectThickness != props.effectThickness ||
            drawInfo.params.effect != textEffect) {
        drawInfo.params.color = props.color;
        drawInfo.params.effectColor = props.effectColor;
        drawInfo.params.effectThickness = props.effectThickness;
        drawInfo.params.effect = textEffect;

        // need the gamma corrected color here
        DrawParams gpuDrawParams;
        gpuDrawParams.bounds = glm::vec4(props.origin, props.bounds);
        gpuDrawParams.color = ColorUtils::sRGBToLinearVec4(drawInfo.params.color);
        gpuDrawParams.unitRange = _distanceRange;
        gpuDrawParams.effect = drawInfo.params.effect;
        gpuDrawParams.effectThickness = drawInfo.params.effectThickness;
        gpuDrawParams.effectColor = ColorUtils::sRGBToLinearVec3(drawInfo.params.effectColor);
        if (!drawInfo.paramsBuffer) {
            drawInfo.paramsBuffer = std::make_shared<gpu::Buffer>(sizeof(DrawParams), nullptr);
        }
        drawInfo.paramsBuffer->setSubData(0, sizeof(DrawParams), (const gpu::Byte*)&gpuDrawParams);

        _needsParamsUpdate = false;
    }

    batch.setPipeline(_pipelines[std::make_tuple(props.color.a < 1.0f, props.unlit, props.forward, props.mirror)]);
    batch.setInputFormat(_format);
    batch.setInputBuffer(0, drawInfo.verticesBuffer, 0, _format->getChannels().at(0)._stride);
    batch.setResourceTexture(render_utils::slot::texture::TextFont, _texture);
    batch.setUniformBuffer(0, drawInfo.paramsBuffer, 0, sizeof(DrawParams));
    batch.setIndexBuffer(gpu::UINT16, drawInfo.indicesBuffer, 0);
    batch.drawIndexed(gpu::TRIANGLES, drawInfo.indexCount, 0);
}
