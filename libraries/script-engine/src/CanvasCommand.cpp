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

    ScriptValue obj = engine->newObject();

    obj.setProperty(CMD_TYPE_PROP_NAME, cmd.kind);

    switch (cmd.kind) {
        case Variant::SetStrokeWidth: {
            obj.setProperty("width", cmd._float[0]);
            return obj;
        }

        case Variant::SetColor: {
            obj.setProperty("color", u8vec3ColorToScriptValue(engine, cmd._color));
            return obj;
        }

        case Variant::SetHints: {
            obj.setProperty("hints", cmd._int[0]);
            return obj;
        }

        case Variant::SetBlendMode: {
            obj.setProperty("mode", cmd._int[0]);
            return obj;
        }

        case Variant::SetFont: {
            obj.setProperty("family", cmd._text);
            obj.setProperty("size", cmd._int[0]);
            obj.setProperty("weight", cmd._int[1]);
            obj.setProperty("italic", cmd._int[2]);
            return obj;
        }

        case Variant::ClearRect: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::FillPath: {
            obj.setProperty("path", qPainterPathToScriptValue(engine, cmd._paintPath));
            return obj;
        }

        case Variant::FillRect: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::FillEllipse: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::FillText: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            obj.setProperty("text", cmd._text);
            obj.setProperty("flag", cmd._int[0]);
            return obj;
        }

        case Variant::StrokePath: {
            obj.setProperty("path", qPainterPathToScriptValue(engine, cmd._paintPath));
            return obj;
        }

        case Variant::StrokeRect: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::StrokeArc: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            obj.setProperty("startAngle", cmd._float[0]);
            obj.setProperty("spanAngle", cmd._float[1]);
            return obj;
        }

        case Variant::StrokeEllipse: {
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::Point: {
            obj.setProperty("x", cmd._point.x());
            obj.setProperty("y", cmd._point.y());
            return obj;
        }

        case Variant::Line: {
            obj.setProperty("x1", cmd._line.x1());
            obj.setProperty("y1", cmd._line.y1());
            obj.setProperty("x2", cmd._line.x2());
            obj.setProperty("y2", cmd._line.y2());
            return obj;
        }

        case Variant::ImageCopy: {
            obj.setProperty("srcX", cmd._rect.x());
            obj.setProperty("srcY", cmd._rect.y());
            obj.setProperty("srcW", cmd._rect.width());
            obj.setProperty("srcH", cmd._rect.height());
            obj.setProperty("destX", cmd._rect2.x());
            obj.setProperty("destY", cmd._rect2.y());
            obj.setProperty("destW", cmd._rect2.width());
            obj.setProperty("destH", cmd._rect2.height());
            obj.setProperty("image", canvasImageToScriptValue(engine, cmd._image));
            return obj;
        }

        default: {
            return obj;
        }
    }
}

bool canvasCommandFromScriptValue(const ScriptValue& object, CanvasCommand& cmd) {
    using Variant = CanvasCommand::Variant;

    uint type = object.property(CMD_TYPE_PROP_NAME).toInt32();

    if (type == Variant::SetStrokeWidth) {
        cmd = CanvasCommand::setStrokeWidth(object.property("width").toNumber());
    } else if (type == Variant::SetColor) {
        glm::u8vec3 c;
        if (!u8vec3FromScriptValue(object.property("color"), c)) { return false; }

        // FIXME: we have a script RGB color type but not an RGBA one
        cmd = CanvasCommand::setColor(c);
    } else if (type == Variant::SetHints) {
        cmd = CanvasCommand::setHints(object.property("hints").toInt32());
    } else if (type == Variant::SetBlendMode) {
        cmd = CanvasCommand::setBlendMode(object.property("mode").toInt32());
    } else if (type == Variant::SetFont) {
        cmd = CanvasCommand::setFont(
            object.property("family").toString(),
            object.property("size").toInt32(),
            object.property("weight").toInt32(),
            object.property("italic").toBool()
        );
    } else if (type == Variant::ClearRect) {
        cmd = CanvasCommand::clearRect(
            object.property("x").toInt32(),
            object.property("y").toInt32(),
            object.property("w").toInt32(),
            object.property("h").toInt32()
        );
    } else if (type == Variant::FillPath) {
        cmd = CanvasCommand::fillPath(qPainterPathFromScriptValue(object.property("path")));
    } else if (type == Variant::FillRect) {
        cmd = CanvasCommand::fillRect(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::FillEllipse) {
        cmd = CanvasCommand::fillEllipse(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::FillText) {
        cmd = CanvasCommand::fillText(
            object.property("text").toString(),
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            ),
            object.property("flag").toInt32()
        );
    } else if (type == Variant::StrokePath) {
        cmd = CanvasCommand::strokePath(qPainterPathFromScriptValue(object.property("path")));
    } else if (type == Variant::StrokeRect) {
        cmd = CanvasCommand::strokeRect(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::StrokeArc) {
        cmd = CanvasCommand::strokeArc(
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
        cmd = CanvasCommand::strokeEllipse(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == Variant::Point) {
        cmd = CanvasCommand::point(
            object.property("x").toNumber(),
            object.property("y").toNumber()
        );
    } else if (type == Variant::Line) {
        cmd = CanvasCommand::line(
            object.property("x1").toNumber(),
            object.property("y1").toNumber(),
            object.property("x2").toNumber(),
            object.property("y2").toNumber()
        );
    } else if (type == Variant::ImageCopy) {
        cmd = CanvasCommand::imageCopy(
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
        cmd = CanvasCommand {};
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
    img.width = object.property(IMG_WIDTH_PROP_NAME).toInt32();
    img.height = object.property(IMG_HEIGHT_PROP_NAME).toInt32();
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

CanvasCommand CanvasCommandInterface::setStrokeWidth(qreal width) const {
    return CanvasCommand::setStrokeWidth(width);
}

CanvasCommand CanvasCommandInterface::setColor(const glm::u8vec3& color) const {
    return CanvasCommand::setColor(color);
}

CanvasCommand CanvasCommandInterface::setHints(int hints) const {
    return CanvasCommand::setHints(hints);
}

CanvasCommand CanvasCommandInterface::setBlendMode(int mode) const {
    return CanvasCommand::setBlendMode(mode);
}

CanvasCommand CanvasCommandInterface::setFont(const QString& family, int size, int weight, bool italic) const {
    return CanvasCommand::setFont(family, size, weight, italic);
}

CanvasCommand CanvasCommandInterface::clearRect(int x, int y, int w, int h) const {
    return CanvasCommand::clearRect(x, y, w, h);
}

CanvasCommand CanvasCommandInterface::fillPath(const QPainterPath& path) const {
    return CanvasCommand::fillPath(path);
}

CanvasCommand CanvasCommandInterface::fillRect(qreal x, qreal y, qreal w, qreal h) const {
    return CanvasCommand::fillRect(QRectF(x, y, w, h));
}

CanvasCommand CanvasCommandInterface::fillEllipse(qreal x, qreal y, qreal w, qreal h) const {
    return CanvasCommand::fillEllipse(QRectF(x, y, w, h));
}

CanvasCommand CanvasCommandInterface::fillText(const QString& text, qreal x, qreal y, qreal w, qreal h, int flag) const {
    return CanvasCommand::fillText(text, QRectF(x, y, w, h), flag);
}

CanvasCommand CanvasCommandInterface::strokePath(const QPainterPath& path) const {
    return CanvasCommand::strokePath(path);
}

CanvasCommand CanvasCommandInterface::strokeRect(qreal x, qreal y, qreal w, qreal h) const {
    return CanvasCommand::strokeRect(QRectF(x, y, w, h));
}

CanvasCommand CanvasCommandInterface::strokeArc(qreal x, qreal y, qreal w, qreal h, qreal startAngle, qreal spanAngle) const {
    return CanvasCommand::strokeArc(QRectF(x, y, w, h), startAngle, spanAngle);
}

CanvasCommand CanvasCommandInterface::strokeEllipse(qreal x, qreal y, qreal w, qreal h) const {
    return CanvasCommand::strokeEllipse(QRectF(x, y, w, h));
}

CanvasCommand CanvasCommandInterface::point(qreal x, qreal y) const {
    return CanvasCommand::point(x, y);
}

CanvasCommand CanvasCommandInterface::line(qreal x1, qreal y1, qreal x2, qreal y2) const {
    return CanvasCommand::line(x1, y1, x2, y2);
}

CanvasCommand CanvasCommandInterface::imageCopy(const CanvasImage& image, qreal sx, qreal sy, qreal sw, qreal sh, qreal dx, qreal dy, qreal dw, qreal dh) const {
    return CanvasCommand::imageCopy(image, QRectF(sx, sy, sw, sh), QRectF(dx, dy, dw, dh));
}
