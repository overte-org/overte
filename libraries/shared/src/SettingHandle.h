//
//  SettingHandle.h
//
//
//  Created by Clement on 1/18/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SettingHandle_h
#define hifi_SettingHandle_h

#include <type_traits>

#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSharedPointer>
#include <QtCore/QDebug>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SettingInterface.h"


/**
 * @brief QSettings analog
 *
 * This class emulates the interface of QSettings, and forwards all reads and writes to the global Setting::Manager.
 *
 * It should be used in the same manner as QSettings -- created only wherever it happens to be needed.
 * It's not thread safe, and each thread should have their own.
 *
 * Unlike QSettings, destruction doesn't cause the config to be saved, instead Config::Manager is in
 * charge of taking care of that.
 *
 * @note childGroups and childKeys are unimplemented because nothing in the code needs them so far.
 */
class Settings {
public:

    class Group {
        public:

        Group(const QString &groupName = QString()) {
            _name = groupName;
        }

        QString name() const { return _name; }

        bool isArray() const { return _arraySize != -1; }

        void setIndex(int i) {
            if ( _arraySize < i+1) {
                _arraySize = i+1;
            }

            _arrayIndex = i;
        }

        int index() const { return _arrayIndex; }
        int size() const { return _arraySize; }
        void setSize(int sz) { _arraySize = sz; }

        private:

        QString _name;
        int _arrayIndex{0};
        int _arraySize{-1};
    };

    static const QString firstRun;
    Settings();

    QString fileName() const;

    void remove(const QString& key);

    // These are not currently being used
    // QStringList childGroups() const;
    // QStringList childKeys() const;

    QStringList allKeys() const;
    bool contains(const QString& key) const;
    int    beginReadArray(const QString & prefix);
    void beginWriteArray(const QString& prefix, int size = -1);
    void endArray();
    void setArrayIndex(int i);

    void beginGroup(const QString& prefix);
    void endGroup();

    void setValue(const QString& name, const QVariant& value);
    QVariant value(const QString& name, const QVariant& defaultValue = QVariant()) const;

    void getFloatValueIfValid(const QString& name, float& floatValue);
    void getBoolValue(const QString& name, bool& boolValue);

    void setVec3Value(const QString& name, const glm::vec3& vecValue);
    void getVec3ValueIfValid(const QString& name, glm::vec3& vecValue);

    void setQuatValue(const QString& name, const glm::quat& quatValue);
    void getQuatValueIfValid(const QString& name, glm::quat& quatValue);

private:
    QString getGroupPrefix() const;
    QString getPath(const QString &value) const;

    QSharedPointer<Setting::Manager> _manager;
    QStack<Group> _groups;
    QString _groupPrefix;
};

namespace Setting {
    template <typename T>
    class Handle : public Interface {
    public:
        Handle(const QString& key) : Interface(key) {}
        Handle(const QStringList& path) : Interface(path.join("/")) {}

        Handle(const QString& key, const T& defaultValue) : Interface(key), _defaultValue(defaultValue) {}
        Handle(const QStringList& path, const T& defaultValue) : Handle(path.join("/"), defaultValue) {}

        static Handle Deprecated(const QString& key) {
            Handle handle = Handle(key);
            handle.deprecate();
            return handle;
        }
        static Handle Deprecated(const QStringList& path) {
            return Deprecated(path.join("/"));
        }

        static Handle Deprecated(const QString& key, const T& defaultValue) {
            Handle handle = Handle(key, defaultValue);
            handle.deprecate();
            return handle;
        }
        static Handle Deprecated(const QStringList& path, const T& defaultValue) {
            return Deprecated(path.join("/"), defaultValue);
        }

        virtual ~Handle() {
            deinit();
        }

        // Returns setting value, returns its default value if not found
        T get() const {
            return get(_defaultValue);
        }

        // Returns setting value, returns other if not found
        T get(const T& other) const {
            maybeInit();
            return (_isSet) ? _value : other;
        }

        bool isSet() const {
            maybeInit();
            return _isSet;
        }

        const T& getDefault() const {
            return _defaultValue;
        }

        void reset() {
            set(_defaultValue);
        }

        void set(const T& value) {
            maybeInit();
            if ((!_isSet && (value != _defaultValue)) || _value != value) {
                _value = value;
                _isSet = true;
                save();
            }
            if (_isDeprecated) {
                deprecate();
            }
        }

        void remove() {
            maybeInit();
            if (_isSet) {
                _isSet = false;
                save();
            }
        }

    protected:
        virtual void setVariant(const QVariant& variant) override;
        virtual QVariant getVariant() override { return QVariant::fromValue(get()); }

    private:
        void deprecate() {
            if (_isSet) {
                if (get() != getDefault()) {
                    qInfo().nospace() << "[DEPRECATION NOTICE] " << _key << "(" << get() << ") has been deprecated, and has no effect";
                } else {
                    remove();
                }
            }
            _isDeprecated = true;
        }

        T _value;
        const T _defaultValue;
        bool _isDeprecated{ false };
    };

    template <typename T>
    void Handle<T>::setVariant(const QVariant& variant) {
        if (variant.canConvert<T>() || std::is_same<T, QVariant>::value) {
            set(variant.value<T>());
        }
    }
}

#endif // hifi_SettingHandle_h
