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

#include <QColor>
#include <QPainter>
#include <QPainterPath>

class ScriptEngine;

struct CanvasImage {
    QByteArray buffer;
    int width, height;

    CanvasImage() : buffer(QByteArray()), width(0), height(0) {}
    CanvasImage(QByteArray buffer, int width, int height) : buffer(buffer), width(width), height(height) {}
};

class CanvasCommand {
public:
    enum class Variant: uint8_t {
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

    enum RenderHint: uint8_t {
        PrimitiveAntialiasing = (1 << 0),
        TextAntialiasing = (1 << 1),
        BilinearImageScaling = (1 << 2),
    };

    struct Invalid {};
    struct SetStrokeWidth { qreal width; };
    struct SetColor { glm::u8vec3 color; };
    struct SetHints { RenderHint hints; };
    struct SetBlendMode { QPainter::CompositionMode mode; };
    struct SetFont { QString family; int size; int weight; bool italic; };
    struct ClearRect { QRect rect; };
    struct FillPath { QPainterPath path; };
    struct FillRect { QRectF rect; };
    struct FillEllipse { QRectF rect; };
    struct FillText { QRectF rect; QString text; Qt::AlignmentFlag flag; };
    struct StrokePath { QPainterPath path; };
    struct StrokeRect { QRectF rect; };
    struct StrokeArc { QRectF rect; qreal startAngle, spanAngle; };
    struct StrokeEllipse { QRectF rect; };
    struct Point { QPointF point; };
    struct Line { QLineF line; };
    struct ImageCopy { QRectF src; QRectF dst; CanvasImage image; };

    CanvasCommand() noexcept: _invalid(), _tag(Variant::Invalid) {}
    CanvasCommand(Invalid cmd) noexcept: _invalid(), _tag(Variant::Invalid) {}
    CanvasCommand(SetStrokeWidth cmd) noexcept: _setStrokeWidth(cmd), _tag(Variant::SetStrokeWidth) {}
    CanvasCommand(SetColor cmd) noexcept: _setColor(cmd), _tag(Variant::SetColor) {}
    CanvasCommand(SetHints cmd) noexcept: _setHints(cmd), _tag(Variant::SetHints) {}
    CanvasCommand(SetBlendMode cmd) noexcept: _setBlendMode(cmd), _tag(Variant::SetBlendMode) {}
    CanvasCommand(SetFont cmd) noexcept: _setFont(cmd), _tag(Variant::SetFont) {}
    CanvasCommand(ClearRect cmd) noexcept: _clearRect(cmd), _tag(Variant::ClearRect) {}
    CanvasCommand(FillPath cmd) noexcept: _fillPath(cmd), _tag(Variant::FillPath) {}
    CanvasCommand(FillRect cmd) noexcept: _fillRect(cmd), _tag(Variant::FillRect) {}
    CanvasCommand(FillEllipse cmd) noexcept: _fillEllipse(cmd), _tag(Variant::FillEllipse) {}
    CanvasCommand(FillText cmd) noexcept: _fillText(cmd), _tag(Variant::FillText) {}
    CanvasCommand(StrokePath cmd) noexcept: _strokePath(cmd), _tag(Variant::StrokePath) {}
    CanvasCommand(StrokeRect cmd) noexcept: _strokeRect(cmd), _tag(Variant::StrokeRect) {}
    CanvasCommand(StrokeArc cmd) noexcept: _strokeArc(cmd), _tag(Variant::StrokeArc) {}
    CanvasCommand(StrokeEllipse cmd) noexcept: _strokeEllipse(cmd), _tag(Variant::StrokeArc) {}
    CanvasCommand(Point cmd) noexcept: _point(cmd), _tag(Variant::Point) {}
    CanvasCommand(Line cmd) noexcept: _line(cmd), _tag(Variant::Line) {}
    CanvasCommand(ImageCopy cmd) noexcept: _imageCopy(cmd), _tag(Variant::ImageCopy) {}

    ~CanvasCommand() noexcept {
        switch (_tag) {
            case Variant::Invalid: _invalid.~Invalid(); break;
            case Variant::SetStrokeWidth: _setStrokeWidth.~SetStrokeWidth(); break;
            case Variant::SetColor: _setColor.~SetColor(); break;
            case Variant::SetHints: _setHints.~SetHints(); break;
            case Variant::SetBlendMode: _setBlendMode.~SetBlendMode(); break;
            case Variant::SetFont: _setFont.~SetFont(); break;
            case Variant::ClearRect: _clearRect.~ClearRect(); break;
            case Variant::FillPath: _fillPath.~FillPath(); break;
            case Variant::FillRect: _fillRect.~FillRect(); break;
            case Variant::FillEllipse: _fillEllipse.~FillEllipse(); break;
            case Variant::FillText: _fillText.~FillText(); break;
            case Variant::StrokePath: _strokePath.~StrokePath(); break;
            case Variant::StrokeRect: _strokeRect.~StrokeRect(); break;
            case Variant::StrokeArc: _strokeArc.~StrokeArc(); break;
            case Variant::StrokeEllipse: _strokeEllipse.~StrokeEllipse(); break;
            case Variant::Point: _point.~Point(); break;
            case Variant::Line: _line.~Line(); break;
            case Variant::ImageCopy: _imageCopy.~ImageCopy(); break;
        }
    }

    CanvasCommand(const CanvasCommand& other) noexcept {
        _tag = other._tag;
        switch (other._tag) {
            case Variant::Invalid: _invalid = other._invalid; break;
            case Variant::SetStrokeWidth: _setStrokeWidth = _setStrokeWidth; break;
            case Variant::SetColor: _setColor = other._setColor; break;
            case Variant::SetHints: _setHints = other._setHints; break;
            case Variant::SetBlendMode: _setBlendMode = other._setBlendMode; break;
            case Variant::SetFont: _setFont = other._setFont; break;
            case Variant::ClearRect: _clearRect = other._clearRect; break;
            case Variant::FillPath: _fillPath = other._fillPath; break;
            case Variant::FillRect: _fillRect = other._fillRect; break;
            case Variant::FillEllipse: _fillEllipse = other._fillEllipse; break;
            case Variant::FillText: _fillText = other._fillText; break;
            case Variant::StrokePath: _strokePath = other._strokePath; break;
            case Variant::StrokeRect: _strokeRect = other._strokeRect; break;
            case Variant::StrokeArc: _strokeArc = other._strokeArc; break;
            case Variant::StrokeEllipse: _strokeEllipse = other._strokeEllipse; break;
            case Variant::Point: _point = other._point; break;
            case Variant::Line: _line = other._line; break;
            case Variant::ImageCopy: _imageCopy = other._imageCopy; break;
        }
    }

    CanvasCommand(CanvasCommand&& other) noexcept {
        _tag = other._tag;
        switch (other._tag) {
            case Variant::Invalid: _invalid = other._invalid; break;
            case Variant::SetStrokeWidth: _setStrokeWidth = _setStrokeWidth; break;
            case Variant::SetColor: _setColor = other._setColor; break;
            case Variant::SetHints: _setHints = other._setHints; break;
            case Variant::SetBlendMode: _setBlendMode = other._setBlendMode; break;
            case Variant::SetFont: _setFont = other._setFont; break;
            case Variant::ClearRect: _clearRect = other._clearRect; break;
            case Variant::FillPath: _fillPath = other._fillPath; break;
            case Variant::FillRect: _fillRect = other._fillRect; break;
            case Variant::FillEllipse: _fillEllipse = other._fillEllipse; break;
            case Variant::FillText: _fillText = other._fillText; break;
            case Variant::StrokePath: _strokePath = other._strokePath; break;
            case Variant::StrokeRect: _strokeRect = other._strokeRect; break;
            case Variant::StrokeArc: _strokeArc = other._strokeArc; break;
            case Variant::StrokeEllipse: _strokeEllipse = other._strokeEllipse; break;
            case Variant::Point: _point = other._point; break;
            case Variant::Line: _line = other._line; break;
            case Variant::ImageCopy: _imageCopy = other._imageCopy; break;
        }
    }

    CanvasCommand& operator=(const CanvasCommand& other) noexcept {
        _tag = other._tag;
        switch (other._tag) {
            case Variant::Invalid: _invalid = other._invalid; break;
            case Variant::SetStrokeWidth: _setStrokeWidth = _setStrokeWidth; break;
            case Variant::SetColor: _setColor = other._setColor; break;
            case Variant::SetHints: _setHints = other._setHints; break;
            case Variant::SetBlendMode: _setBlendMode = other._setBlendMode; break;
            case Variant::SetFont: _setFont = other._setFont; break;
            case Variant::ClearRect: _clearRect = other._clearRect; break;
            case Variant::FillPath: _fillPath = other._fillPath; break;
            case Variant::FillRect: _fillRect = other._fillRect; break;
            case Variant::FillEllipse: _fillEllipse = other._fillEllipse; break;
            case Variant::FillText: _fillText = other._fillText; break;
            case Variant::StrokePath: _strokePath = other._strokePath; break;
            case Variant::StrokeRect: _strokeRect = other._strokeRect; break;
            case Variant::StrokeArc: _strokeArc = other._strokeArc; break;
            case Variant::StrokeEllipse: _strokeEllipse = other._strokeEllipse; break;
            case Variant::Point: _point = other._point; break;
            case Variant::Line: _line = other._line; break;
            case Variant::ImageCopy: _imageCopy = other._imageCopy; break;
        }
        return *this;
    }

    void set(Invalid&& cmd) { _tag = Variant::Invalid; _invalid = cmd; }
    void set(SetStrokeWidth&& cmd) { _tag = Variant::SetStrokeWidth; _setStrokeWidth = cmd; }
    void set(SetColor&& cmd) { _tag = Variant::SetColor; _setColor = cmd; }
    void set(SetHints&& cmd) { _tag = Variant::SetHints; _setHints = cmd; }
    void set(SetBlendMode&& cmd) { _tag = Variant::SetBlendMode; _setBlendMode = cmd; }
    void set(SetFont&& cmd) { _tag = Variant::SetFont; _setFont = cmd; }
    void set(ClearRect&& cmd) { _tag = Variant::ClearRect; _clearRect = cmd; }
    void set(FillPath&& cmd) { _tag = Variant::FillPath; _fillPath = cmd; }
    void set(FillRect&& cmd) { _tag = Variant::FillRect; _fillRect = cmd; }
    void set(FillEllipse&& cmd) { _tag = Variant::FillEllipse; _fillEllipse = cmd; }
    void set(FillText&& cmd) { _tag = Variant::FillText; _fillText = cmd; }
    void set(StrokePath&& cmd) { _tag = Variant::StrokePath; _strokePath = cmd; }
    void set(StrokeRect&& cmd) { _tag = Variant::StrokeRect; _strokeRect = cmd; }
    void set(StrokeArc&& cmd) { _tag = Variant::StrokeArc; _strokeArc = cmd; }
    void set(StrokeEllipse&& cmd) { _tag = Variant::StrokeEllipse; _strokeEllipse = cmd; }
    void set(Point&& cmd) { _tag = Variant::Point; _point = cmd; }
    void set(Line&& cmd) { _tag = Variant::Line; _line = cmd; }
    void set(ImageCopy&& cmd) { _tag = Variant::ImageCopy; _imageCopy = cmd; }

    Variant kind() const { return _tag; }

    union {
        Invalid _invalid;
        SetStrokeWidth _setStrokeWidth;
        SetColor _setColor;
        SetHints _setHints;
        SetBlendMode _setBlendMode;
        SetFont _setFont;
        ClearRect _clearRect;
        FillPath _fillPath;
        FillRect _fillRect;
        FillText _fillText;
        FillEllipse _fillEllipse;
        StrokePath _strokePath;
        StrokeRect _strokeRect;
        StrokeArc _strokeArc;
        StrokeEllipse _strokeEllipse;
        Point _point;
        Line _line;
        ImageCopy _imageCopy;
    };

private:
    Variant _tag;
};

class CanvasCommandFactory : public QObject {
    Q_OBJECT

public:
    static CanvasCommand setStrokeWidth(qreal width) {
        return CanvasCommand(CanvasCommand::SetStrokeWidth { width });
    }

    static CanvasCommand setColor(glm::u8vec3 color) {
        return CanvasCommand(CanvasCommand::SetColor { color });
    }

    static CanvasCommand setHints(uint hints) {
        return CanvasCommand(CanvasCommand::SetHints { static_cast<CanvasCommand::RenderHint>(hints) });
    }
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

#endif // hifi_CanvasCommand_h

/// @}
