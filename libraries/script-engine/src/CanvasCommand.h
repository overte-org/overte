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

/*@jsdoc
 * @typedef {object} CanvasImage
 * @property {ArrayBuffer} data - RGBA8 pixel data
 * @property {number} width - Image width in pixels
 * @property {number} height - Image height in pixels
 */
struct CanvasImage {
    // TODO: TypedArray reference
    QByteArray buffer;

    int width, height;
    bool _ownsData; // true if buffer is valid, false if using TypedArray

    CanvasImage() : buffer(QByteArray()), width(0), height(0), _ownsData(true) {}
    CanvasImage(QByteArray buffer, int width, int height) : buffer(buffer), width(width), height(height), _ownsData(true) {}
};

/*@jsdoc
 * @typedef {object} CanvasPathElement
 * @property {number} type - 0: Move to, 1: Line to, 2: Cubic curve to
 * @property {number} x
 * @property {number} y
 * @property {number} c1x
 * @property {number} c1y
 * @property {number} c2x
 * @property {number} c2y
 */
struct CanvasPathElement {
    int type;
    qreal x, y, c1x, c1y, c2x, c2y;

    CanvasPathElement(int type, qreal x, qreal y) : type(type), x(x), y(y), c1x(0), c1y(0), c2x(0), c2y(0) {}
    CanvasPathElement(int type, qreal x, qreal y, qreal c1x, qreal c1y, qreal c2x, qreal c2y) : type(type), x(x), y(y), c1x(c1x), c1y(c1y), c2x(c2x), c2y(c2y) {}
    CanvasPathElement() : type(0), x(0), y(0), c1x(0), c1y(0), c2x(0), c2y(0) {}
};

/*@jsdoc
 * @namespace CanvasCommand
 *
 * @hifi-interface
 * @hifi-client-entity
 *
 * @readonly
 * @property {number} HINT_NO_ANTIALIASING=1 - If set, drawn shapes will have pixelated edges.
 * @property {number} HINT_NO_TEXT_ANTIALIASING=2 - If set, text will have pixelated edges.
 * @property {number} HINT_NEAREST_SCALING=4 - If set, drawn images will be scaled using pixelated, nearest-neighbor scaling.
 *
 * @property {number} TEXT_ALIGN_LEFT=1 - Align drawn text to the left side of its bounding box.
 * @property {number} TEXT_ALIGN_RIGHT=2 - Align drawn text to the right side of its bounding box.
 * @property {number} TEXT_ALIGN_HCENTER=4 - Align drawn text horizontally to the center of its bounding box.
 * @property {number} TEXT_ALIGN_JUSTIFY=8 - Justifies text horizontally to fit in its bounding box.
 * @property {number} TEXT_ALIGN_TOP=32 - Align drawn text to the top of its bounding box.
 * @property {number} TEXT_ALIGN_BOTTOM=64 - Align drawn text to the bottom of its bounding box.
 * @property {number} TEXT_ALIGN_VCENTER=32 - Align drawn text vertically to the center of its bounding box.
 * @property {number} TEXT_ALIGN_CENTER=132 - Align drawn text to the center of its bounding box, both horizontally and vertically.
 *
 * @property {number} BLEND_SOURCEOVER=0
 * @property {number} BLEND_DESTINATIONOVER=1
 * @property {number} BLEND_CLEAR=2
 * @property {number} BLEND_SOURCE=3
 * @property {number} BLEND_DESTINATION=4
 * @property {number} BLEND_SOURCEIN=5
 * @property {number} BLEND_DESTINATIONIN=6
 * @property {number} BLEND_SOURCEOUT=7
 * @property {number} BLEND_DESTINATIONOUT=8
 * @property {number} BLEND_SOURCEATOP=9
 * @property {number} BLEND_DESTINATIONATOP=10
 * @property {number} BLEND_XOR=11
 * @property {number} BLEND_PLUS=12
 * @property {number} BLEND_MULTIPLY=13
 * @property {number} BLEND_SCREEN=14
 * @property {number} BLEND_OVERLAY=15
 * @property {number} BLEND_DARKEN=16
 * @property {number} BLEND_LIGHTEN=17
 * @property {number} BLEND_COLORDODGE=18
 * @property {number} BLEND_COLORBURN=19
 * @property {number} BLEND_HARDLIGHT=20
 * @property {number} BLEND_SOFTLIGHT=21
 * @property {number} BLEND_DIFFERENCE=22
 * @property {number} BLEND_EXCLUSION=23
 *
 * @example <caption>Create a canvas entity and draw "Hello, world!" into it as text.</caption>
 * const CanvasCommand = Script.require("canvasCommand");
 *
 * const canvas = Entities.addEntity({
 *     type: "Canvas",
 *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -1 })),
 *     dimensions: { x: 1, y: 0.5, z: 0.01 },
 *     lifetime: 30,  // Delete after 30 seconds.
 *     width: 256,
 *     height: 128,
 *     unlit: true,
 *     transparent: true,
 * }, "local");
 *
 * Entities.canvasPushCommands(canvas, [
 *     CanvasCommand.color([255, 255, 255, 255]),
 *     CanvasCommand.font("sans-serif", 20),
 *     CanvasCommand.fillText(
 *         "Hello, world!",
 *         0, 0,
 *         256, 128,
 *         CanvasCommand.TEXT_ALIGN_CENTER
 *     ),
 * ]);
 *
 * Entities.canvasCommit(canvas);
 * @example <caption>Create a canvas entity and draw an XOR pattern into it.</caption>
 * const CanvasCommand = Script.require("canvasCommand");
 *
 * const canvas = Entities.addEntity({
 *     type: "Canvas",
 *     position: Vec3.sum(MyAvatar.getHeadPosition(), Vec3.multiplyQbyV(MyAvatar.orientation, [0, 0, -2])),
 *     rotation: MyAvatar.orientation,
 *     dimensions: [2, 2, 0.1],
 *     width: 256,
 *     height: 256,
 *     unlit: true,
 *     collisionless: true,
 * }, "local");
 *
 * const buffer = new Uint8Array(256 * 256 * 4);
 * const img = {data: buffer.buffer, width: 256, height: 256};
 *
 * for (let x = 0; x < 256; x++) {
 *     for (let y = 0; y < 256; y++) {
 *         let color = x ^ y;
 *         buffer[(y * 256 * 4) + (x * 4) + 0] = color;
 *         buffer[(y * 256 * 4) + (x * 4) + 1] = color;
 *         buffer[(y * 256 * 4) + (x * 4) + 2] = color;
 *         buffer[(y * 256 * 4) + (x * 4) + 3] = 255;
 *     }
 * }
 *
 * Entities.canvasPushPixels(canvas, img);
 * Entities.canvasCommit(canvas);
 *
 * Script.scriptEnding.connect(() => Entities.deleteEntity(canvas));
 *
 * // delete after 10 seconds
 * Script.setTimeout(() => Script.stop(), 1000 * 10);
 */

/*@jsdoc
 * Sets the line width for stroked shapes.
 * @name CanvasCommand.strokeWidth
 * @function
 * @param {number} width - Stroke width in pixels.
 * @returns {object}
 */

/*@jsdoc
 * Sets the stroke and fill color to use.
 * @name CanvasCommand.color
 * @function
 * @param {Color|Vec4} color - Stroke and fill color to use.
 * @returns {object}
 */

/*@jsdoc
 * Sets rendering hints.
 * @name CanvasCommand.hints
 * @function
 * @param {number} hints
 * @returns {object}
 */

/*@jsdoc
 * Sets the blending mode.
 * @name CanvasCommand.blendMode
 * @function
 * @param {number} mode
 * @returns {object}
 */

/*@jsdoc
 * Sets the font to use for {@link CanvasCommand.fillText}.
 * @name CanvasCommand.font
 * @function
 * @param {string} family
 * @param {number} size=16 - Pixel height
 * @param {number} weight=400 - 400 is normal, 700 is bold
 * @param {boolean} italic=false
 * @returns {object}
 */

/*@jsdoc
 * Clears a region of a canvas with transparent black.
 * @name CanvasCommand.clearRect
 * @function
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.fillPath
 * @function
 * @param {CanvasPathElement[]} path
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.fillRect
 * @function
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.fillEllipse
 * @function
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.fillText
 * @function
 * @param {string} text
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @param {number} flags
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.strokePath
 * @function
 * @param {CanvasPathElement[]} path
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.strokeRect
 * @function
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.strokeArc
 * @function
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @param {number} startAngle
 * @param {number} spanAngle
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.strokeEllipse
 * @function
 * @param {number} x - Left of the rectangle.
 * @param {number} x - Top of the rectangle.
 * @param {number} w - Width of the rectangle.
 * @param {number} h - Height of the rectangle.
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.point
 * @function
 * @param {number} x
 * @param {number} y
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.line
 * @function
 * @param {number} x1
 * @param {number} y1
 * @param {number} x2
 * @param {number} y2
 * @returns {object}
 */

/*@jsdoc
 * @name CanvasCommand.imageCopy
 * @function
 * @param {CanvasImage} image
 * @param {Vec4|Rect} srcRect
 * @param {Vec4|Rect} destRect
 * @returns {object}
 */

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
        NoPrimitiveAntialiasing = (1 << 0),
        NoTextAntialiasing = (1 << 1),
        NearestImageScaling = (1 << 2),
    };

    static CanvasCommand none() {
        return CanvasCommand {};
    }

    static CanvasCommand setStrokeWidth(qreal width) {
        CanvasCommand cmd;
        cmd.kind = SetStrokeWidth;
        cmd._float[0] = width;
        return cmd;
    }

    static CanvasCommand setColor(const QColor& color) {
        CanvasCommand cmd;
        cmd.kind = SetColor;
        cmd._color = color;
        return cmd;
    }

    static CanvasCommand setHints(int hints) {
        CanvasCommand cmd;
        cmd.kind = SetHints;
        cmd._int[0] = hints;
        return cmd;
    }

    static CanvasCommand setBlendMode(int mode) {
        CanvasCommand cmd;
        cmd.kind = SetBlendMode;
        cmd._int[0] = mode;
        return cmd;
    }

    static CanvasCommand setFont(const QString& family, int size, int weight, bool italic) {
        CanvasCommand cmd;
        cmd.kind = SetFont;
        cmd._text = family;
        cmd._int[0] = size;
        cmd._int[1] = weight;
        cmd._int[2] = italic;
        return cmd;
    }

    static CanvasCommand clearRect(int x, int y, int w, int h) {
        CanvasCommand cmd;
        cmd.kind = ClearRect;
        cmd._int[0] = x;
        cmd._int[1] = y;
        cmd._int[2] = w;
        cmd._int[3] = h;
        return cmd;
    }

    static CanvasCommand fillPath(const QPainterPath& path) {
        CanvasCommand cmd;
        cmd.kind = FillPath;
        cmd._paintPath = path;
        return cmd;
    }

    static CanvasCommand fillRect(const QRectF& rect) {
        CanvasCommand cmd;
        cmd.kind = FillRect;
        return CanvasCommand { .kind = FillRect, ._rect = rect };
    }

    static CanvasCommand fillEllipse(const QRectF& rect) {
        CanvasCommand cmd;
        cmd.kind = FillEllipse;
        cmd._rect = rect;
        return cmd;
    }

    static CanvasCommand fillText(const QString& text, const QRectF& rect, int flag) {
        CanvasCommand cmd;
        cmd.kind = FillText;
        cmd._text = text;
        cmd._rect = rect;
        cmd._int[0] = flag;
        return cmd;
    }

    static CanvasCommand strokePath(const QPainterPath& path) {
        CanvasCommand cmd;
        cmd.kind = StrokePath;
        cmd._paintPath = path;
        return cmd;
    }

    static CanvasCommand strokeRect(const QRectF& rect) {
        CanvasCommand cmd;
        cmd.kind = StrokeRect;
        cmd._rect = rect;
        return cmd;
    }

    static CanvasCommand strokeArc(const QRectF& rect, qreal startAngle, qreal spanAngle) {
        CanvasCommand cmd;
        cmd.kind = StrokeArc;
        cmd._rect = rect;
        cmd._float[0] = startAngle;
        cmd._float[1] = spanAngle;
        return cmd;
    }

    static CanvasCommand strokeEllipse(const QRectF& rect) {
        CanvasCommand cmd;
        cmd.kind = StrokeEllipse;
        cmd._rect = rect;
        return cmd;
    }

    static CanvasCommand point(qreal x, qreal y) {
        CanvasCommand cmd;
        cmd.kind = Point;
        cmd._point = QPointF(x, y);
        return cmd;
    }

    static CanvasCommand line(qreal x1, qreal y1, qreal x2, qreal y2) {
        CanvasCommand cmd;
        cmd.kind = Line;
        cmd._line = QLineF(x1, y1, x2, y2);
        return cmd;
    }

    static CanvasCommand imageCopy(const CanvasImage& image, const QRectF& src, const QRectF& dst) {
        CanvasCommand cmd;
        cmd.kind = ImageCopy;
        cmd._rect = src;
        cmd._rect2 = dst;
        cmd._image = image;
        return cmd;
    }

    Variant kind = Invalid;

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
