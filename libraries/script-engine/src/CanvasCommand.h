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

    static CanvasCommand setColor(const glm::u8vec3& color) {
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
    glm::u8vec3 _color = {};
    QPainterPath _paintPath = QPainterPath();
    CanvasImage _image = {};
};

class CanvasCommandInterface : public QObject, protected Scriptable {
    Q_OBJECT

    Q_PROPERTY(int TEXT_ALIGN_LEFT READ TEXT_ALIGN_LEFT CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_RIGHT READ TEXT_ALIGN_RIGHT CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_HCENTER READ TEXT_ALIGN_HCENTER CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_JUSTIFY READ TEXT_ALIGN_JUSTIFY CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_TOP READ TEXT_ALIGN_TOP CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_BOTTOM READ TEXT_ALIGN_BOTTOM CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_VCENTER READ TEXT_ALIGN_VCENTER CONSTANT)
    Q_PROPERTY(int TEXT_ALIGN_BASELINE READ TEXT_ALIGN_BASELINE CONSTANT)

    Q_PROPERTY(int HINT_ANTIALIASING READ HINT_ANTIALIASING CONSTANT)
    Q_PROPERTY(int HINT_TEXT_ANTIALIASING READ HINT_TEXT_ANTIALIASING CONSTANT)
    Q_PROPERTY(int HINT_BILINEAR_SCALING READ HINT_BILINEAR_SCALING CONSTANT)

    Q_PROPERTY(int BLEND_SOURCEOVER READ BLEND_SOURCEOVER CONSTANT)
    Q_PROPERTY(int BLEND_DESTINATIONOVER READ BLEND_DESTINATIONOVER CONSTANT)
    Q_PROPERTY(int BLEND_CLEAR READ BLEND_CLEAR CONSTANT)
    Q_PROPERTY(int BLEND_SOURCE READ BLEND_SOURCE CONSTANT)
    Q_PROPERTY(int BLEND_DESTINATION READ BLEND_DESTINATION CONSTANT)
    Q_PROPERTY(int BLEND_SOURCEIN READ BLEND_SOURCEIN CONSTANT)
    Q_PROPERTY(int BLEND_DESTINATIONIN READ BLEND_DESTINATIONIN CONSTANT)
    Q_PROPERTY(int BLEND_SOURCEOUT READ BLEND_SOURCEOUT CONSTANT)
    Q_PROPERTY(int BLEND_DESTINATIONOUT READ BLEND_DESTINATIONOUT CONSTANT)
    Q_PROPERTY(int BLEND_SOURCEATOP READ BLEND_SOURCEATOP CONSTANT)
    Q_PROPERTY(int BLEND_DESTINATIONATOP READ BLEND_DESTINATIONATOP CONSTANT)
    Q_PROPERTY(int BLEND_XOR READ BLEND_XOR CONSTANT)
    Q_PROPERTY(int BLEND_PLUS READ BLEND_PLUS CONSTANT)
    Q_PROPERTY(int BLEND_MULTIPLY READ BLEND_MULTIPLY CONSTANT)
    Q_PROPERTY(int BLEND_SCREEN READ BLEND_SCREEN CONSTANT)
    Q_PROPERTY(int BLEND_OVERLAY READ BLEND_OVERLAY CONSTANT)
    Q_PROPERTY(int BLEND_DARKEN READ BLEND_DARKEN CONSTANT)
    Q_PROPERTY(int BLEND_LIGHTEN READ BLEND_LIGHTEN CONSTANT)
    Q_PROPERTY(int BLEND_COLORDODGE READ BLEND_COLORDODGE CONSTANT)
    Q_PROPERTY(int BLEND_COLORBURN READ BLEND_COLORBURN CONSTANT)
    Q_PROPERTY(int BLEND_HARDLIGHT READ BLEND_HARDLIGHT CONSTANT)
    Q_PROPERTY(int BLEND_SOFTLIGHT READ BLEND_SOFTLIGHT CONSTANT)
    Q_PROPERTY(int BLEND_DIFFERENCE READ BLEND_DIFFERENCE CONSTANT)
    Q_PROPERTY(int BLEND_EXCLUSION READ BLEND_EXCLUSION CONSTANT)

public slots:
    CanvasCommand setStrokeWidth(qreal width) const;
    CanvasCommand setColor(const glm::u8vec3& color) const;
    CanvasCommand setHints(int hints) const;
    CanvasCommand setBlendMode(int mode) const;
    CanvasCommand setFont(const QString& family, int size = 12, int weight = QFont::Normal, bool italic = false) const;
    CanvasCommand clearRect(int x, int y, int w, int h) const;
    CanvasCommand fillPath(const QPainterPath& path) const;
    CanvasCommand fillRect(qreal x, qreal y, qreal w, qreal h) const;
    CanvasCommand fillEllipse(qreal x, qreal y, qreal w, qreal h) const;
    CanvasCommand fillText(const QString& text, qreal x, qreal y, qreal w = 0, qreal h = 0, int flag = 0) const;
    CanvasCommand strokePath(const QPainterPath& path) const;
    CanvasCommand strokeRect(qreal x, qreal y, qreal w, qreal h) const;
    CanvasCommand strokeArc(qreal x, qreal y, qreal w, qreal h, qreal startAngle, qreal spanAngle) const;
    CanvasCommand strokeEllipse(qreal x, qreal y, qreal w, qreal h) const;
    CanvasCommand point(qreal x, qreal y) const;
    CanvasCommand line(qreal x1, qreal y1, qreal x2, qreal y2) const;
    CanvasCommand imageCopy(const CanvasImage& image, qreal sx, qreal sy, qreal sw, qreal sh, qreal dx, qreal dy, qreal dw, qreal dh) const;

private:
    int TEXT_ALIGN_LEFT() const { return Qt::AlignLeft; }
    int TEXT_ALIGN_RIGHT() const { return Qt::AlignRight; }
    int TEXT_ALIGN_HCENTER() const { return Qt::AlignHCenter; }
    int TEXT_ALIGN_JUSTIFY() const { return Qt::AlignJustify; }
    int TEXT_ALIGN_TOP() const { return Qt::AlignTop; }
    int TEXT_ALIGN_BOTTOM() const { return Qt::AlignBottom; }
    int TEXT_ALIGN_VCENTER() const { return Qt::AlignVCenter; }
    int TEXT_ALIGN_BASELINE() const { return Qt::AlignBaseline; }

    int HINT_ANTIALIASING() const { return CanvasCommand::RenderHint::PrimitiveAntialiasing; }
    int HINT_TEXT_ANTIALIASING() const { return CanvasCommand::RenderHint::PrimitiveAntialiasing; }
    int HINT_BILINEAR_SCALING() const { return CanvasCommand::RenderHint::BilinearImageScaling; }

    int BLEND_SOURCEOVER() const { return QPainter::CompositionMode_SourceOver; }
    int BLEND_DESTINATIONOVER() const { return QPainter::CompositionMode_DestinationOver; }
    int BLEND_CLEAR() const { return QPainter::CompositionMode_Clear; }
    int BLEND_SOURCE() const { return QPainter::CompositionMode_Source; }
    int BLEND_DESTINATION() const { return QPainter::CompositionMode_Destination; }
    int BLEND_SOURCEIN() const { return QPainter::CompositionMode_SourceIn; }
    int BLEND_DESTINATIONIN() const { return QPainter::CompositionMode_DestinationIn; }
    int BLEND_SOURCEOUT() const { return QPainter::CompositionMode_SourceOut; }
    int BLEND_DESTINATIONOUT() const { return QPainter::CompositionMode_DestinationOut; }
    int BLEND_SOURCEATOP() const { return QPainter::CompositionMode_SourceAtop; }
    int BLEND_DESTINATIONATOP() const { return QPainter::CompositionMode_DestinationAtop; }
    int BLEND_XOR() const { return QPainter::CompositionMode_Xor; }
    int BLEND_PLUS() const { return QPainter::CompositionMode_Plus; }
    int BLEND_MULTIPLY() const { return QPainter::CompositionMode_Multiply; }
    int BLEND_SCREEN() const { return QPainter::CompositionMode_Screen; }
    int BLEND_OVERLAY() const { return QPainter::CompositionMode_Overlay; }
    int BLEND_DARKEN() const { return QPainter::CompositionMode_Darken; }
    int BLEND_LIGHTEN() const { return QPainter::CompositionMode_Lighten; }
    int BLEND_COLORDODGE() const { return QPainter::CompositionMode_ColorDodge; }
    int BLEND_COLORBURN() const { return QPainter::CompositionMode_ColorBurn; }
    int BLEND_HARDLIGHT() const { return QPainter::CompositionMode_HardLight; }
    int BLEND_SOFTLIGHT() const { return QPainter::CompositionMode_SoftLight; }
    int BLEND_DIFFERENCE() const { return QPainter::CompositionMode_Difference; }
    int BLEND_EXCLUSION() const { return QPainter::CompositionMode_Exclusion; }
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
