//
//  ScriptValueUtils.cpp
//  libraries/shared/src
//
//  Created by Anthony Thibault on 4/15/16.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Utilities for working with QtScriptValues
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptValueUtils.h"

#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtCore/QRect>
#include <QtCore/QUrl>
#include <QtCore/QUuid>
#include <QtCore/QTimer>

#include <AACube.h>
#include <shared/MiniPromises.h>
#include <RegisteredMetaTypes.h>
#include <EntityItemID.h>

#include "ScriptEngine.h"
#include "ScriptEngineCast.h"
#include "ScriptValueIterator.h"
#include "ScriptEngineLogging.h"

bool isListOfStrings(const ScriptValue& arg) {
    if (!arg.isArray()) {
        return false;
    }

    auto lengthProperty = arg.property("length");
    if (!lengthProperty.isNumber()) {
        return false;
    }

    int length = lengthProperty.toInt32();
    for (int i = 0; i < length; i++) {
        if (!arg.property(i).isString()) {
            return false;
        }
    }

    return true;
}

void registerMetaTypes(ScriptEngine* engine) {
    scriptRegisterMetaType<glm::vec2, vec2ToScriptValue, vec2FromScriptValue>(engine);
    scriptRegisterMetaType<glm::vec3, vec3ToScriptValue, vec3FromScriptValue>(engine);
    scriptRegisterMetaType<glm::u8vec3, u8vec3ToScriptValue, u8vec3FromScriptValue>(engine);
    scriptRegisterMetaType<glm::vec4, vec4toScriptValue, vec4FromScriptValue>(engine);
    scriptRegisterMetaType<glm::quat, quatToScriptValue, quatFromScriptValue>(engine);
    scriptRegisterMetaType<glm::mat4, mat4toScriptValue, mat4FromScriptValue>(engine);

    scriptRegisterMetaType<QVector< glm::vec3 >, qVectorVec3ToScriptValue, qVectorVec3FromScriptValue>(engine);
    scriptRegisterMetaType<QVector< glm::quat >, qVectorQuatToScriptValue, qVectorQuatFromScriptValue>(engine);
    scriptRegisterMetaType<QVector< bool >, qVectorBoolToScriptValue, qVectorBoolFromScriptValue>(engine);
    scriptRegisterMetaType<QVector< float >, qVectorFloatToScriptValue, qVectorFloatFromScriptValue>(engine);
    scriptRegisterMetaType<QVector< uint32_t >, qVectorIntToScriptValue, qVectorIntFromScriptValue>(engine);
    scriptRegisterMetaType<QVector< QUuid >, qVectorQUuidToScriptValue, qVectorQUuidFromScriptValue>(engine);

    scriptRegisterMetaType<QSizeF, qSizeFToScriptValue, qSizeFFromScriptValue>(engine);
    scriptRegisterMetaType<QRect, qRectToScriptValue, qRectFromScriptValue>(engine);
    scriptRegisterMetaType<QUrl, qURLToScriptValue, qURLFromScriptValue>(engine);
    scriptRegisterMetaType<QColor, qColorToScriptValue, qColorFromScriptValue>(engine);
    scriptRegisterMetaType<QTimer*, qTimerToScriptValue, qTimerFromScriptValue>(engine, "QTimer*");

    scriptRegisterMetaType<PickRay, pickRayToScriptValue, pickRayFromScriptValue>(engine);
    scriptRegisterMetaType<Collision, collisionToScriptValue, collisionFromScriptValue>(engine);
    scriptRegisterMetaType<QUuid, quuidToScriptValue, quuidFromScriptValue>(engine);
    scriptRegisterMetaType<AACube, aaCubeToScriptValue, aaCubeFromScriptValue>(engine);

    scriptRegisterMetaType<StencilMaskMode, stencilMaskModeToScriptValue, stencilMaskModeFromScriptValue>(engine);

    scriptRegisterMetaType<AnimationDetails, animationDetailsToScriptValue, animationDetailsFromScriptValue>(engine);

    scriptRegisterMetaType<MeshProxy*, meshToScriptValue, meshFromScriptValue>(engine);
    scriptRegisterMetaType<MeshProxyList, meshesToScriptValue, meshesFromScriptValue>(engine);

    scriptRegisterSequenceMetaType<QVector<unsigned int> >(engine);
    scriptRegisterSequenceMetaType<QVector<QUuid>>(engine);
    scriptRegisterSequenceMetaType<QVector<EntityItemID>>(engine);

    scriptRegisterSequenceMetaType<QVector<glm::vec2>>(engine);
    scriptRegisterSequenceMetaType<QVector<glm::quat>>(engine);
    scriptRegisterSequenceMetaType<QVector<QString>>(engine);
}

ScriptValue vec2ToScriptValue(ScriptEngine* engine, const glm::vec2& vec2) {
    auto prototype = engine->globalObject().property("__hifi_vec2__");
    if (!prototype.property("defined").toBool()) {
        prototype = engine->evaluate(
            "__hifi_vec2__ = Object.defineProperties({}, { "
            "defined: { value: true },"
            "0: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "1: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "u: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "v: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } }"
            "})");
    }
    ScriptValue value = engine->newObject();
    value.setProperty("x", vec2.x);
    value.setProperty("y", vec2.y);
    value.setPrototype(prototype);
    return value;
}

bool vec2FromScriptValue(const ScriptValue& object, glm::vec2& vec2) {
    if (object.isNumber()) {
        vec2 = glm::vec2(object.toVariant().toFloat());
    } else if (object.isArray()) {
        QVariantList list = object.toVariant().toList();
        if (list.length() == 2) {
            vec2.x = list[0].toFloat();
            vec2.y = list[1].toFloat();
        }
    } else {
        ScriptValue x = object.property("x");
        if (!x.isValid()) {
            x = object.property("u");
        }

        ScriptValue y = object.property("y");
        if (!y.isValid()) {
            y = object.property("v");
        }

        vec2.x = x.toVariant().toFloat();
        vec2.y = y.toVariant().toFloat();
    }
    return true;
}

ScriptValue vec3ToScriptValue(ScriptEngine* engine, const glm::vec3& vec3) {
    auto prototype = engine->globalObject().property("__hifi_vec3__");
    if (!prototype.property("defined").toBool()) {
        prototype = engine->evaluate(
            "__hifi_vec3__ = Object.defineProperties({}, { "
            "defined: { value: true },"
            "0: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "1: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "2: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } },"
            "r: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "g: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "b: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } },"
            "red: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "green: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "blue: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } }"
            "})");
    }
    ScriptValue value = engine->newObject();
    value.setProperty("x", vec3.x);
    value.setProperty("y", vec3.y);
    value.setProperty("z", vec3.z);
    value.setPrototype(prototype);
    return value;
}

ScriptValue vec3ColorToScriptValue(ScriptEngine* engine, const glm::vec3& vec3) {
    auto prototype = engine->globalObject().property("__hifi_vec3_color__");
    if (!prototype.property("defined").toBool()) {
        prototype = engine->evaluate(
            "__hifi_vec3_color__ = Object.defineProperties({}, { "
            "defined: { value: true },"
            "0: { set: function(nv) { return this.red = nv; }, get: function() { return this.red; } },"
            "1: { set: function(nv) { return this.green = nv; }, get: function() { return this.green; } },"
            "2: { set: function(nv) { return this.blue = nv; }, get: function() { return this.blue; } },"
            "r: { set: function(nv) { return this.red = nv; }, get: function() { return this.red; } },"
            "g: { set: function(nv) { return this.green = nv; }, get: function() { return this.green; } },"
            "b: { set: function(nv) { return this.blue = nv; }, get: function() { return this.blue; } },"
            "x: { set: function(nv) { return this.red = nv; }, get: function() { return this.red; } },"
            "y: { set: function(nv) { return this.green = nv; }, get: function() { return this.green; } },"
            "z: { set: function(nv) { return this.blue = nv; }, get: function() { return this.blue; } }"
            "})");
    }
    ScriptValue value = engine->newObject();
    value.setProperty("red", vec3.x);
    value.setProperty("green", vec3.y);
    value.setProperty("blue", vec3.z);
    value.setPrototype(prototype);
    return value;
}

bool vec3FromScriptValue(const ScriptValue& object, glm::vec3& vec3) {
    if (object.isNumber()) {
        vec3 = glm::vec3(object.toVariant().toFloat());
    } else if (object.isString()) {
        QColor qColor(object.toString());
        if (qColor.isValid()) {
            vec3.x = qColor.red();
            vec3.y = qColor.green();
            vec3.z = qColor.blue();
        }
    } else if (object.isArray()) {
        QVariantList list = object.toVariant().toList();
        if (list.length() == 3) {
            vec3.x = list[0].toFloat();
            vec3.y = list[1].toFloat();
            vec3.z = list[2].toFloat();
        }
    } else {
        ScriptValue x = object.property("x");
        if (!x.isValid()) {
            x = object.property("r");
        }
        if (!x.isValid()) {
            x = object.property("red");
        }

        ScriptValue y = object.property("y");
        if (!y.isValid()) {
            y = object.property("g");
        }
        if (!y.isValid()) {
            y = object.property("green");
        }

        ScriptValue z = object.property("z");
        if (!z.isValid()) {
            z = object.property("b");
        }
        if (!z.isValid()) {
            z = object.property("blue");
        }

        vec3.x = x.toVariant().toFloat();
        vec3.y = y.toVariant().toFloat();
        vec3.z = z.toVariant().toFloat();
    }
    return true;
}

ScriptValue u8vec3ToScriptValue(ScriptEngine* engine, const glm::u8vec3& vec3) {
    auto prototype = engine->globalObject().property("__hifi_u8vec3__");
    if (!prototype.property("defined").toBool()) {
        prototype = engine->evaluate(
            "__hifi_u8vec3__ = Object.defineProperties({}, { "
            "defined: { value: true },"
            "0: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "1: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "2: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } },"
            "r: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "g: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "b: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } },"
            "red: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "green: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "blue: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } }"
            "})");
    }
    ScriptValue value = engine->newObject();
    value.setProperty("x", vec3.x);
    value.setProperty("y", vec3.y);
    value.setProperty("z", vec3.z);
    value.setPrototype(prototype);
    return value;
}

ScriptValue u8vec3ColorToScriptValue(ScriptEngine* engine, const glm::u8vec3& vec3) {
    auto prototype = engine->globalObject().property("__hifi_u8vec3_color__");
    if (!prototype.property("defined").toBool()) {
        prototype = engine->evaluate(
            "__hifi_u8vec3_color__ = Object.defineProperties({}, { "
            "defined: { value: true },"
            "0: { set: function(nv) { return this.red = nv; }, get: function() { return this.red; } },"
            "1: { set: function(nv) { return this.green = nv; }, get: function() { return this.green; } },"
            "2: { set: function(nv) { return this.blue = nv; }, get: function() { return this.blue; } },"
            "r: { set: function(nv) { return this.red = nv; }, get: function() { return this.red; } },"
            "g: { set: function(nv) { return this.green = nv; }, get: function() { return this.green; } },"
            "b: { set: function(nv) { return this.blue = nv; }, get: function() { return this.blue; } },"
            "x: { set: function(nv) { return this.red = nv; }, get: function() { return this.red; } },"
            "y: { set: function(nv) { return this.green = nv; }, get: function() { return this.green; } },"
            "z: { set: function(nv) { return this.blue = nv; }, get: function() { return this.blue; } }"
            "})");
    }
    ScriptValue value = engine->newObject();
    value.setProperty("red", vec3.x);
    value.setProperty("green", vec3.y);
    value.setProperty("blue", vec3.z);
    value.setPrototype(prototype);
    return value;
}

bool u8vec3FromScriptValue(const ScriptValue& object, glm::u8vec3& vec3) {
    if (object.isNumber()) {
        vec3 = glm::vec3(object.toVariant().toUInt());
    } else if (object.isString()) {
        QColor qColor(object.toString());
        if (qColor.isValid()) {
            vec3.x = (uint8_t)qColor.red();
            vec3.y = (uint8_t)qColor.green();
            vec3.z = (uint8_t)qColor.blue();
        }
    } else if (object.isArray()) {
        QVariantList list = object.toVariant().toList();
        if (list.length() == 3) {
            vec3.x = list[0].toUInt();
            vec3.y = list[1].toUInt();
            vec3.z = list[2].toUInt();
        }
    } else {
        ScriptValue x = object.property("x");
        if (!x.isValid()) {
            x = object.property("r");
        }
        if (!x.isValid()) {
            x = object.property("red");
        }

        ScriptValue y = object.property("y");
        if (!y.isValid()) {
            y = object.property("g");
        }
        if (!y.isValid()) {
            y = object.property("green");
        }

        ScriptValue z = object.property("z");
        if (!z.isValid()) {
            z = object.property("b");
        }
        if (!z.isValid()) {
            z = object.property("blue");
        }

        vec3.x = x.toVariant().toUInt();
        vec3.y = y.toVariant().toUInt();
        vec3.z = z.toVariant().toUInt();
    }
    return true;
}

ScriptValue vec4toScriptValue(ScriptEngine* engine, const glm::vec4& vec4) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("x", vec4.x);
    obj.setProperty("y", vec4.y);
    obj.setProperty("z", vec4.z);
    obj.setProperty("w", vec4.w);
    return obj;
}

bool vec4FromScriptValue(const ScriptValue& object, glm::vec4& vec4) {
    vec4.x = object.property("x").toVariant().toFloat();
    vec4.y = object.property("y").toVariant().toFloat();
    vec4.z = object.property("z").toVariant().toFloat();
    vec4.w = object.property("w").toVariant().toFloat();
    return true;
}

ScriptValue mat4toScriptValue(ScriptEngine* engine, const glm::mat4& mat4) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("r0c0", mat4[0][0]);
    obj.setProperty("r1c0", mat4[0][1]);
    obj.setProperty("r2c0", mat4[0][2]);
    obj.setProperty("r3c0", mat4[0][3]);
    obj.setProperty("r0c1", mat4[1][0]);
    obj.setProperty("r1c1", mat4[1][1]);
    obj.setProperty("r2c1", mat4[1][2]);
    obj.setProperty("r3c1", mat4[1][3]);
    obj.setProperty("r0c2", mat4[2][0]);
    obj.setProperty("r1c2", mat4[2][1]);
    obj.setProperty("r2c2", mat4[2][2]);
    obj.setProperty("r3c2", mat4[2][3]);
    obj.setProperty("r0c3", mat4[3][0]);
    obj.setProperty("r1c3", mat4[3][1]);
    obj.setProperty("r2c3", mat4[3][2]);
    obj.setProperty("r3c3", mat4[3][3]);
    return obj;
}

bool mat4FromScriptValue(const ScriptValue& object, glm::mat4& mat4) {
    mat4[0][0] = object.property("r0c0").toVariant().toFloat();
    mat4[0][1] = object.property("r1c0").toVariant().toFloat();
    mat4[0][2] = object.property("r2c0").toVariant().toFloat();
    mat4[0][3] = object.property("r3c0").toVariant().toFloat();
    mat4[1][0] = object.property("r0c1").toVariant().toFloat();
    mat4[1][1] = object.property("r1c1").toVariant().toFloat();
    mat4[1][2] = object.property("r2c1").toVariant().toFloat();
    mat4[1][3] = object.property("r3c1").toVariant().toFloat();
    mat4[2][0] = object.property("r0c2").toVariant().toFloat();
    mat4[2][1] = object.property("r1c2").toVariant().toFloat();
    mat4[2][2] = object.property("r2c2").toVariant().toFloat();
    mat4[2][3] = object.property("r3c2").toVariant().toFloat();
    mat4[3][0] = object.property("r0c3").toVariant().toFloat();
    mat4[3][1] = object.property("r1c3").toVariant().toFloat();
    mat4[3][2] = object.property("r2c3").toVariant().toFloat();
    mat4[3][3] = object.property("r3c3").toVariant().toFloat();
    return true;
}

ScriptValue qVectorVec3ColorToScriptValue(ScriptEngine* engine, const QVector<glm::vec3>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        array.setProperty(i, vec3ColorToScriptValue(engine, vector.at(i)));
    }
    return array;
}

ScriptValue qVectorVec3ToScriptValue(ScriptEngine* engine, const QVector<glm::vec3>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        array.setProperty(i, vec3ToScriptValue(engine, vector.at(i)));
    }
    return array;
}

QVector<glm::vec3> qVectorVec3FromScriptValue(const ScriptValue& array) {
    QVector<glm::vec3> newVector;
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        glm::vec3 newVec3 = glm::vec3();
        vec3FromScriptValue(array.property(i), newVec3);
        newVector << newVec3;
    }
    return newVector;
}

bool qVectorVec3FromScriptValue(const ScriptValue& array, QVector<glm::vec3>& vector) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        glm::vec3 newVec3 = glm::vec3();
        vec3FromScriptValue(array.property(i), newVec3);
        vector << newVec3;
    }
    return true;
}

ScriptValue quatToScriptValue(ScriptEngine* engine, const glm::quat& quat) {
    ScriptValue obj = engine->newObject();
    if (quat.x != quat.x || quat.y != quat.y || quat.z != quat.z || quat.w != quat.w) {
        // if quat contains a NaN don't try to convert it
        return obj;
    }
    obj.setProperty("x", quat.x);
    obj.setProperty("y", quat.y);
    obj.setProperty("z", quat.z);
    obj.setProperty("w", quat.w);
    return obj;
}

bool quatFromScriptValue(const ScriptValue& object, glm::quat& quat) {
    quat.x = object.property("x").toVariant().toFloat();
    quat.y = object.property("y").toVariant().toFloat();
    quat.z = object.property("z").toVariant().toFloat();
    quat.w = object.property("w").toVariant().toFloat();

    // enforce normalized quaternion
    float length = glm::length(quat);
    if (length > FLT_EPSILON) {
        quat /= length;
    } else {
        quat = glm::quat();
    }
    return true;
}

ScriptValue qVectorQuatToScriptValue(ScriptEngine* engine, const QVector<glm::quat>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        array.setProperty(i, quatToScriptValue(engine, vector.at(i)));
    }
    return array;
}

ScriptValue qVectorBoolToScriptValue(ScriptEngine* engine, const QVector<bool>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        array.setProperty(i, vector.at(i));
    }
    return array;
}

QVector<float> qVectorFloatFromScriptValue(const ScriptValue& array) {
    if (!array.isArray()) {
        return QVector<float>();
    }
    QVector<float> newVector;
    int length = array.property("length").toInteger();
    newVector.reserve(length);
    for (int i = 0; i < length; i++) {
        if (array.property(i).isNumber()) {
            newVector << array.property(i).toNumber();
        }
    }

    return newVector;
}

ScriptValue qVectorQUuidToScriptValue(ScriptEngine* engine, const QVector<QUuid>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        array.setProperty(i, quuidToScriptValue(engine, vector.at(i)));
    }
    return array;
}

bool qVectorQUuidFromScriptValue(const ScriptValue& array, QVector<QUuid>& vector) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        vector << array.property(i).toVariant().toUuid();
    }
    return true;
}

QVector<QUuid> qVectorQUuidFromScriptValue(const ScriptValue& array) {
    if (!array.isArray()) {
        return QVector<QUuid>();
    }
    QVector<QUuid> newVector;
    int length = array.property("length").toInteger();
    newVector.reserve(length);
    for (int i = 0; i < length; i++) {
        QString uuidAsString = array.property(i).toString();
        QUuid fromString(uuidAsString);
        newVector << fromString;
    }
    return newVector;
}

ScriptValue qVectorFloatToScriptValue(ScriptEngine* engine, const QVector<float>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        float num = vector.at(i);
        array.setProperty(i, engine->newValue(num));
    }
    return array;
}

ScriptValue qVectorIntToScriptValue(ScriptEngine* engine, const QVector<uint32_t>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        int num = vector.at(i);
        array.setProperty(i, engine->newValue(num));
    }
    return array;
}

bool qVectorFloatFromScriptValue(const ScriptValue& array, QVector<float>& vector) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        vector << array.property(i).toVariant().toFloat();
    }
    return true;
}

bool qVectorIntFromScriptValue(const ScriptValue& array, QVector<uint32_t>& vector) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        vector << array.property(i).toVariant().toInt();
    }
    return true;
}

QVector<glm::quat> qVectorQuatFromScriptValue(const ScriptValue& array) {
    QVector<glm::quat> newVector;
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        glm::quat newQuat = glm::quat();
        quatFromScriptValue(array.property(i), newQuat);
        newVector << newQuat;
    }
    return newVector;
}

bool qVectorQuatFromScriptValue(const ScriptValue& array, QVector<glm::quat>& vector) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        glm::quat newQuat = glm::quat();
        quatFromScriptValue(array.property(i), newQuat);
        vector << newQuat;
    }
    return true;
}

QVector<bool> qVectorBoolFromScriptValue(const ScriptValue& array) {
    QVector<bool> newVector;
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        newVector << array.property(i).toBool();
    }
    return newVector;
}

bool qVectorBoolFromScriptValue(const ScriptValue& array, QVector<bool>& vector) {
    int length = array.property("length").toInteger();

    for (int i = 0; i < length; i++) {
        vector << array.property(i).toBool();
    }
    return true;
}

ScriptValue qRectToScriptValue(ScriptEngine* engine, const QRect& rect) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("x", rect.x());
    obj.setProperty("y", rect.y());
    obj.setProperty("width", rect.width());
    obj.setProperty("height", rect.height());
    return obj;
}

bool qRectFromScriptValue(const ScriptValue& object, QRect& rect) {
    rect.setX(object.property("x").toVariant().toInt());
    rect.setY(object.property("y").toVariant().toInt());
    rect.setWidth(object.property("width").toVariant().toInt());
    rect.setHeight(object.property("height").toVariant().toInt());
    return true;
}

ScriptValue qRectFToScriptValue(ScriptEngine* engine, const QRectF& rect) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("x", rect.x());
    obj.setProperty("y", rect.y());
    obj.setProperty("width", rect.width());
    obj.setProperty("height", rect.height());
    return obj;
}

bool qRectFFromScriptValue(const ScriptValue& object, QRectF& rect) {
    rect.setX(object.property("x").toVariant().toFloat());
    rect.setY(object.property("y").toVariant().toFloat());
    rect.setWidth(object.property("width").toVariant().toFloat());
    rect.setHeight(object.property("height").toVariant().toFloat());
    return true;
}

ScriptValue qColorToScriptValue(ScriptEngine* engine, const QColor& color) {
    ScriptValue object = engine->newObject();
    object.setProperty("red", color.red());
    object.setProperty("green", color.green());
    object.setProperty("blue", color.blue());
    object.setProperty("alpha", color.alpha());
    return object;
}

/**jsdoc
 * An axis-aligned cube, defined as the bottom right near (minimum axes values) corner of the cube plus the dimension of its 
 * sides.
 * @typedef {object} AACube
 * @property {number} x - X coordinate of the brn corner of the cube.
 * @property {number} y - Y coordinate of the brn corner of the cube.
 * @property {number} z - Z coordinate of the brn corner of the cube.
 * @property {number} scale - The dimensions of each side of the cube.
 */
ScriptValue aaCubeToScriptValue(ScriptEngine* engine, const AACube& aaCube) {
    ScriptValue obj = engine->newObject();
    const glm::vec3& corner = aaCube.getCorner();
    obj.setProperty("x", corner.x);
    obj.setProperty("y", corner.y);
    obj.setProperty("z", corner.z);
    obj.setProperty("scale", aaCube.getScale());
    return obj;
}

bool aaCubeFromScriptValue(const ScriptValue& object, AACube& aaCube) {
    glm::vec3 corner;
    corner.x = object.property("x").toVariant().toFloat();
    corner.y = object.property("y").toVariant().toFloat();
    corner.z = object.property("z").toVariant().toFloat();
    float scale = object.property("scale").toVariant().toFloat();

    aaCube.setBox(corner, scale);
    return true;
}

ScriptValue qTimerToScriptValue(ScriptEngine* engine, QTimer* const &in) {
    return engine->newQObject(in, ScriptEngine::QtOwnership);
}

bool qTimerFromScriptValue(const ScriptValue& object, QTimer* &out) {
    return (out = qobject_cast<QTimer*>(object.toQObject())) != nullptr;
}

bool qColorFromScriptValue(const ScriptValue& object, QColor& color) {
    if (object.isNumber()) {
        color.setRgb(object.toUInt32());

    } else if (object.isString()) {
        color.setNamedColor(object.toString());

    } else {
        ScriptValue alphaValue = object.property("alpha");
        color.setRgb(object.property("red").toInt32(), object.property("green").toInt32(), object.property("blue").toInt32(),
                     alphaValue.isNumber() ? alphaValue.toInt32() : 255);
    }
    return true;
}

ScriptValue qURLToScriptValue(ScriptEngine* engine, const QUrl& url) {
    return engine->newValue(url.toString());
}

bool qURLFromScriptValue(const ScriptValue& object, QUrl& url) {
    url = object.toString();
    return true;
}

ScriptValue pickRayToScriptValue(ScriptEngine* engine, const PickRay& pickRay) {
    ScriptValue obj = engine->newObject();
    ScriptValue origin = vec3ToScriptValue(engine, pickRay.origin);
    obj.setProperty("origin", origin);
    ScriptValue direction = vec3ToScriptValue(engine, pickRay.direction);
    obj.setProperty("direction", direction);
    return obj;
}

bool pickRayFromScriptValue(const ScriptValue& object, PickRay& pickRay) {
    ScriptValue originValue = object.property("origin");
    if (originValue.isValid()) {
        auto x = originValue.property("x");
        auto y = originValue.property("y");
        auto z = originValue.property("z");
        if (x.isValid() && y.isValid() && z.isValid()) {
            pickRay.origin.x = x.toVariant().toFloat();
            pickRay.origin.y = y.toVariant().toFloat();
            pickRay.origin.z = z.toVariant().toFloat();
        }
    }
    ScriptValue directionValue = object.property("direction");
    if (directionValue.isValid()) {
        auto x = directionValue.property("x");
        auto y = directionValue.property("y");
        auto z = directionValue.property("z");
        if (x.isValid() && y.isValid() && z.isValid()) {
            pickRay.direction.x = x.toVariant().toFloat();
            pickRay.direction.y = y.toVariant().toFloat();
            pickRay.direction.z = z.toVariant().toFloat();
        }
    }
    return true;
}

/**jsdoc
 * Details of a collision between avatars and entities.
 * @typedef {object} Collision
 * @property {ContactEventType} type - The contact type of the collision event.
 * @property {Uuid} idA - The ID of one of the avatars or entities in the collision.
 * @property {Uuid} idB - The ID of the other of the avatars or entities in the collision.
 * @property {Vec3} penetration - The amount of penetration between the two items.
 * @property {Vec3} contactPoint - The point of contact.
 * @property {Vec3} velocityChange - The change in relative velocity of the two items, in m/s.
 */
ScriptValue collisionToScriptValue(ScriptEngine* engine, const Collision& collision) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("type", collision.type);
    obj.setProperty("idA", quuidToScriptValue(engine, collision.idA));
    obj.setProperty("idB", quuidToScriptValue(engine, collision.idB));
    obj.setProperty("penetration", vec3ToScriptValue(engine, collision.penetration));
    obj.setProperty("contactPoint", vec3ToScriptValue(engine, collision.contactPoint));
    obj.setProperty("velocityChange", vec3ToScriptValue(engine, collision.velocityChange));
    return obj;
}

bool collisionFromScriptValue(const ScriptValue& object, Collision& collision) {
    // TODO: implement this when we know what it means to accept collision events from JS
    return false;
}

ScriptValue quuidToScriptValue(ScriptEngine* engine, const QUuid& uuid) {
    if (uuid.isNull()) {
        return engine->nullValue();
    }
    ScriptValue obj(engine->newValue(uuid.toString()));
    return obj;
}

bool quuidFromScriptValue(const ScriptValue& object, QUuid& uuid) {
    if (object.isNull()) {
        uuid = QUuid();
        return true;
    }
    QString uuidAsString = object.toVariant().toString();
    QUuid fromString(uuidAsString);
    uuid = fromString;
    return true;
}

/**jsdoc
 * A 2D size value.
 * @typedef {object} Size
 * @property {number} height - The height value.
 * @property {number} width - The width value.
 */
ScriptValue qSizeFToScriptValue(ScriptEngine* engine, const QSizeF& qSizeF) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("width", qSizeF.width());
    obj.setProperty("height", qSizeF.height());
    return obj;
}

bool qSizeFFromScriptValue(const ScriptValue& object, QSizeF& qSizeF) {
    qSizeF.setWidth(object.property("width").toVariant().toFloat());
    qSizeF.setHeight(object.property("height").toVariant().toFloat());
    return true;
}

/**jsdoc
 * The details of an animation that is playing.
 * @typedef {object} Avatar.AnimationDetails
 * @property {string} role - <em>Not used.</em>
 * @property {string} url - The URL to the animation file. Animation files need to be in glTF or FBX format but only need to 
 *     contain the avatar skeleton and animation data. glTF models may be in JSON or binary format (".gltf" or ".glb" URLs 
 *     respectively).
 *     <p><strong>Warning:</strong> glTF animations currently do not always animate correctly.</p>
 * @property {number} fps - The frames per second(FPS) rate for the animation playback. 30 FPS is normal speed.
 * @property {number} priority - <em>Not used.</em>
 * @property {boolean} loop - <code>true</code> if the animation should loop, <code>false</code> if it shouldn't.
 * @property {boolean} hold - <em>Not used.</em>
 * @property {number} firstFrame - The frame the animation should start at.
 * @property {number} lastFrame - The frame the animation should stop at.
 * @property {boolean} running - <em>Not used.</em>
 * @property {number} currentFrame - The current frame being played.
 * @property {boolean} startAutomatically - <em>Not used.</em>
 * @property {boolean} allowTranslation - <em>Not used.</em>
 */
ScriptValue animationDetailsToScriptValue(ScriptEngine* engine, const AnimationDetails& details) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("role", details.role);
    obj.setProperty("url", details.url.toString());
    obj.setProperty("fps", details.fps);
    obj.setProperty("priority", details.priority);
    obj.setProperty("loop", details.loop);
    obj.setProperty("hold", details.hold);
    obj.setProperty("startAutomatically", details.startAutomatically);
    obj.setProperty("firstFrame", details.firstFrame);
    obj.setProperty("lastFrame", details.lastFrame);
    obj.setProperty("running", details.running);
    obj.setProperty("currentFrame", details.currentFrame);
    obj.setProperty("allowTranslation", details.allowTranslation);
    return obj;
}

bool animationDetailsFromScriptValue(const ScriptValue& object, AnimationDetails& details) {
    // nothing for now...
    return false;
}

ScriptValue meshToScriptValue(ScriptEngine* engine, MeshProxy* const& in) {
    return engine->newQObject(in, ScriptEngine::QtOwnership);
}

bool meshFromScriptValue(const ScriptValue& value, MeshProxy*& out) {
    return (out = qobject_cast<MeshProxy*>(value.toQObject())) != nullptr;
}

ScriptValue meshesToScriptValue(ScriptEngine* engine, const MeshProxyList& in) {
    // ScriptValueList result;
    ScriptValue result = engine->newArray();
    int i = 0;
    foreach (MeshProxy* const meshProxy, in) { result.setProperty(i++, meshToScriptValue(engine, meshProxy)); }
    return result;
}

bool meshesFromScriptValue(const ScriptValue& value, MeshProxyList& out) {
    ScriptValueIteratorPointer itr(value.newIterator());

    qDebug(scriptengine) << "in meshesFromScriptValue, value.length =" << value.property("length").toInt32();

    while (itr->hasNext()) {
        itr->next();
        MeshProxy* meshProxy = scriptvalue_cast<MeshProxyList::value_type>(itr->value());
        if (meshProxy) {
            out.append(meshProxy);
        } else {
            qDebug(scriptengine) << "null meshProxy";
        }
    }
    return true;
}

/**jsdoc
 * A triangle in a mesh.
 * @typedef {object} MeshFace
 * @property {number[]} vertices - The indexes of the three vertices that make up the face.
 */
ScriptValue meshFaceToScriptValue(ScriptEngine* engine, const MeshFace& meshFace) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("vertices", qVectorIntToScriptValue(engine, meshFace.vertexIndices));
    return obj;
}

bool meshFaceFromScriptValue(const ScriptValue& object, MeshFace& meshFaceResult) {
    return qVectorIntFromScriptValue(object.property("vertices"), meshFaceResult.vertexIndices);
}

ScriptValue qVectorMeshFaceToScriptValue(ScriptEngine* engine, const QVector<MeshFace>& vector) {
    ScriptValue array = engine->newArray();
    for (int i = 0; i < vector.size(); i++) {
        array.setProperty(i, meshFaceToScriptValue(engine, vector.at(i)));
    }
    return array;
}

bool qVectorMeshFaceFromScriptValue(const ScriptValue& array, QVector<MeshFace>& result) {
    int length = array.property("length").toInteger();
    result.clear();

    for (int i = 0; i < length; i++) {
        MeshFace meshFace = MeshFace();
        meshFaceFromScriptValue(array.property(i), meshFace);
        result << meshFace;
    }
    return true;
}

ScriptValue stencilMaskModeToScriptValue(ScriptEngine* engine, const StencilMaskMode& stencilMode) {
    return engine->newValue((int)stencilMode);
}

bool stencilMaskModeFromScriptValue(const ScriptValue& object, StencilMaskMode& stencilMode) {
    stencilMode = StencilMaskMode(object.toVariant().toInt());
    return true;
}

bool promiseFromScriptValue(const ScriptValue& object, std::shared_ptr<MiniPromise>& promise) {
    Q_ASSERT(false);
    return false;
}
ScriptValue promiseToScriptValue(ScriptEngine* engine, const std::shared_ptr<MiniPromise>& promise) {
    return engine->newQObject(promise.get());
}

ScriptValue EntityItemIDtoScriptValue(ScriptEngine* engine, const EntityItemID& id) {
    return quuidToScriptValue(engine, id);
}

bool EntityItemIDfromScriptValue(const ScriptValue& object, EntityItemID& id) {
    return quuidFromScriptValue(object, id);
}

QVector<EntityItemID> qVectorEntityItemIDFromScriptValue(const ScriptValue& array) {
    if (!array.isArray()) {
        return QVector<EntityItemID>();
    }
    QVector<EntityItemID> newVector;
    int length = array.property("length").toInteger();
    newVector.reserve(length);
    for (int i = 0; i < length; i++) {
        QString uuidAsString = array.property(i).toString();
        EntityItemID fromString(uuidAsString);
        newVector << fromString;
    }
    return newVector;
}
