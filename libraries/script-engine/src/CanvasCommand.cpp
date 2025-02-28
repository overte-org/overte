//
//  CanvasCommand.cpp
//  libraries/script-engine/src
//
//  Created by Ada <ada@thingvellir.net> on 2025-02-27
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "CanvasCommand.h"

#include <QtCore/QVariant>

#include "ScriptEngine.h"
#include "ScriptEngineCast.h"
#include "ScriptValue.h"
#include "ScriptManager.h"
#include "ScriptValueUtils.h"
#include "v8/FastScriptValueUtils.h"

STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptRegisterMetaType<CanvasCommand, canvasCommandToScriptValue, canvasCommandFromScriptValue>(scriptEngine, "CanvasCommand");
    scriptRegisterMetaType<CanvasImage, canvasImageToScriptValue, canvasImageFromScriptValue>(scriptEngine, "CanvasImage");
    scriptRegisterMetaType<QPainterPath, qPainterPathToScriptValue, qPainterPathFromScriptValue>(scriptEngine, "CanvasPath");
    scriptRegisterMetaType<QVector<CanvasCommand>, qVectorCanvasCommandToScriptValue, qVectorCanvasCommandFromScriptValue>(scriptEngine);
}));

const QString CMD_TYPE_PROP_NAME = "type";

const QString IMG_WIDTH_PROP_NAME = "width";
const QString IMG_HEIGHT_PROP_NAME = "height";
const QString IMG_BUFFER_PROP_NAME = "buffer";

ScriptValue canvasCommandToScriptValue(ScriptEngine* engine, const CanvasCommand& cmd) {
    using Variant = CanvasCommand::Variant;
    using namespace canvas_cmd;

    ScriptValue obj = engine->newObject();

    obj.setProperty(CMD_TYPE_PROP_NAME, cmd.kind());

    switch (cmd.kind()) {
        case Variant::SetStrokeWidth: {
            auto props = dynamic_cast<const SetStrokeWidth&>(cmd);
            obj.setProperty("width", props.width);
            return obj;
        }

        case Variant::SetColor: {
            auto props = dynamic_cast<const SetColor&>(cmd);
            obj.setProperty("color", u8vec3ColorToScriptValue(engine, props.color));
            return obj;
        }

        case Variant::SetHints: {
            auto props = dynamic_cast<const SetHints&>(cmd);
            obj.setProperty("hints", props.hints);
            return obj;
        }

        case Variant::SetBlendMode: {
            auto props = dynamic_cast<const SetBlendMode&>(cmd);
            obj.setProperty("mode", props.mode);
            return obj;
        }

        case Variant::SetFont: {
            auto props = dynamic_cast<const SetFont&>(cmd);
            obj.setProperty("family", props.family);
            obj.setProperty("size", props.size);
            obj.setProperty("weight", props.weight);
            obj.setProperty("italic", props.italic);
            return obj;
        }

        case Variant::ClearRect: {
            auto props = dynamic_cast<const ClearRect&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::FillPath: {
            auto props = dynamic_cast<const FillPath&>(cmd);
            obj.setProperty("path", qPainterPathToScriptValue(engine, props.path));
            return obj;
        }

        case Variant::FillRect: {
            auto props = dynamic_cast<const FillRect&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::FillEllipse: {
            auto props = dynamic_cast<const FillEllipse&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::FillText: {
            auto props = dynamic_cast<const FillText&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            obj.setProperty("text", props.text);
            obj.setProperty("flag", static_cast<uint>(props.flag));
            return obj;
        }

        case Variant::StrokePath: {
            auto props = dynamic_cast<const StrokePath&>(cmd);
            obj.setProperty("path", qPainterPathToScriptValue(engine, props.path));
            return obj;
        }

        case Variant::StrokeRect: {
            auto props = dynamic_cast<const StrokeRect&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::StrokeArc: {
            auto props = dynamic_cast<const StrokeArc&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            obj.setProperty("startAngle", props.startAngle);
            obj.setProperty("spanAngle", props.spanAngle);
            return obj;
        }

        case Variant::StrokeEllipse: {
            auto props = dynamic_cast<const StrokeEllipse&>(cmd);
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::Point: {
            auto props = dynamic_cast<const Point&>(cmd);
            obj.setProperty("x", props.point.x());
            obj.setProperty("y", props.point.y());
            return obj;
        }

        case Variant::Line: {
            auto props = dynamic_cast<const Line&>(cmd);
            obj.setProperty("x1", props.line.x1());
            obj.setProperty("y1", props.line.y1());
            obj.setProperty("x2", props.line.x2());
            obj.setProperty("y2", props.line.y2());
            return obj;
        }

        case Variant::ImageCopy: {
            auto props = dynamic_cast<const ImageCopy&>(cmd);
            obj.setProperty("srcX", props.src.x());
            obj.setProperty("srcY", props.src.y());
            obj.setProperty("srcW", props.src.width());
            obj.setProperty("srcH", props.src.height());
            obj.setProperty("destX", props.dst.x());
            obj.setProperty("destY", props.dst.y());
            obj.setProperty("destW", props.dst.width());
            obj.setProperty("destH", props.dst.height());
            obj.setProperty("image", canvasImageToScriptValue(engine, props.image));
            return obj;
        }

        default: {
            return obj;
        }
    }
}

bool canvasCommandFromScriptValue(const ScriptValue& object, CanvasCommand& cmd) {
    using Variant = CanvasCommand::Variant;
    using namespace canvas_cmd;

    uint type = object.property(CMD_TYPE_PROP_NAME).toInt32();

    if (type == Variant::SetStrokeWidth) {
        cmd = SetStrokeWidth { object.property("width").toNumber() };
    } else if (type == Variant::SetColor) {
        glm::u8vec3 c;
        if (!u8vec3FromScriptValue(object.property("color"), c)) { return false; }

        // FIXME: we have a script RGB color type but not an RGBA one
        cmd = SetColor(c);
    } else if (type == Variant::SetHints) {
        cmd = SetHints(object.property("hints").toUInt32());
    } else if (type == Variant::SetBlendMode) {
        cmd = SetBlendMode(object.property("mode").toUInt32());
    } else if (type == Variant::SetFont) {
        cmd = SetFont(
            object.property("family").toString(),
            object.property("size").toInt32(),
            object.property("weight").toInt32(),
            object.property("italic").toBool()
        );
    } else if (type == Variant::ClearRect) {
        cmd = ClearRect(QRect(
            object.property("x").toInt32(),
            object.property("y").toInt32(),
            object.property("w").toInt32(),
            object.property("h").toInt32()
        ));
    } else if (type == Variant::FillPath) {
        cmd = FillPath(qPainterPathFromScriptValue(object.property("path")));
    } else if (type == Variant::FillRect) {
        cmd = FillRect(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::FillEllipse) {
        cmd = FillEllipse(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::FillText) {
        cmd = FillText(
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            ),
            object.property("text").toString(),
            object.property("flag").toInt32()
        );
    } else if (type == Variant::StrokePath) {
        cmd = StrokePath(qPainterPathFromScriptValue(object.property("path")));
    } else if (type == Variant::StrokeRect) {
        cmd = StrokeRect(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::StrokeArc) {
        cmd = StrokeArc(
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            ),
            object.property("startAngle").toNumber(),
            object.property("spanAngle").toNumber()
        );
    } else if (type == Variant::StrokeEllipse) {
        cmd = StrokeEllipse(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::Point) {
        cmd = Point(
            object.property("x").toNumber(),
            object.property("y").toNumber()
        );
    } else if (type == Variant::Line) {
        cmd = Line(
            object.property("x1").toNumber(),
            object.property("y1").toNumber(),
            object.property("x2").toNumber(),
            object.property("y2").toNumber()
        );
    } else if (type == Variant::ImageCopy) {
        cmd = ImageCopy(
            canvasImageFromScriptValue(object.property("image")),
            QRectF(
                object.property("srcX").toNumber(),
                object.property("srcY").toNumber(),
                object.property("srcW").toNumber(),
                object.property("srcH").toNumber()
            ),
            QRectF(
                object.property("destX").toNumber(),
                object.property("destY").toNumber(),
                object.property("destW").toNumber(),
                object.property("destH").toNumber()
            )
        );
    } else {
        cmd = Invalid();
    }

    return true;
}

CanvasCommand canvasCommandFromScriptValue(const ScriptValue& object) {
    CanvasCommand cmd;
    canvasCommandFromScriptValue(object, cmd);
    return cmd;
}

QVector<CanvasCommand> qVectorCanvasCommandFromScriptValue(const ScriptValue& object) {
    QVector<CanvasCommand> list;
    qVectorCanvasCommandFromScriptValue(object, list);
    return list;
}

bool qVectorCanvasCommandFromScriptValue(const ScriptValue& array, QVector<CanvasCommand>& list) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        list << canvasCommandFromScriptValue(array.property(i));
    }

    return true;
}

ScriptValue qVectorCanvasCommandToScriptValue(ScriptEngine* engine, const QVector<CanvasCommand>& list) {
    auto array = engine->newArray(list.length());

    for (int i = 0; i < list.length(); i++) {
        array.setProperty(i, canvasCommandToScriptValue(engine, list.at(i)));
    }

    return array;
}

ScriptValue canvasImageToScriptValue(ScriptEngine* engine, const CanvasImage& img) {
    ScriptValue obj = engine->newObject();
    obj.setProperty(IMG_WIDTH_PROP_NAME, img.width);
    obj.setProperty(IMG_HEIGHT_PROP_NAME, img.height);
    obj.setProperty(IMG_BUFFER_PROP_NAME, qBytearrayToScriptValue(engine, img.buffer));
    return obj;
}

bool canvasImageFromScriptValue(const ScriptValue& object, CanvasImage& img) {
    img.width = object.property(IMG_WIDTH_PROP_NAME).toUInt32();
    img.height = object.property(IMG_HEIGHT_PROP_NAME).toUInt32();
    return qBytearrayFromScriptValue(object.property(IMG_BUFFER_PROP_NAME), img.buffer);
}

CanvasImage canvasImageFromScriptValue(const ScriptValue& object) {
    CanvasImage img = {};
    canvasImageFromScriptValue(object, img);
    return img;
}

ScriptValue qPainterPathToScriptValue(ScriptEngine* engine, const QPainterPath& path) {
    ScriptValue array = engine->newArray(path.elementCount());

    for (int i = 0; i < path.elementCount(); i++) {
        ScriptValue obj = engine->newObject();
        auto elem = path.elementAt(i);

        // curves have another two points
        if (elem.type == QPainterPath::CurveToElement) {
            obj.setProperty("type", static_cast<uint>(elem.type));
            obj.setProperty("c1x", static_cast<float>(elem.x));
            obj.setProperty("c1y", static_cast<float>(elem.y));
            obj.setProperty("c2x", static_cast<float>(path.elementAt(i + 1).x));
            obj.setProperty("c2y", static_cast<float>(path.elementAt(i + 1).y));
            obj.setProperty("x", static_cast<float>(path.elementAt(i + 2).x));
            obj.setProperty("y", static_cast<float>(path.elementAt(i + 2).y));

            i += 2;
        } else {
            obj.setProperty("type", static_cast<uint>(elem.type));
            obj.setProperty("x", static_cast<float>(elem.x));
            obj.setProperty("y", static_cast<float>(elem.y));
        }
    }

    return array;
}

bool qPainterPathFromScriptValue(const ScriptValue& array, QPainterPath& path) {
    int length = array.property("length").toInteger();
    path.reserve(length);

    for (int i = 0; i < length; i++) {
        ScriptValue obj = array.property(i);
        uint type = obj.property("type").toUInt32();
        auto x = obj.property("x").toNumber();
        auto y = obj.property("y").toNumber();

        if (type == QPainterPath::CurveToElement) {
            auto c1x = obj.property("c1x").toNumber();
            auto c1y = obj.property("c1y").toNumber();
            auto c2x = obj.property("c2x").toNumber();
            auto c2y = obj.property("c2y").toNumber();

            path.cubicTo(c1x, c1y, c2x, c2y, x, y);
        } else if (type == QPainterPath::LineToElement) {
            path.lineTo(x, y);
        } else {
            path.moveTo(x, y);
        }
    }

    return true;
}

QPainterPath qPainterPathFromScriptValue(const ScriptValue& object) {
    QPainterPath p;
    qPainterPathFromScriptValue(object, p);
    return p;
}

using namespace canvas_cmd;

CanvasCommand CanvasCommandInterface::setStrokeWidth(qreal width) const {
    return SetStrokeWidth(width);
}

CanvasCommand CanvasCommandInterface::setColor(const glm::u8vec3& color) const {
    return SetColor(color);
}

CanvasCommand CanvasCommandInterface::setHints(int hints) const {
    return SetHints(hints);
}

CanvasCommand CanvasCommandInterface::setBlendMode(int mode) const {
    return SetBlendMode(mode);
}

CanvasCommand CanvasCommandInterface::setFont(const QString& family, int size, int weight, bool italic) const {
    return SetFont(family, size, weight, italic);
}

CanvasCommand CanvasCommandInterface::clearRect(const QRect& rect) const {
    return ClearRect(rect);
}

CanvasCommand CanvasCommandInterface::fillPath(const QPainterPath& path) const {
    return FillPath(path);
}

CanvasCommand CanvasCommandInterface::fillRect(const QRectF& rect) const {
    return FillRect(rect);
}

CanvasCommand CanvasCommandInterface::fillEllipse(const QRectF& rect) const {
    return FillEllipse(rect);
}

CanvasCommand CanvasCommandInterface::fillText(const QString& text, const QRectF& rect, int flag) const {
    return FillText(rect, text, flag);
}

CanvasCommand CanvasCommandInterface::strokePath(const QPainterPath& path) const {
    return StrokePath(path);
}

CanvasCommand CanvasCommandInterface::strokeRect(const QRectF& rect) const {
    return StrokeRect(rect);
}

CanvasCommand CanvasCommandInterface::strokeArc(const QRectF& rect, qreal startAngle, qreal spanAngle) const {
    return StrokeArc(rect, startAngle, spanAngle);
}

CanvasCommand CanvasCommandInterface::strokeEllipse(const QRectF& rect) const {
    return StrokeEllipse(rect);
}

CanvasCommand CanvasCommandInterface::point(qreal x, qreal y) const {
    return Point(x, y);
}

CanvasCommand CanvasCommandInterface::line(qreal x1, qreal y1, qreal x2, qreal y2) const {
    return Line(x1, y1, x2, y2);
}

CanvasCommand CanvasCommandInterface::imageCopy(const CanvasImage& image, const QRectF& src, const QRectF& dest) const {
    return ImageCopy(image, src, dest);
}
