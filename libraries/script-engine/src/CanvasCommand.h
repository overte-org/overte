//
//  CanvasCommand.h
//  libraries/script-engine/src
//
//  Created by Ada <ada@thingvellir.net> on 2025-02-27
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_CanvasCommand_h
#define hifi_CanvasCommand_h

#include "ScriptValue.h"
#include "ScriptValueUtils.h"
#include "Scriptable.h"

#include <QPainter>
#include <QPainterPath>

class ScriptEngine;

struct CanvasImage {
    QByteArray buffer;
    int width, height;

    CanvasImage() : buffer(QByteArray()), width(0), height(0) {}
    CanvasImage(QByteArray buffer, int width, int height) : buffer(buffer), width(width), height(height) {}
};

struct CanvasPathElement {
    int type;
    qreal x, y, c1x, c1y, c2x, c2y;

    CanvasPathElement(int type, qreal x, qreal y) : type(type), x(x), y(y), c1x(0), c1y(0), c2x(0), c2y(0) {}
    CanvasPathElement(int type, qreal x, qreal y, qreal c1x, qreal c1y, qreal c2x, qreal c2y) : type(type), x(x), y(y), c1x(c1x), c1y(c1y), c2x(c2x), c2y(c2y) {}
    CanvasPathElement() : type(0), x(0), y(0), c1x(0), c1y(0), c2x(0), c2y(0) {}
};

struct CanvasCommand {
    enum Variant {
        Invalid,
        SetStrokeWidth,
        SetColor,
        SetHints,
        SetBlendMode,
        SetFont,
        ClearRect,
        FillPath,
        FillRect,
        FillEllipse,
        FillText,
        StrokePath,
        StrokeRect,
        StrokeArc,
        StrokeEllipse,
        Point,
        Line,
        ImageCopy,
    };

    enum RenderHint {
        PrimitiveAntialiasing = (1 << 0),
        TextAntialiasing = (1 << 1),
        BilinearImageScaling = (1 << 2),
    };

    static CanvasCommand none() {
        return CanvasCommand {};
    }

    static CanvasCommand setStrokeWidth(qreal width) {
        return CanvasCommand { .kind = Variant::SetStrokeWidth, ._float = {width} };
    }

    static CanvasCommand setColor(const QColor& color) {
        return CanvasCommand { .kind = Variant::SetColor, ._color = color };
    }

    static CanvasCommand setHints(int hints) {
        return CanvasCommand { .kind = Variant::SetHints, ._int = {hints} };
    }

    static CanvasCommand setBlendMode(int mode) {
        return CanvasCommand { .kind = Variant::SetBlendMode, ._int = {mode} };
    }

    static CanvasCommand setFont(const QString& family, int size, int weight, bool italic) {
        return CanvasCommand { .kind = Variant::SetFont, ._text = family, ._int = {size, weight, italic} };
    }

    static CanvasCommand clearRect(int x, int y, int w, int h) {
        return CanvasCommand { .kind = Variant::ClearRect, ._int = {x, y, w, h} };
    }

    static CanvasCommand fillPath(const QPainterPath& path) {
        return CanvasCommand { .kind = Variant::FillPath, ._paintPath = path };
    }

    static CanvasCommand fillRect(const QRectF& rect) {
        return CanvasCommand { .kind = Variant::FillRect, ._rect = rect };
    }

    static CanvasCommand fillEllipse(const QRectF& rect) {
        return CanvasCommand { .kind = Variant::FillEllipse, ._rect = rect };
    }

    static CanvasCommand fillText(const QString& text, const QRectF& rect, int flag) {
        return CanvasCommand { .kind = Variant::FillText, ._rect = rect, ._text = text, ._int = {flag} };
    }

    static CanvasCommand strokePath(const QPainterPath& path) {
        return CanvasCommand { .kind = Variant::StrokePath, ._paintPath = path };
    }

    static CanvasCommand strokeRect(const QRectF& rect) {
        return CanvasCommand { .kind = Variant::StrokeRect, ._rect = rect };
    }

    static CanvasCommand strokeArc(const QRectF& rect, qreal startAngle, qreal spanAngle) {
        return CanvasCommand { .kind = Variant::StrokeArc, ._rect = rect, ._float = {startAngle, spanAngle} };
    }

    static CanvasCommand strokeEllipse(const QRectF& rect) {
        return CanvasCommand { .kind = Variant::StrokeEllipse, ._rect = rect };
    }

    static CanvasCommand point(qreal x, qreal y) {
        return CanvasCommand { .kind = Variant::Point, ._point = QPointF(x, y) };
    }

    static CanvasCommand line(qreal x1, qreal y1, qreal x2, qreal y2) {
        return CanvasCommand { .kind = Variant::Line, ._line = QLineF(x1, y1, x2, y2) };
    }

    static CanvasCommand imageCopy(const CanvasImage& image, const QRectF& src, const QRectF& dst) {
        return CanvasCommand { .kind = Variant::ImageCopy, ._rect = src, ._rect2 = dst, ._image = image };
    }

    Variant kind = Variant::Invalid;

    QRectF _rect = QRectF();
    QRectF _rect2 = QRectF();
    QString _text = QString();
    QPointF _point = QPointF();
    QLineF _line = QLineF();
    qreal _float[4] = {};
    int _int[4] = {};
    QColor _color = {};
    QPainterPath _paintPath = QPainterPath();
    CanvasImage _image = {};
};

void registerCanvasMetaTypes(ScriptEngine *engine);

Q_DECLARE_METATYPE(CanvasCommand)
ScriptValue canvasCommandToScriptValue(ScriptEngine* engine, const CanvasCommand& cmd);
bool canvasCommandFromScriptValue(const ScriptValue& object, CanvasCommand& cmd);
CanvasCommand canvasCommandFromScriptValue(const ScriptValue& object);

Q_DECLARE_METATYPE(QPainterPath)
ScriptValue qPainterPathToScriptValue(ScriptEngine* engine, const QPainterPath& path);
bool qPainterPathFromScriptValue(const ScriptValue& object, QPainterPath& path);
QPainterPath qPainterPathFromScriptValue(const ScriptValue& object);

Q_DECLARE_METATYPE(QVector<CanvasCommand>)
ScriptValue qVectorCanvasCommandToScriptValue(ScriptEngine* engine, const QVector<CanvasCommand>& list);
bool qVectorCanvasCommandFromScriptValue(const ScriptValue& object, QVector<CanvasCommand>& list);
QVector<CanvasCommand> qVectorCanvasCommandFromScriptValue(const ScriptValue& object);

Q_DECLARE_METATYPE(CanvasImage)
ScriptValue canvasImageToScriptValue(ScriptEngine* engine, const CanvasImage& img);
bool canvasImageFromScriptValue(const ScriptValue& object, CanvasImage& img);
CanvasImage canvasImageFromScriptValue(const ScriptValue& object);

Q_DECLARE_METATYPE(CanvasPathElement)
ScriptValue canvasPathElementToScriptValue(ScriptEngine* engine, const CanvasPathElement& elem);
bool canvasPathElementFromScriptValue(const ScriptValue& object, CanvasPathElement& elem);
CanvasPathElement canvasPathElementFromScriptValue(const ScriptValue& object);

#endif // hifi_CanvasCommand_h

/// @}
