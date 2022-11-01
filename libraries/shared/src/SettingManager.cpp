//
//  SettingManager.cpp
//
//
//  Created by Clement on 2/2/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SettingManager.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QtCore/QUuid>

#include "SettingInterface.h"

Q_LOGGING_CATEGORY(settings_manager, "settings.manager")
Q_LOGGING_CATEGORY(settings_writer, "settings.manager.writer")

namespace Setting {


    void WriteWorker::start() {
        // QSettings seems to have some issues with moving to a new thread.
        // Make sure the object is only created once the thread is already up and running.
        init();
    }

    void WriteWorker::setValue(const QString key, const QVariant value) {
       //qCDebug(settings_writer) << "Setting config " << key << "to" << value;

        init();

        if (!_qSettings->contains(key) || _qSettings->value(key) != value) {
            _qSettings->setValue(key, value);
        }
    }

    void WriteWorker::removeKey(const QString key) {
        init();
        _qSettings->remove(key);
    }

    void WriteWorker::sync() {
        //qCDebug(settings_writer) << "Forcing settings sync";
        init();
        _qSettings->sync();
    }

    void WriteWorker::threadFinished() {
        qCDebug(settings_writer) << "Settings write worker syncing and terminating";
        sync();
        this->deleteLater();
    }

    void WriteWorker::terminate() {
        qCDebug(settings_writer) << "Settings write worker being asked to terminate. Syncing and terminating.";
        sync();
        this->deleteLater();
        QThread::currentThread()->exit(0);
    }

    Manager::Manager(QObject *parent) {
        WriteWorker *worker = new WriteWorker();

        // We operate purely from memory, and forward all changes to a thread that has writing the
        // settings as its only job.

        qCDebug(settings_manager) << "Initializing settings write thread";

        _workerThread.setObjectName("Settings Writer");
        worker->moveToThread(&_workerThread);
        // connect(&_workerThread, &QThread::started, worker, &WriteWorker::start, Qt::QueuedConnection);

        // All normal connections are queued, so that we're sure they happen asynchronously.
        connect(&_workerThread, &QThread::finished, worker, &WriteWorker::threadFinished, Qt::QueuedConnection);
        connect(this, &Manager::valueChanged, worker, &WriteWorker::setValue, Qt::QueuedConnection);
        connect(this, &Manager::keyRemoved, worker, &WriteWorker::removeKey, Qt::QueuedConnection);
        connect(this, &Manager::syncRequested, worker, &WriteWorker::sync, Qt::QueuedConnection);

        // This one is blocking because we want to wait until it's actually processed.
        connect(this, &Manager::terminationRequested, worker, &WriteWorker::terminate, Qt::BlockingQueuedConnection);


        _workerThread.start();

        // Load all current settings
        QSettings settings;
        _fileName = settings.fileName();

        for(QString key : settings.allKeys()) {
            //qCDebug(settings_manager) << "Loaded key" << key << "with value" << settings.value(key);
            _settings[key] = settings.value(key);
        }
    }


    Manager::~Manager() {

    }

    // Custom deleter does nothing, because we need to shutdown later than the dependency manager
    void Manager::customDeleter() { }

    void Manager::registerHandle(Interface* handle) {
        const QString& key = handle->getKey();
        withWriteLock([&] {
            if (_handles.contains(key)) {
                qCWarning(settings_manager) << "Setting::Manager::registerHandle(): Key registered more than once, overriding: " << key;
            }
            _handles.insert(key, handle);
        });
    }

    void Manager::removeHandle(const QString& key) {
        withWriteLock([&] {
            _handles.remove(key);
        });
    }

    void Manager::loadSetting(Interface* handle) {
        const auto& key = handle->getKey();

        withWriteLock([&] {
            QVariant loadedValue = _settings[key];

            if (loadedValue.isValid()) {
                handle->setVariant(loadedValue);
            }
        });
    }


    void Manager::saveSetting(Interface* handle) {
        const auto& key = handle->getKey();

        if (handle->isSet()) {
            QVariant handleValue = handle->getVariant();

            withWriteLock([&] {
                _settings[key] = handleValue;
            });

            emit valueChanged(key, handleValue);
        } else {
            withWriteLock([&] {
                _settings.remove(key);
            });

            emit keyRemoved(key);
        }

    }


    /**
     * @brief Forces saving the current configuration
     *
     * @warning This function is for testing only, should only be called from the test suite.
     */
    void Manager::forceSave() {
        emit syncRequested();
    }

    void Manager::terminateThread() {
        qCDebug(settings_manager) << "Terminating settings writer thread";

        emit terminationRequested(); // This blocks

        _workerThread.exit();
        _workerThread.wait(THREAD_TERMINATION_TIMEOUT);
        qCDebug(settings_manager) << "Settings writer terminated";
    }

    QString Manager::fileName() const {
        return resultWithReadLock<QString>([&] {
            return _fileName;
        });
    }

    void Manager::remove(const QString &key) {
        withWriteLock([&] {
            _settings.remove(key);
        });

        emit keyRemoved(key);
    }

    QStringList Manager::allKeys() const {
        return resultWithReadLock<QStringList>([&] {
            return _settings.keys();
        });
    }

    bool Manager::contains(const QString &key) const {
        return resultWithReadLock<bool>([&] {
            return _settings.contains(key);
        });
    }

    void Manager::setValue(const QString &key, const QVariant &value) {
        withWriteLock([&] {
            _settings[key] = value;
        });

        emit valueChanged(key, value);
    }

    QVariant Manager::value(const QString &key, const QVariant &defaultValue) const {
        return resultWithReadLock<QVariant>([&] {
            return _settings.value(key, defaultValue);
        });
    }
}
