//
//  Created by Bradley Austin Davis on 2015/11/13
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "RecordingScriptingInterface.h"

#include <QStandardPaths>
#include <QtCore/QThread>
#include <QtCore/QUrl>
#include <QtWidgets/QFileDialog>

#include <shared/QtHelpers.h>
#include <AssetClient.h>
#include <AssetUpload.h>
#include <BuildInfo.h>
#include <NumericalConstants.h>
#include <PathUtils.h>
#include <Transform.h>
#include "Deck.h"
#include "Recorder.h"
#include "Clip.h"
#include "Frame.h"
#include "ClipCache.h"

#include <ScriptEngine.h>
#include <ScriptEngineLogging.h>
#include <ScriptManager.h>
#include <ScriptValue.h>

using namespace recording;

static const QString HFR_EXTENSION = "hfr";

RecordingScriptingInterface::RecordingScriptingInterface() {
    Locker(_mutex);
    _player = DependencyManager::get<Deck>();
    _recorder = DependencyManager::get<Recorder>();
}

bool RecordingScriptingInterface::isPlaying() const {
    Locker(_mutex);
    return _player->isPlaying();
}

bool RecordingScriptingInterface::isPaused() const {
    Locker(_mutex);
    return _player->isPaused();
}

float RecordingScriptingInterface::playerElapsed() const {
    Locker(_mutex);
    return _player->position();
}

float RecordingScriptingInterface::playerLength() const {
    Locker(_mutex);
    return _player->length();
}

void RecordingScriptingInterface::playClip(NetworkClipLoaderPointer clipLoader, const QString& url, const ScriptValue& callback) {
    Locker(_mutex);
    _player->queueClip(clipLoader->getClip());

    if (callback.isFunction()) {
        auto engine = callback.engine();
        ScriptValueList args{ engine->newValue(true), engine->newValue(url) };
        callback.call(ScriptValue(), args);
    }
}

void RecordingScriptingInterface::loadRecording(const QString& url, const ScriptValue& callback) {
    Locker(_mutex);

    auto clipLoader = DependencyManager::get<recording::ClipCache>()->getClipLoader(url);

    if (clipLoader->isLoaded()) {
        qCDebug(scriptengine) << "Recording already loaded from" << url;
        playClip(clipLoader, url, callback);
        return;
    }

    // hold a strong pointer to the loading clip so that it has a chance to load
    _clipLoaders.insert(clipLoader);

    auto weakClipLoader = clipLoader.toWeakRef();

    auto manager = callback.engine()->manager();
    if (!manager) {
        qWarning() << "This script does not belong to a ScriptManager";
        return;
    }

    // when clip loaded, call the callback with the URL and success boolean
    connect(clipLoader.data(), &recording::NetworkClipLoader::clipLoaded, manager,
            [this, weakClipLoader, url, callback]() mutable {

        if (auto clipLoader = weakClipLoader.toStrongRef()) {
            qCDebug(scriptengine) << "Loaded recording from" << url;

            playClip(clipLoader, url, callback);

            // drop our strong pointer to this clip so it is cleaned up
            _clipLoaders.remove(clipLoader);
        }
    });

    // when clip load fails, call the callback with the URL and failure boolean
    connect(clipLoader.data(), &recording::NetworkClipLoader::failed, manager,
            [this, weakClipLoader, url, callback](QNetworkReply::NetworkError error) mutable {
        qCDebug(scriptengine) << "Failed to load recording from\"" << url << '"';

        if (callback.isFunction()) {
            auto engine = callback.engine();
            ScriptValueList args{ engine->newValue(false), engine->newValue(url) };
            callback.call(ScriptValue(), args);
        }

        if (auto clipLoader = weakClipLoader.toStrongRef()) {
            // drop out strong pointer to this clip so it is cleaned up
            _clipLoaders.remove(clipLoader);
        }
    });
}

void RecordingScriptingInterface::startPlaying() {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "startPlaying");
        return;
    }

    Locker(_mutex);
    _player->play();
}

void RecordingScriptingInterface::setPlayerVolume(float volume) {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "setPlayerVolume", Q_ARG(float, volume));
        return;
    }

    Locker(_mutex);
    _player->setVolume(std::min(std::max(volume, 0.0f), 1.0f));
}

void RecordingScriptingInterface::setPlayerAudioOffset(float audioOffset) {
    // FIXME 
}

void RecordingScriptingInterface::setPlayerTime(float time) {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "setPlayerTime", Q_ARG(float, time));
        return;
    }
    Locker(_mutex);
    _player->seek(time);
}

void RecordingScriptingInterface::setPlayFromCurrentLocation(bool playFromCurrentLocation) {
    _playFromCurrentLocation = playFromCurrentLocation;
}

void RecordingScriptingInterface::setPlayerLoop(bool loop) {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "setPlayerLoop", Q_ARG(bool, loop));
        return;
    }

    Locker(_mutex);
    _player->loop(loop);
}

void RecordingScriptingInterface::setPlayerUseDisplayName(bool useDisplayName) {
    _useDisplayName = useDisplayName;
}

void RecordingScriptingInterface::setPlayerUseAttachments(bool useAttachments) {
    _useAttachments = useAttachments;
}

void RecordingScriptingInterface::setPlayerUseHeadModel(bool useHeadModel) {
    _useHeadModel = useHeadModel;
}

void RecordingScriptingInterface::setPlayerUseSkeletonModel(bool useSkeletonModel) {
    _useSkeletonModel = useSkeletonModel;
}

void RecordingScriptingInterface::pausePlayer() {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "pausePlayer");
        return;
    }
    Locker(_mutex);
    _player->pause();
}

void RecordingScriptingInterface::stopPlaying() {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "stopPlaying");
        return;
    }
    Locker(_mutex);
    _player->stop();
}

bool RecordingScriptingInterface::isRecording() const {
    return _recorder->isRecording();
}

float RecordingScriptingInterface::recorderElapsed() const {
    return _recorder->position();
}

void RecordingScriptingInterface::startRecording() {
    if (_recorder->isRecording()) {
        qCWarning(scriptengine) << "Recorder is already running";
        return;
    }

    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "startRecording");
        return;
    }

    Locker(_mutex);
    _recorder->start();
}

void RecordingScriptingInterface::stopRecording() {
    if (!_recorder->isRecording()) {
        qCWarning(scriptengine) << "Recorder is not running";
        return;
    }

    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "stopRecording");
        return;
    }

    Locker(_mutex);
    _recorder->stop();
    _lastClip = _recorder->getClip();
    _lastClip->seek(0);
}

QString RecordingScriptingInterface::getDefaultRecordingSaveDirectory() {
    QString directory = PathUtils::getAppLocalDataPath() + "Avatar Recordings/";
    if (!QDir(directory).exists()) {
        QDir().mkdir(directory);
    }
    return directory;
}

void RecordingScriptingInterface::saveRecording(const QString& filename) {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "saveRecording",
            Q_ARG(QString, filename));
        return;
    }

    Locker(_mutex);
    if (!_lastClip) {
        qWarning() << "There is no recording to save";
        return;
    }

    recording::Clip::toFile(filename, _lastClip);
}

bool RecordingScriptingInterface::saveRecordingToAsset(const ScriptValue& getClipAtpUrl) {
    if (!getClipAtpUrl.isFunction()) {
        qCWarning(scriptengine) << "The argument is not a function.";
        return false;
    }

    Locker(_mutex);
    if (!_lastClip) {
        qWarning() << "There is no recording to save";
        return false;
    }

    auto manager = getClipAtpUrl.engine()->manager();
    if (!manager) {
        qWarning() << "This script does not belong to a ScriptManager";
        return false;
    }

    if (auto upload = DependencyManager::get<AssetClient>()->createUpload(recording::Clip::toBuffer(_lastClip))) {
        QObject::connect(upload, &AssetUpload::finished, manager, [=](AssetUpload* upload, const QString& hash) mutable {
            QString clip_atp_url = "";

            if (upload->getError() == AssetUpload::NoError) {

                clip_atp_url = QString("%1:%2").arg(URL_SCHEME_ATP, hash);
                upload->deleteLater();
            } else {
                qCWarning(scriptengine) << "Error during the Asset upload.";
            }

            ScriptValueList args;
            args << getClipAtpUrl.engine()->newValue(clip_atp_url);
            getClipAtpUrl.call(ScriptValue(), args);
        });
        upload->start();
        return true;
    }

    qCWarning(scriptengine) << "Saving on asset failed.";
    return false;
}

void RecordingScriptingInterface::loadLastRecording() {
    if (QThread::currentThread() != thread()) {
        BLOCKING_INVOKE_METHOD(this, "loadLastRecording");
        return;
    }

    Locker(_mutex);

    if (!_lastClip) {
        qCDebug(scriptengine) << "There is no recording to load";
        return;
    }

    _player->queueClip(_lastClip);
    _player->play();
}

