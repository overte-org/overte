//
//  SettingManager.h
//
//
//  Created by Clement on 2/2/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SettingManager_h
#define hifi_SettingManager_h

#include <QtCore/QPointer>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QUuid>

#include <QSettings>
#include <QThread>

#include "DependencyManager.h"
#include "shared/ReadWriteLockable.h"


// This is for the testing system.
class SettingsTests;
class SettingsTestsWorker;



namespace Setting {
    class Interface;

    /**
     * @brief Settings write worker
     *
     * This class is used by Setting::Manager to write settings to permanent storage
     * without blocking anything else. It receives setting updates, and writes them
     * to disk whenever convenient.
     *
     * All communication to this class must be done over queued connections.
     *
     * This class is purely an implementation detail and shouldn't be used outside of Setting::Manager.
     *
     */
    class WriteWorker : public QObject {
        Q_OBJECT

        public slots:

        /**
         * @brief Initialize anything that needs initializing, called on thread start.
         *
         */
        void start();

        /**
         * @brief Sets a configuration value
         *
         * @param key Configuration key
         * @param value Configuration value
         */
        void setValue(const QString &key, const QVariant &value);

        /**
         * @brief Remove a value from the configuration
         *
         * @param key Key to remove
         */
        void removeKey(const QString &key);

        /**
         * @brief Force writing the config to disk
         *
         */
        void sync();

        private:

        void init() {
            if (!_qSettings) {
                _qSettings = new QSettings();
            }
        }

        QSettings* _qSettings = nullptr;
    };

    /**
     * @brief Settings manager
     *
     * This class is the main implementation of the settings system, and the container
     * of the current configuration.
     *
     * Most users should either use the Setting::Handle or the Settings classes instead,
     * both of which talk to the single global instance of this class.
     *
     * The class is thread-safe, and delegates config writing to a separate thread. It
     * is safe to change settings as often as it might be needed.
     *
     */
    class Manager : public QObject, public ReadWriteLockable, public Dependency {
        Q_OBJECT

    public:
        Manager(QObject *parent = nullptr);

        void customDeleter() override;

        QString fileName() const;
        void remove(const QString &key);
        QStringList allKeys() const;
        bool contains(const QString &key) const;
        void setValue(const QString &key, const QVariant &value);
        QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    protected:
        ~Manager();
        void registerHandle(Interface* handle);
        void removeHandle(const QString& key);

        void loadSetting(Interface* handle);
        void saveSetting(Interface* handle);
        void forceSave();

    signals:
        void valueChanged(const QString &key, const QVariant &value);
        void keyRemoved(const QString &key);
        void syncRequested();

    private:
        QHash<QString, Interface*> _handles;
        const QVariant UNSET_VALUE { QUuid::createUuid() };


        friend class Interface;
        friend class ::SettingsTests;
        friend class ::SettingsTestsWorker;

        friend void cleanupSettingsSaveThread();
        friend void setupSettingsSaveThread();


        QHash<QString, QVariant> _settings;
        QString _fileName;
        QThread _workerThread;
    };
}

#endif // hifi_SettingManager_h
