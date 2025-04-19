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
    scriptRegisterMetaType<QPainterPath, qPainterPathToScriptValue, qPainterPathFromScriptValue>(scriptEngine, "QPainterPath");
    scriptRegisterMetaType<QVector<CanvasCommand>, qVectorCanvasCommandToScriptValue, qVectorCanvasCommandFromScriptValue>(scriptEngine);
    scriptRegisterMetaType<CanvasPathElement, canvasPathElementToScriptValue, canvasPathElementFromScriptValue>(scriptEngine, "CanvasPathElement");
}));

const QString CMD_TYPE_PROP_NAME = "type";

const QString IMG_WIDTH_PROP_NAME = "width";
const QString IMG_HEIGHT_PROP_NAME = "height";
const QString IMG_BUFFER_PROP_NAME = "buffer";

ScriptValue canvasCommandToScriptValue(ScriptEngine* engine, const CanvasCommand& cmd) {
    using Variant = CanvasCommand::Variant;

    ScriptValue obj = engine->newObject();

    switch (cmd.kind) {
        case Variant::SetStrokeWidth: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "strokeWidth");
            obj.setProperty("width", cmd._float[0]);
            return obj;
        }

        case Variant::SetColor: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "color");

            auto color = engine->newArray(4);
            color.setProperty(0, cmd._color.red());
            color.setProperty(1, cmd._color.green());
            color.setProperty(2, cmd._color.blue());
            color.setProperty(3, cmd._color.alpha());
            obj.setProperty("color", color);
            return obj;
        }

        case Variant::SetHints: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "hints");
            obj.setProperty("hints", cmd._int[0]);
            return obj;
        }

        case Variant::SetBlendMode: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "blendMode");
            obj.setProperty("mode", cmd._int[0]);
            return obj;
        }

        case Variant::SetFont: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "font");
            obj.setProperty("family", cmd._text);
            obj.setProperty("size", cmd._int[0]);
            obj.setProperty("weight", cmd._int[1]);
            obj.setProperty("italic", cmd._int[2]);
            return obj;
        }

        case Variant::ClearRect: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "clearRect");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::FillPath: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "fillPath");
            obj.setProperty("path", qPainterPathToScriptValue(engine, cmd._paintPath));
            return obj;
        }

        case Variant::FillRect: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "fillRect");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::FillEllipse: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "fillEllipse");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::FillText: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "fillText");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            obj.setProperty("text", cmd._text);
            obj.setProperty("flag", cmd._int[0]);
            return obj;
        }

        case Variant::StrokePath: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "strokePath");
            obj.setProperty("path", qPainterPathToScriptValue(engine, cmd._paintPath));
            return obj;
        }

        case Variant::StrokeRect: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "strokeRect");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::StrokeArc: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "strokeArc");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            obj.setProperty("startAngle", cmd._float[0]);
            obj.setProperty("spanAngle", cmd._float[1]);
            return obj;
        }

        case Variant::StrokeEllipse: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "strokeEllipse");
            obj.setProperty("x", cmd._rect.x());
            obj.setProperty("y", cmd._rect.y());
            obj.setProperty("w", cmd._rect.width());
            obj.setProperty("h", cmd._rect.height());
            return obj;
        }

        case Variant::Point: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "point");
            obj.setProperty("x", cmd._point.x());
            obj.setProperty("y", cmd._point.y());
            return obj;
        }

        case Variant::Line: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "line");
            obj.setProperty("x1", cmd._line.x1());
            obj.setProperty("y1", cmd._line.y1());
            obj.setProperty("x2", cmd._line.x2());
            obj.setProperty("y2", cmd._line.y2());
            return obj;
        }

        case Variant::ImageCopy: {
            obj.setProperty(CMD_TYPE_PROP_NAME, "imageCopy");
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

        case Variant::Invalid: break;
    }

    return obj;
}

bool canvasCommandFromScriptValue(const ScriptValue& object, CanvasCommand& cmd) {
    QString type = object.property(CMD_TYPE_PROP_NAME).toString();

    if (type == "strokeWidth") {
        cmd = CanvasCommand::setStrokeWidth(object.property("width").toNumber());
    } else if (type == "color") {
        QColor c;
        if (!qColorFromScriptValue(object.property("color"), c)) { return false; }

        cmd = CanvasCommand::setColor(c);
    } else if (type == "hints") {
        cmd = CanvasCommand::setHints(object.property("hints").toInt32());
    } else if (type == "blendMode") {
        cmd = CanvasCommand::setBlendMode(object.property("mode").toInt32());
    } else if (type == "font") {
        cmd = CanvasCommand::setFont(
            object.property("family").toString(),
            object.property("size").toInt32(),
            object.property("weight").toInt32(),
            object.property("italic").toBool()
        );
    } else if (type == "clearRect") {
        cmd = CanvasCommand::clearRect(
            object.property("x").toInt32(),
            object.property("y").toInt32(),
            object.property("w").toInt32(),
            object.property("h").toInt32()
        );
    } else if (type == "fillPath") {
        cmd = CanvasCommand::fillPath(qPainterPathFromScriptValue(object.property("path")));
    } else if (type == "fillRect") {
        cmd = CanvasCommand::fillRect(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == "fillEllipse") {
        cmd = CanvasCommand::fillEllipse(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == "fillText") {
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
    } else if (type == "strokePath") {
        cmd = CanvasCommand::strokePath(qPainterPathFromScriptValue(object.property("path")));
    } else if (type == "strokeRect") {
        cmd = CanvasCommand::strokeRect(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == "strokeArc") {
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
    } else if (type == "strokeEllipse") {
        cmd = CanvasCommand::strokeEllipse(QRectF(
            object.property("x").toNumber(),
            object.property("y").toNumber(),
            object.property("w").toNumber(),
            object.property("h").toNumber()
        ));
    } else if (type == "point") {
        cmd = CanvasCommand::point(
            object.property("x").toNumber(),
            object.property("y").toNumber()
        );
    } else if (type == "line") {
        cmd = CanvasCommand::line(
            object.property("x1").toNumber(),
            object.property("y1").toNumber(),
            object.property("x2").toNumber(),
            object.property("y2").toNumber()
        );
    } else if (type == "imageCopy") {
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

ScriptValue canvasPathElementToScriptValue(ScriptEngine* engine, const CanvasPathElement& elem) {
    auto obj = engine->newObject();
    obj.setProperty("type", elem.type);
    obj.setProperty("x", elem.x);
    obj.setProperty("y", elem.y);

    if (elem.type == QPainterPath::CurveToElement) {
        obj.setProperty("c1x", elem.c1x);
        obj.setProperty("c1y", elem.c1y);
        obj.setProperty("c2x", elem.c2x);
        obj.setProperty("c2y", elem.c2y);
    }

    return obj;
}

bool canvasPathElementFromScriptValue(const ScriptValue& obj, CanvasPathElement& elem) {
    int type = obj.property("type").toInt32();

    elem.type = type;
    elem.x = obj.property("x").toNumber();
    elem.y = obj.property("y").toNumber();

    if (type == QPainterPath::CurveToElement) {
        elem.c1x = obj.property("c1x").toNumber();
        elem.c1y = obj.property("c1y").toNumber();
        elem.c2x = obj.property("c2x").toNumber();
        elem.c2y = obj.property("c2y").toNumber();
    }

    return true;
}

CanvasPathElement canvasPathElementFromScriptValue(const ScriptValue& object) {
    CanvasPathElement e;
    canvasPathElementFromScriptValue(object, e);
    return e;
}
