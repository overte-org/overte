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

    obj.setProperty(CMD_TYPE_PROP_NAME, (uint)cmd.kind());

    switch (cmd.kind()) {
        case Variant::SetStrokeWidth: {
            auto props = cmd._setStrokeWidth;
            obj.setProperty("width", props.width);
            return obj;
        }

        case Variant::SetColor: {
            auto props = cmd._setColor;
            obj.setProperty("color", u8vec3ColorToScriptValue(engine, glm::u8vec3(props.color.red(), props.color.green(), props.color.blue())));
            return obj;
        }

        case Variant::SetHints: {
            auto props = cmd._setHints;
            obj.setProperty("hints", props.hints);
            return obj;
        }

        case Variant::SetBlendMode: {
            auto props = cmd._setBlendMode;
            obj.setProperty("mode", props.mode);
            return obj;
        }

        case Variant::SetFont: {
            auto props = cmd._setFont;
            obj.setProperty("family", props.family);
            obj.setProperty("size", props.size);
            obj.setProperty("weight", props.weight);
            obj.setProperty("italic", props.italic);
            return obj;
        }

        case Variant::FillPath: {
            auto props = cmd._fillPath;
            obj.setProperty("path", qPainterPathToScriptValue(engine, props.path));
            return obj;
        }

        case Variant::FillRect: {
            auto props = cmd._fillRect;
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::FillEllipse: {
            auto props = cmd._fillEllipse;
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::FillText: {
            auto props = cmd._fillText;
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            obj.setProperty("text", props.text);
            obj.setProperty("flag", static_cast<uint>(props.flag));
            return obj;
        }

        case Variant::StrokePath: {
            auto props = cmd._strokePath;
            obj.setProperty("path", qPainterPathToScriptValue(engine, props.path));
            return obj;
        }

        case Variant::StrokeRect: {
            auto props = cmd._strokeRect;
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::StrokeArc: {
            auto props = cmd._strokeArc;
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            obj.setProperty("startAngle", props.startAngle);
            obj.setProperty("spanAngle", props.spanAngle);
            return obj;
        }

        case Variant::StrokeEllipse: {
            auto props = cmd._strokeEllipse;
            obj.setProperty("x", props.rect.x());
            obj.setProperty("y", props.rect.y());
            obj.setProperty("w", props.rect.width());
            obj.setProperty("h", props.rect.height());
            return obj;
        }

        case Variant::Point: {
            auto props = cmd._point;
            obj.setProperty("x", props.point.x());
            obj.setProperty("y", props.point.y());
            return obj;
        }

        case Variant::Line: {
            auto props = cmd._line;
            obj.setProperty("x1", props.line.x1());
            obj.setProperty("y1", props.line.y1());
            obj.setProperty("x2", props.line.x2());
            obj.setProperty("y2", props.line.y2());
            return obj;
        }

        case Variant::ImageCopy: {
            auto props = cmd._imageCopy;
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

    uint type = object.property(CMD_TYPE_PROP_NAME).toVariant().toUInt();

    if (type == static_cast<uint>(Variant::SetStrokeWidth)) {
        cmd.set(CanvasCommand::SetStrokeWidth { object.property("width").toNumber() });
    } else if (type == static_cast<uint>(Variant::SetColor)) {
        glm::u8vec3 c;
        if (!u8vec3FromScriptValue(object.property("color"), c)) { return false; }

        // FIXME: we have a script RGB color type but not an RGBA one
        cmd.set(CanvasCommand::SetColor { QColor(c[0], c[1], c[2], 255) });
    } else if (type == static_cast<uint>(Variant::SetHints)) {
        cmd.set(CanvasCommand::SetHints {
            static_cast<CanvasCommand::RenderHint>(object.property("hints").toVariant().toUInt())
        });
    } else if (type == static_cast<uint>(Variant::SetBlendMode)) {
        cmd.set(CanvasCommand::SetBlendMode {
            static_cast<QPainter::CompositionMode>(object.property("mode").toVariant().toUInt())
        });
    } else if (type == static_cast<uint>(Variant::SetFont)) {
        cmd.set(CanvasCommand::SetFont {
            object.property("family").toString(),
            object.property("size").toVariant().toInt(),
            object.property("weight").toVariant().toInt(),
            object.property("italic").toBool(),
        });
    } else if (type == static_cast<uint>(Variant::ClearRect)) {
        cmd.set(CanvasCommand::ClearRect {
            QRect(
                object.property("x").toVariant().toInt(),
                object.property("y").toVariant().toInt(),
                object.property("w").toVariant().toInt(),
                object.property("h").toVariant().toInt()
            )
        });
    } else if (type == static_cast<uint>(Variant::FillPath)) {
        cmd.set(CanvasCommand::FillPath {
            qPainterPathFromScriptValue(object.property("path"))
        });
    } else if (type == static_cast<uint>(Variant::FillRect)) {
        cmd.set(CanvasCommand::FillRect {
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            )
        });
    } else if (type == static_cast<uint>(Variant::FillEllipse)) {
        cmd.set(CanvasCommand::FillEllipse {
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            )
        });
    } else if (type == static_cast<uint>(Variant::FillText)) {
        cmd.set(CanvasCommand::FillText {
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            ),
            object.property("text").toString(),
            static_cast<Qt::AlignmentFlag>(object.property("flag").toVariant().toUInt()),
        });
    } else if (type == static_cast<uint>(Variant::StrokePath)) {
        cmd.set(CanvasCommand::StrokePath {
            qPainterPathFromScriptValue(object.property("path"))
        });
    } else if (type == static_cast<uint>(Variant::StrokeRect)) {
        cmd.set(CanvasCommand::StrokeRect {
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            )
        });
    } else if (type == static_cast<uint>(Variant::StrokeArc)) {
        cmd.set(CanvasCommand::StrokeArc {
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            ),
            object.property("startAngle").toNumber(),
            object.property("spanAngle").toNumber(),
        });
    } else if (type == static_cast<uint>(Variant::StrokeEllipse)) {
        cmd.set(CanvasCommand::StrokeEllipse {
            QRectF(
                object.property("x").toNumber(),
                object.property("y").toNumber(),
                object.property("w").toNumber(),
                object.property("h").toNumber()
            )
        });
    } else if (type == static_cast<uint>(Variant::Point)) {
        cmd.set(CanvasCommand::Point {
            QPointF(
                object.property("x").toNumber(),
                object.property("y").toNumber()
            )
        });
    } else if (type == static_cast<uint>(Variant::Line)) {
        cmd.set(CanvasCommand::Line {
            QLineF(
                object.property("x1").toNumber(),
                object.property("y1").toNumber(),
                object.property("x2").toNumber(),
                object.property("y2").toNumber()
            )
        });
    } else if (type == static_cast<uint>(Variant::ImageCopy)) {
        cmd.set(CanvasCommand::ImageCopy {
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
            ),
            canvasImageFromScriptValue(object.property("image")),
        });
    } else {
        cmd.set(CanvasCommand::Invalid());
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

ScriptValue canvasImageToScriptValue(ScriptEngine* engine, const CanvasImage& img) {
    ScriptValue obj = engine->newObject();
    obj.setProperty(IMG_WIDTH_PROP_NAME, img.width);
    obj.setProperty(IMG_HEIGHT_PROP_NAME, img.height);
    obj.setProperty(IMG_BUFFER_PROP_NAME, qBytearrayToScriptValue(engine, img.buffer));
    return obj;
}

bool canvasImageFromScriptValue(const ScriptValue& object, CanvasImage& img) {
    img.width = object.property(IMG_WIDTH_PROP_NAME).toVariant().toUInt();
    img.height = object.property(IMG_HEIGHT_PROP_NAME).toVariant().toUInt();
    img.buffer = object.property(IMG_BUFFER_PROP_NAME).toVariant().toByteArray();
    return true;
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
        uint type = obj.property("type").toVariant().toUInt();
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
