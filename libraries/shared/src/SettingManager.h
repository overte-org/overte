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
#include <QLoggingCategory>

#include "DependencyManager.h"
#include "shared/ReadWriteLockable.h"

Q_DECLARE_LOGGING_CATEGORY(settings_manager)
Q_DECLARE_LOGGING_CATEGORY(settings_writer)

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
        void setValue(const QString key, const QVariant value);

        /**
         * @brief Remove a value from the configuration
         *
         * @param key Key to remove
         */
        void removeKey(const QString key);

        /**
         * @brief Force writing the config to disk
         *
         */
        void sync();

        /**
         * @brief Called when the thread is terminating
         *
         */
        void threadFinished();

        /**
         * @brief Thread is being asked to finish work and quit
         *
         */
        void terminate();

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

        /**
         * @brief Returns the filename where the config file will be written
         *
         * @return QString Path to the config file
         */
        QString fileName() const;

        /**
         * @brief Remove a configuration key
         *
         * @param key Key to remove
         */
        void remove(const QString &key);

        /**
         * @brief Lists all keys in the configuration
         *
         * @return QStringList List of keys
         */
        QStringList allKeys() const;

        /**
         * @brief Returns whether a key is part of the configuration
         *
         * @param key Key to look for
         * @return true Key is in the configuration
         * @return false Key isn't in the configuration
         */
        bool contains(const QString &key) const;

        /**
         * @brief Set a setting to a value
         *
         * @param key Setting to set
         * @param value Value
         */
        void setValue(const QString &key, const QVariant &value);

        /**
         * @brief Returns the value of a setting
         *
         * @param key Setting to look for
         * @param defaultValue Default value to return, if the setting has no value
         * @return QVariant Current value of the setting, of defaultValue.
         */
        QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    protected:
        /**
         * @brief How long to wait for writer thread termination
         *
         * We probably want a timeout here since we don't want to block shutdown indefinitely in case of
         * any weirdness.
         */
        const int THREAD_TERMINATION_TIMEOUT = 2000;

        ~Manager();
        void registerHandle(Interface* handle);
        void removeHandle(const QString& key);

        void loadSetting(Interface* handle);
        void saveSetting(Interface* handle);


        /**
         * @brief Force saving the config to disk.
         *
         * Normally unnecessary to use. Asynchronous.
         */
        void forceSave();

        /**
         * @brief Write config to disk and terminate the writer thread
         *
         * This is part of the shutdown process.
         */
        void terminateThread();

    signals:
        void valueChanged(const QString key, QVariant value);
        void keyRemoved(const QString key);
        void syncRequested();
        void terminationRequested();

    private:
        QHash<QString, Interface*> _handles;

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
