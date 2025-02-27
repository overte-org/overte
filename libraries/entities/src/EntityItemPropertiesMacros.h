//
//  EntityItemPropertiesMacros.h
//  libraries/entities/src
//
//  Created by Brad Hefta-Gaub on 9/10/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//


#ifndef hifi_EntityItemPropertiesMacros_h
#define hifi_EntityItemPropertiesMacros_h

#include <EntityItemID.h>
#include <EntityPropertyFlags.h>
#include <RegisteredMetaTypes.h>
#include <ScriptEngine.h>
#include <ScriptValue.h>
#include <ScriptValueUtils.h>

const quint64 UNKNOWN_CREATED_TIME = 0;

using vec3Color = glm::vec3;
using u8vec3Color = glm::u8vec3;

struct EntityPropertyInfo {
    EntityPropertyInfo(EntityPropertyList propEnum) :
        propertyEnums(propEnum) {}
    EntityPropertyInfo(EntityPropertyList propEnum, QVariant min, QVariant max) :
        propertyEnums(propEnum), minimum(min), maximum(max) {}
    EntityPropertyInfo() = default;
    EntityPropertyFlags propertyEnums;
    QVariant minimum;
    QVariant maximum;
};

template <typename T>
EntityPropertyInfo makePropertyInfo(EntityPropertyList p, typename std::enable_if<!std::is_integral<T>::value>::type* = 0) {
    return EntityPropertyInfo(p);
}

template <typename T>
EntityPropertyInfo makePropertyInfo(EntityPropertyList p, typename std::enable_if<std::is_integral<T>::value>::type* = 0) {
    return EntityPropertyInfo(p, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
}

#define APPEND_ENTITY_PROPERTY(P,V) \
        if (requestedProperties.getHasProperty(P)) {                \
            LevelDetails propertyLevel = packetData->startLevel();  \
            successPropertyFits = packetData->appendValue(V);       \
            if (successPropertyFits) {                              \
                propertyFlags |= P;                                 \
                propertiesDidntFit -= P;                            \
                propertyCount++;                                    \
                packetData->endLevel(propertyLevel);                \
            } else {                                                \
                packetData->discardLevel(propertyLevel);            \
                appendState = OctreeElement::PARTIAL;               \
            }                                                       \
        } else {                                                    \
            propertiesDidntFit -= P;                                \
        }

#define READ_ENTITY_PROPERTY(P,T,S)                                                \
        if (propertyFlags.getHasProperty(P)) {                                     \
            T fromBuffer;                                                          \
            int bytes = OctreePacketData::unpackDataFromBytes(dataAt, fromBuffer); \
            dataAt += bytes;                                                       \
            bytesRead += bytes;                                                    \
            if (overwriteLocalData) {                                              \
                S(fromBuffer);                                                     \
            }                                                                      \
            somethingChanged = true;                                               \
        }

#define SKIP_ENTITY_PROPERTY(P,T)                                                  \
        if (propertyFlags.getHasProperty(P)) {                                     \
            T fromBuffer;                                                          \
            int bytes = OctreePacketData::unpackDataFromBytes(dataAt, fromBuffer); \
            dataAt += bytes;                                                       \
            bytesRead += bytes;                                                    \
        }

#define DECODE_GROUP_PROPERTY_HAS_CHANGED(P,N) \
        if (propertyFlags.getHasProperty(P)) {  \
            set##N##Changed(true); \
        }


#define READ_ENTITY_PROPERTY_TO_PROPERTIES(P,T,O)                                  \
        if (propertyFlags.getHasProperty(P)) {                                     \
            T fromBuffer;                                                          \
            int bytes = OctreePacketData::unpackDataFromBytes(dataAt, fromBuffer); \
            dataAt += bytes;                                                       \
            processedBytes += bytes;                                               \
            properties.O(fromBuffer);                                              \
        }

#define SET_ENTITY_PROPERTY_FROM_PROPERTIES(P,M)    \
    if (properties._##P##Changed) {    \
        M(properties._##P);                         \
        somethingChanged = true;                    \
    }

#define SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(G,P,p,M)  \
    if (properties.get##G().p##Changed()) {                 \
        M(properties.get##G().get##P());                    \
        somethingChanged = true;                            \
    }

#define SET_ENTITY_PROPERTY_FROM_PROPERTIES_GETTER(C,G,S)    \
    if (properties.C()) {    \
        S(properties.G());                         \
        somethingChanged = true;                    \
    }

#define COPY_ENTITY_PROPERTY_TO_PROPERTIES(P,M) \
    properties._##P = M();                      \
    properties._##P##Changed = false;

#define COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(G,P,M)  \
    properties.get##G().set##P(M());                     \
    properties.get##G().set##P##Changed(false);

#define CHECK_PROPERTY_CHANGE(P,M) \
    if (_##M##Changed) {           \
        changedProperties += P;    \
    }

inline ScriptValue convertScriptValue(ScriptEngine* e, const glm::vec2& v) { return vec2ToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const glm::vec3& v) { return vec3ToScriptValue(e, v); }
inline ScriptValue vec3Color_convertScriptValue(ScriptEngine* e, const glm::vec3& v) { return vec3ColorToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const glm::u8vec3& v) { return u8vec3ToScriptValue(e, v); }
inline ScriptValue u8vec3Color_convertScriptValue(ScriptEngine* e, const glm::u8vec3& v) { return u8vec3ColorToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, float v) { return e->newValue(v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, int v) { return e->newValue(v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, bool v) { return e->newValue(v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, quint16 v) { return e->newValue(v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, quint32 v) { return e->newValue(v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, quint64 v) { return e->newValue((double)v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QString& v) { return e->newValue(v); }

inline ScriptValue convertScriptValue(ScriptEngine* e, const glm::quat& v) { return quatToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const ScriptValue& v) { return v; }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QVector<glm::vec3>& v) {return qVectorVec3ToScriptValue(e, v); }
inline ScriptValue qVectorVec3Color_convertScriptValue(ScriptEngine* e, const QVector<glm::vec3>& v) {return qVectorVec3ColorToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QVector<glm::quat>& v) {return qVectorQuatToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QVector<bool>& v) {return qVectorBoolToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QVector<float>& v) { return qVectorFloatToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QVector<QUuid>& v) { return qVectorQUuidToScriptValue(e, v); }
inline ScriptValue convertScriptValue(ScriptEngine* e, const QVector<QString>& v) { return qVectorQStringToScriptValue(e, v); }

inline ScriptValue convertScriptValue(ScriptEngine* e, const QRect& v) { return qRectToScriptValue(e, v); }

inline ScriptValue convertScriptValue(ScriptEngine* e, const QByteArray& v) {
    QByteArray b64 = v.toBase64();
    return e->newValue(QString(b64));
}

inline ScriptValue convertScriptValue(ScriptEngine* e, const EntityItemID& v) { return e->newValue(QUuid(v).toString()); }

inline ScriptValue convertScriptValue(ScriptEngine* e, const AACube& v) { return aaCubeToScriptValue(e, v); }

#define COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(X,G,g,P,p) \
    if (((!returnNothingOnEmptyPropertyFlags && desiredProperties.isEmpty()) || desiredProperties.getHasProperty(X)) && \
        (!skipDefaults || defaultEntityProperties.get##G().get##P() != get##P())) { \
        ScriptValue groupProperties = properties.property(#g); \
        if (!groupProperties.isValid()) { \
            groupProperties = engine->newObject(); \
        } \
        ScriptValue V = convertScriptValue(engine, get##P()); \
        groupProperties.setProperty(#p, V); \
        properties.setProperty(#g, groupProperties); \
    }

#define COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_TYPED(X,G,g,P,p,T) \
    if (((!returnNothingOnEmptyPropertyFlags && desiredProperties.isEmpty()) || desiredProperties.getHasProperty(X)) && \
        (!skipDefaults || defaultEntityProperties.get##G().get##P() != get##P())) { \
        ScriptValue groupProperties = properties.property(#g); \
        if (!groupProperties.isValid()) { \
            groupProperties = engine->newObject(); \
        } \
        ScriptValue V = T##_convertScriptValue(engine, get##P()); \
        groupProperties.setProperty(#p, V); \
        properties.setProperty(#g, groupProperties); \
    }

#define COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_GETTER(X,G,g,P,p,M)                       \
    if (((!returnNothingOnEmptyPropertyFlags && desiredProperties.isEmpty()) || desiredProperties.getHasProperty(X)) &&       \
        (!skipDefaults || defaultEntityProperties.get##G().get##P() != get##P())) {   \
        ScriptValue groupProperties = properties.property(#g);                        \
        if (!groupProperties.isValid()) {                                             \
            groupProperties = engine->newObject();                                    \
        }                                                                             \
        ScriptValue V = convertScriptValue(engine, M());                              \
        groupProperties.setProperty(#p, V);                                           \
        properties.setProperty(#g, groupProperties);                                  \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE(p,P) \
    if (((!returnNothingOnEmptyPropertyFlags && _desiredProperties.isEmpty()) || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        ScriptValue V = convertScriptValue(engine, _##P); \
        properties.setProperty(#P, V);     \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_TYPED(p,P,T) \
    if (((!returnNothingOnEmptyPropertyFlags && _desiredProperties.isEmpty()) || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        ScriptValue V = T##_convertScriptValue(engine, _##P); \
        properties.setProperty(#P, V); \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER_NO_SKIP(P, G) \
    properties.setProperty(#P, G);

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER(p, P, G) \
    if (((!returnNothingOnEmptyPropertyFlags && _desiredProperties.isEmpty()) || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        ScriptValue V = convertScriptValue(engine, G); \
        properties.setProperty(#P, V); \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER_TYPED(p, P, G, T) \
    if ((_desiredProperties.isEmpty() || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        ScriptValue V = T##_convertScriptValue(engine, G()); \
        properties.setProperty(#P, V); \
    }

// same as COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER but uses #X instead of #P in the setProperty() step
#define COPY_PROXY_PROPERTY_TO_QSCRIPTVALUE_GETTER(p, P, X, G) \
    if (((!returnNothingOnEmptyPropertyFlags && _desiredProperties.isEmpty()) || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) { \
        ScriptValue V = convertScriptValue(engine, G()); \
        properties.setProperty(#X, V); \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_GETTER_ALWAYS(P, G) \
    if (!skipDefaults || defaultEntityProperties._##P != _##P) { \
        ScriptValue V = convertScriptValue(engine, G); \
        properties.setProperty(#P, V); \
    }

#define COPY_PROPERTY_TO_QSCRIPTVALUE_IF_URL_PERMISSION(p, P)                                                             \
    if (((!returnNothingOnEmptyPropertyFlags && _desiredProperties.isEmpty()) || _desiredProperties.getHasProperty(p)) && \
        (!skipDefaults || defaultEntityProperties._##P != _##P)) {                                                        \
        if (isMyOwnAvatarEntity || nodeList->getThisNodeCanViewAssetURLs()) {                                             \
            ScriptValue V = convertScriptValue(engine, _##P);                                                             \
            properties.setProperty(#P, V);                                                                                \
        } else {                                                                                                          \
            const QString emptyURL = "";                                                                                  \
            ScriptValue V = convertScriptValue(engine, emptyURL);                                                         \
            properties.setProperty(#P, V);                                                                                \
        }                                                                                                                 \
    }

#define COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_IF_URL_PERMISSION(X, G, g, P, p)                                            \
    if (((!returnNothingOnEmptyPropertyFlags && desiredProperties.isEmpty()) || desiredProperties.getHasProperty(X)) && \
        (!skipDefaults || defaultEntityProperties.get##G().get##P() != get##P())) {                                     \
        if (isMyOwnAvatarEntity || nodeList->getThisNodeCanViewAssetURLs()) {                                           \
            ScriptValue groupProperties = properties.property(#g);                                                      \
            if (!groupProperties.isValid()) {                                                                           \
                groupProperties = engine->newObject();                                                                  \
            }                                                                                                           \
            ScriptValue V = convertScriptValue(engine, get##P());                                                       \
            groupProperties.setProperty(#p, V);                                                                         \
            properties.setProperty(#g, groupProperties);                                                                \
        } else {                                                                                                        \
            const QString emptyURL = "";                                                                                \
            ScriptValue V = convertScriptValue(engine, emptyURL);                                                       \
            properties.setProperty(#P, V);                                                                              \
        }                                                                                                               \
    }

typedef QVector<glm::vec3> qVectorVec3;
typedef QVector<glm::quat> qVectorQuat;
typedef QVector<bool> qVectorBool;
typedef QVector<float> qVectorFloat;
typedef QVector<QUuid> qVectorQUuid;
typedef QVector<QString> qVectorQString;
typedef QSet<QString> qSetQString;
inline float float_convertFromScriptValue(const ScriptValue& v, bool& isValid) { return v.toVariant().toFloat(&isValid); }
inline quint64 quint64_convertFromScriptValue(const ScriptValue& v, bool& isValid) { return v.toVariant().toULongLong(&isValid); }
inline quint32 quint32_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    // Use QString::toUInt() so that isValid is set to false if the number is outside the quint32 range.
    return v.toString().toUInt(&isValid);
}
inline quint16 quint16_convertFromScriptValue(const ScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline uint16_t uint16_t_convertFromScriptValue(const ScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline uint32_t uint32_t_convertFromScriptValue(const ScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline int int_convertFromScriptValue(const ScriptValue& v, bool& isValid) { return v.toVariant().toInt(&isValid); }
inline bool bool_convertFromScriptValue(const ScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toBool(); }
inline uint8_t uint8_t_convertFromScriptValue(const ScriptValue& v, bool& isValid) { isValid = true; return (uint8_t)(0xff & v.toVariant().toInt(&isValid)); }
inline QString QString_convertFromScriptValue(const ScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toString().trimmed(); }
inline QUuid QUuid_convertFromScriptValue(const ScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toUuid(); }
inline EntityItemID EntityItemID_convertFromScriptValue(const ScriptValue& v, bool& isValid) { isValid = true; return v.toVariant().toUuid(); }

inline QByteArray QByteArray_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    QString b64 = v.toVariant().toString().trimmed();
    return QByteArray::fromBase64(b64.toUtf8());
}

inline glm::vec2 vec2_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    glm::vec2 vec2;
    vec2FromScriptValue(v, vec2);
    return vec2;
}

inline glm::vec3 vec3_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    glm::vec3 vec3;
    vec3FromScriptValue(v, vec3);
    return vec3;
}

inline glm::vec3 vec3Color_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    glm::vec3 vec3;
    vec3FromScriptValue(v, vec3);
    return vec3;
}

inline glm::u8vec3 u8vec3Color_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    glm::u8vec3 vec3;
    u8vec3FromScriptValue(v, vec3);
    return vec3;
}

inline AACube AACube_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    AACube result;
    aaCubeFromScriptValue(v, result);
    return result;
}

inline qVectorFloat qVectorFloat_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorFloatFromScriptValue(v);
}

inline qVectorVec3 qVectorVec3_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorVec3FromScriptValue(v);
}

inline qVectorQuat qVectorQuat_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorQuatFromScriptValue(v);
}

inline qVectorBool qVectorBool_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorBoolFromScriptValue(v);
}

inline qVectorQUuid qVectorQUuid_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorQUuidFromScriptValue(v);
}

inline qVectorQString qVectorQString_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    return qVectorQStringFromScriptValue(v);
}

inline glm::quat quat_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = false; /// assume it can't be converted
    ScriptValue x = v.property("x");
    ScriptValue y = v.property("y");
    ScriptValue z = v.property("z");
    ScriptValue w = v.property("w");
    if (x.isValid() && y.isValid() && z.isValid() && w.isValid()) {
        glm::quat newValue;
        newValue.x = x.toVariant().toFloat();
        newValue.y = y.toVariant().toFloat();
        newValue.z = z.toVariant().toFloat();
        newValue.w = w.toVariant().toFloat();
        isValid = !glm::isnan(newValue.x) &&
                  !glm::isnan(newValue.y) &&
                  !glm::isnan(newValue.z) &&
                  !glm::isnan(newValue.w);
        if (isValid) {
            return newValue;
        }
    }
    return glm::quat();
}

inline QRect QRect_convertFromScriptValue(const ScriptValue& v, bool& isValid) {
    isValid = true;
    QRect rect;
    qRectFromScriptValue(v, rect);
    return rect;
}

#define COPY_PROPERTY_IF_CHANGED(P) \
{                                   \
    if (other._##P##Changed) {      \
        _##P = other._##P;          \
    }                               \
}

#define COPY_PROPERTY_FROM_QSCRIPTVALUE(P, T, S)                         \
    {                                                                    \
        if (namesSet.contains(#P)) {                                     \
            ScriptValue V = object.property(#P);                         \
            if (V.isValid()) {                                           \
                bool isValid = false;                                    \
                T newValue = T##_convertFromScriptValue(V, isValid);     \
                if (isValid && (_defaultSettings || newValue != _##P)) { \
                    S(newValue);                                         \
                }                                                        \
            }                                                            \
        }                                                                \
    }

#define COPY_PROPERTY_FROM_QSCRIPTVALUE_GETTER(P, T, S, G)          \
{                                                                   \
    if (namesSet.contains(#P)) {                                    \
        ScriptValue V = object.property(#P);                        \
        if (V.isValid()) {                                          \
            bool isValid = false;                                   \
            T newValue = T##_convertFromScriptValue(V, isValid);    \
            if (isValid && (_defaultSettings || newValue != G())) { \
                S(newValue);                                        \
            }                                                       \
        }                                                           \
    }                                                               \
}

#define COPY_PROPERTY_FROM_QSCRIPTVALUE_NOCHECK(P, T, S)         \
{                                                                \
    if (namesSet.contains(#P)) {                                 \
        ScriptValue V = object.property(#P);                     \
        if (V.isValid()) {                                       \
            bool isValid = false;                                \
            T newValue = T##_convertFromScriptValue(V, isValid); \
            if (isValid && (_defaultSettings)) {                 \
                S(newValue);                                     \
            }                                                    \
        }                                                        \
    }                                                            \
}

#define COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(G, P, T, S)                    \
    {                                                                        \
        if (namesSet.contains(#G)) {                                         \
            ScriptValue G = object.property(#G);                             \
            if (G.isValid()) {                                               \
                ScriptValue V = G.property(#P);                              \
                if (V.isValid()) {                                           \
                    bool isValid = false;                                    \
                    T newValue = T##_convertFromScriptValue(V, isValid);     \
                    if (isValid && (_defaultSettings || newValue != _##P)) { \
                        S(newValue);                                         \
                    }                                                        \
                }                                                            \
            }                                                                \
        }                                                                    \
    }

#define COPY_PROPERTY_FROM_QSCRIPTVALUE_ENUM(P, S)                        \
    {                                                                     \
        if (namesSet.contains(#P)) {                                      \
            ScriptValue P = object.property(#P);                          \
            if (P.isValid()) {                                            \
                QString newValue = P.toVariant().toString();              \
                if (_defaultSettings || newValue != get##S##AsString()) { \
                    set##S##FromString(newValue);                         \
                }                                                         \
            }                                                             \
        }                                                                 \
    }

#define COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE_ENUM(G, P, S)                   \
    {                                                                         \
        if (namesSet.contains(#G)) {                                          \
            ScriptValue G = object.property(#G);                              \
            if (G.isValid()) {                                                \
                ScriptValue P = G.property(#P);                               \
                if (P.isValid()) {                                            \
                    QString newValue = P.toVariant().toString();              \
                    if (_defaultSettings || newValue != get##S##AsString()) { \
                        set##S##FromString(newValue);                         \
                    }                                                         \
                }                                                             \
            }                                                                 \
        }                                                                     \
    }

#define DEFINE_PROPERTY_GROUP(N, n, T)           \
    public:                                      \
        const T& get##N() const { return _##n; } \
        T& get##N() { return _##n; }             \
    private:                                     \
        T _##n;                                  \
        static T _static##N; 


#define ADD_PROPERTY_TO_MAP(P, n, T) \
    { \
        EntityPropertyInfo propertyInfo { makePropertyInfo<T>(P) }; \
        _propertyInfos[#n] = propertyInfo; \
		_enumsToPropertyStrings[P] = #n; \
    }

#define ADD_PROPERTY_TO_MAP_WITH_RANGE(P, n, M, X) \
    { \
        EntityPropertyInfo propertyInfo = EntityPropertyInfo(P, M, X); \
        _propertyInfos[#n] = propertyInfo; \
		_enumsToPropertyStrings[P] = #n; \
    }

#define ADD_GROUP_PROPERTY_TO_MAP(P, g, n) \
    { \
        EntityPropertyInfo propertyInfo = EntityPropertyInfo(P); \
        _propertyInfos[#g "." #n] = propertyInfo; \
        _propertyInfos[#g].propertyEnums << P; \
        _enumsToPropertyStrings[P] = #g "." #n; \
    }

#define ADD_GROUP_PROPERTY_TO_MAP_WITH_RANGE(P, g, n, M, X) \
    { \
        EntityPropertyInfo propertyInfo = EntityPropertyInfo(P, M, X); \
        _propertyInfos[#g "." #n] = propertyInfo; \
        _propertyInfos[#g].propertyEnums << P; \
        _enumsToPropertyStrings[P] = #g "." #n; \
    }

#define DEFINE_CORE(N, n, T, V) \
    public: \
        bool n##Changed() const { return _##n##Changed; } \
        void set##N##Changed(bool value) { _##n##Changed = value; } \
    private: \
        T _##n = V; \
        bool _##n##Changed { false };

#define DEFINE_PROPERTY(N, n, T, V)        \
    public: \
        T get##N() const { return _##n; } \
        void set##N(T value) { _##n = value; _##n##Changed = true; } \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF(N, n, T, V)        \
    public: \
        const T& get##N() const { return _##n; } \
        void set##N(const T& value) { _##n = value; _##n##Changed = true; } \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF_WITH_SETTER(N, n, T, V)        \
    public: \
        const T& get##N() const { return _##n; } \
        void set##N(const T& value); \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF_WITH_SETTER_AND_GETTER(N, n, T, V)        \
    public: \
        T get##N() const; \
        void set##N(const T& value); \
    DEFINE_CORE(N, n, T, V)

#define DEFINE_PROPERTY_REF_ENUM(N, n, T, V) \
    public: \
        const T& get##N() const { return _##n; } \
        void set##N(const T& value) { _##n = value; _##n##Changed = true; } \
        QString get##N##AsString() const; \
        void set##N##FromString(const QString& name); \
    DEFINE_CORE(N, n, T, V)

#define DEBUG_PROPERTY(D, P, N, n, x)                \
    D << "  " << #n << ":" << P.get##N() << x << "[changed:" << P.n##Changed() << "]\n";

#define DEBUG_PROPERTY_IF_CHANGED(D, P, N, n, x)                \
    if (P.n##Changed()) {                                       \
        D << "  " << #n << ":" << P.get##N() << x << "\n";      \
    }

// EntityItem helpers
#define DEFINE_VARIABLE_NO_GETTER_SETTER(N, n, T, V) \
    protected:                                       \
        T _##n = V;

#define DEFINE_VARIABLE(N, n, T, V)  \
    public:                          \
        T get##N() const;            \
        void set##N(T value);        \
    protected:                       \
        T _##n = V;

#define DEFINE_VARIABLE_REF(N, n, T, V)  \
    public:                              \
        T get##N() const;                \
        void set##N(const T& value);     \
    protected:                           \
        T _##n = V;

#define DEFINE_VARIABLE_BASIC(N, n, T, V)      \
    public:                                    \
        T get##N() const {                     \
            return resultWithReadLock<T>([&] { \
                return _##n;                   \
            });                                \
        }                                      \
        void set##N(T value) {                 \
            withWriteLock([&] {                \
                _##n = value;                  \
            });                                \
        }                                      \
    protected:                                 \
        T _##n = V;

#define DEFINE_VARIABLE_BASIC_REF(N, n, T, V)  \
    public:                                    \
        T get##N() const {                     \
            return resultWithReadLock<T>([&] { \
                return _##n;                   \
            });                                \
        }                                      \
        void set##N(const T& value) {          \
            withWriteLock([&] {                \
                _##n = value;                  \
            });                                \
        }                                      \
    protected:                                 \
        T _##n = V;

#define DEFINE_VARIABLE_RENDER(N, n, T, V)           \
    public:                                          \
        T get##N() const {                           \
            return resultWithReadLock<T>([&] {       \
                return _##n;                         \
            });                                      \
        }                                            \
        void set##N(T value) {                       \
            withWriteLock([&] {                      \
                _needsRenderUpdate |= _##n != value; \
                _##n = value;                        \
            });                                      \
        }                                            \
    protected:                                       \
        T _##n = V;

#define DEFINE_VARIABLE_RENDER_REF(N, n, T, V)       \
    public:                                          \
        T get##N() const {                           \
            return resultWithReadLock<T>([&] {       \
                return _##n;                         \
            });                                      \
        }                                            \
        void set##N(const T& value) {                \
            withWriteLock([&] {                      \
                _needsRenderUpdate |= _##n != value; \
                _##n = value;                        \
            });                                      \
        }                                            \
    protected:                                       \
        T _##n = V;

#define ENTITY_PROPERTY_SUBCLASS_METHODS                                                                \
    EntityItemProperties getProperties(const EntityPropertyFlags& desiredProperties,                    \
                                       bool allowEmptyDesiredProperties) const override;                \
    bool setSubClassProperties(const EntityItemProperties& properties) override;                        \
    EntityPropertyFlags getEntityProperties(EncodeBitstreamParams& params) const override;              \
    void appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,                \
                            EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,   \
                            EntityPropertyFlags& requestedProperties,                                   \
                            EntityPropertyFlags& propertyFlags,                                         \
                            EntityPropertyFlags& propertiesDidntFit,                                    \
                            int& propertyCount,                                                         \
                            OctreeElement::AppendState& appendState) const override;                    \
    int readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,                \
                                         ReadBitstreamToTreeParams& args,                               \
                                         EntityPropertyFlags& propertyFlags, bool overwriteLocalData,   \
                                         bool& somethingChanged) override;                              \
    virtual void debugDump() const override;

#define ENTITY_PROPERTY_GROUP_METHODS(P)                                                                    \
    virtual void copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties,   \
                                   ScriptEngine* engine, bool skipDefaults,                                 \
                                   EntityItemProperties& defaultEntityProperties,                           \
                                   bool returnNothingOnEmptyPropertyFlags,                                  \
                                   bool isMyOwnAvatarEntity) const override;                                \
    virtual void copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet,              \
                                     bool& _defaultSettings) override;                                      \
    void merge(const P& other);                                                                             \
    virtual void debugDump() const override;                                                                \
    virtual void listChangedProperties(QList<QString>& out) override;                                       \
    virtual bool appendToEditPacket(OctreePacketData* packetData,                                           \
                                    EntityPropertyFlags& requestedProperties,                               \
                                    EntityPropertyFlags& propertyFlags,                                     \
                                    EntityPropertyFlags& propertiesDidntFit,                                \
                                    int& propertyCount,                                                     \
                                    OctreeElement::AppendState& appendState) const override;                \
    virtual bool decodeFromEditPacket(EntityPropertyFlags& propertyFlags,                                   \
                                      const unsigned char*& dataAt, int& processedBytes) override;          \
    virtual void markAllChanged() override;                                                                 \
    virtual EntityPropertyFlags getChangedProperties() const override;                                      \
    virtual void getProperties(EntityItemProperties& propertiesOut) const override;                         \
    virtual bool setProperties(const EntityItemProperties& properties) override;                            \
    virtual EntityPropertyFlags getEntityProperties(EncodeBitstreamParams& params) const override;          \
    virtual int readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,            \
                                                 ReadBitstreamToTreeParams& args,                           \
                                                 EntityPropertyFlags& propertyFlags,                        \
                                                 bool overwriteLocalData, bool& somethingChanged) override; \
	static void addPropertyMap(QHash<QString, EntityPropertyInfo>& _propertyInfos,                          \
                               QHash<EntityPropertyList, QString>& _enumsToPropertyStrings);

#endif // hifi_EntityItemPropertiesMacros_h
