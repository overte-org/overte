//
//  Application_Assets.cpp
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Application.h"

#include <QQuickItem>
#include <QTemporaryDir>

#include <AddressManager.h>
#include <AssetUpload.h>
#include <MappingRequest.h>

#include "ArchiveDownloadInterface.h"
#include "InterfaceLogging.h"
#include "Menu.h"
#include "ModelPackager.h"

static const QString SVO_EXTENSION = ".svo";
static const QString SVO_JSON_EXTENSION = ".svo.json";
static const QString JSON_EXTENSION = ".json";
static const QString JS_EXTENSION = ".js";
static const QString FST_EXTENSION = ".fst";
static const QString FBX_EXTENSION = ".fbx";
static const QString OBJ_EXTENSION = ".obj";
static const QString JSON_GZ_EXTENSION = ".json.gz";
static const QString CONTENT_ZIP_EXTENSION = ".content.zip";
static const QString ZIP_EXTENSION = ".zip";
static const QString JPG_EXTENSION = ".jpg";
static const QString PNG_EXTENSION = ".png";

static const QString WEB_VIEW_TAG = "noDownload=true";

const std::vector<std::pair<QString, Application::AcceptURLMethod>> Application::_acceptedExtensions {
    { SVO_EXTENSION, &Application::importSVOFromURL },
    { SVO_JSON_EXTENSION, &Application::importSVOFromURL },
    { JSON_EXTENSION, &Application::importJSONFromURL },
    { JS_EXTENSION, &Application::askToLoadScript },
    { FST_EXTENSION, &Application::askToSetAvatarUrl },
    { JSON_GZ_EXTENSION, &Application::askToReplaceDomainContent },
    { CONTENT_ZIP_EXTENSION, &Application::askToReplaceDomainContent },
    { ZIP_EXTENSION, &Application::importFromZIP },
    { JPG_EXTENSION, &Application::importImage },
    { PNG_EXTENSION, &Application::importImage }
};

bool Application::canAcceptURL(const QString& urlString) const {
    QUrl url(urlString);
    if (url.query().contains(WEB_VIEW_TAG)) {
        return false;
    } else if (urlString.startsWith(URL_SCHEME_OVERTE)) {
        return true;
    }
    QString lowerPath = url.path().toLower();
    for (auto& pair : _acceptedExtensions) {
        if (lowerPath.endsWith(pair.first, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool Application::acceptURL(const QString& urlString, bool defaultUpload) {
    QUrl url(urlString);

    if (url.scheme() == URL_SCHEME_OVERTE) {
        // this is a hifi URL - have the AddressManager handle it
        QMetaObject::invokeMethod(DependencyManager::get<AddressManager>().data(), "handleLookupString",
                                  Qt::AutoConnection, Q_ARG(const QString&, urlString));
        return true;
    }

    QString lowerPath = url.path().toLower();
    for (auto& pair : _acceptedExtensions) {
        if (lowerPath.endsWith(pair.first, Qt::CaseInsensitive)) {
            AcceptURLMethod method = pair.second;
            return (this->*method)(urlString);
        }
    }

    if (defaultUpload && !url.fileName().isEmpty() && url.isLocalFile()) {
        showAssetServerWidget(urlString);
    }
    return defaultUpload;
}

void Application::addAssetToWorldFromURL(QString url) {

    QString filename;
    if (url.contains("filename")) {
        filename = url.section("filename=", 1, 1);  // Filename is in "?filename=" parameter at end of URL.
    }
    if (url.contains("poly.google.com/downloads")) {
        filename = url.section('/', -1);
        if (url.contains("noDownload")) {
            filename.remove(".zip?noDownload=false");
        } else {
            filename.remove(".zip");
        }

    }

    if (!DependencyManager::get<NodeList>()->getThisNodeCanWriteAssets()) {
        QString errorInfo = "You do not have permissions to write to the Asset Server.";
        qWarning(interfaceapp) << "Error downloading model: " + errorInfo;
        addAssetToWorldError(filename, errorInfo);
        return;
    }

    addAssetToWorldInfo(filename, "Downloading model file " + filename + ".");

    auto request = DependencyManager::get<ResourceManager>()->createResourceRequest(
        nullptr, QUrl(url), true, -1, "Application::addAssetToWorldFromURL");
    connect(request, &ResourceRequest::finished, this, &Application::addAssetToWorldFromURLRequestFinished);
    request->send();
}

void Application::addAssetToWorldFromURLRequestFinished() {
    auto request = qobject_cast<ResourceRequest*>(sender());
    Q_ASSERT(request != nullptr);
    auto url = request->getUrl().toString();
    auto result = request->getResult();

    QString filename;
    bool isBlocks = false;

    if (url.contains("filename")) {
        filename = url.section("filename=", 1, 1);  // Filename is in "?filename=" parameter at end of URL.
    }
    if (url.contains("poly.google.com/downloads")) {
        filename = url.section('/', -1);
        if (url.contains("noDownload")) {
            filename.remove(".zip?noDownload=false");
        } else {
            filename.remove(".zip");
        }
        isBlocks = true;
    }

    if (result == ResourceRequest::Success) {
        QTemporaryDir temporaryDir;
        temporaryDir.setAutoRemove(false);
        if (temporaryDir.isValid()) {
            QString temporaryDirPath = temporaryDir.path();
            QString downloadPath = temporaryDirPath + "/" + filename;

            QFile tempFile(downloadPath);
            if (tempFile.open(QIODevice::WriteOnly)) {
                tempFile.write(request->getData());
                addAssetToWorldInfoClear(filename);  // Remove message from list; next one added will have a different key.
                tempFile.close();
                qApp->getFileDownloadInterface()->runUnzip(downloadPath, url, true, false, isBlocks);
            } else {
                QString errorInfo = "Couldn't open temporary file for download";
                qWarning(interfaceapp) << errorInfo;
                addAssetToWorldError(filename, errorInfo);
            }
        } else {
            QString errorInfo = "Couldn't create temporary directory for download";
            qWarning(interfaceapp) << errorInfo;
            addAssetToWorldError(filename, errorInfo);
        }
    } else {
        qWarning(interfaceapp) << "Error downloading" << url << ":" << request->getResultString();
        addAssetToWorldError(filename, "Error downloading " + filename + " : " + request->getResultString());
    }

    request->deleteLater();
}

QString filenameFromPath(QString filePath) {
    return filePath.right(filePath.length() - filePath.lastIndexOf("/") - 1);
}

void Application::addAssetToWorld(QString path, QString zipFile, bool isZip, bool isBlocks) {
    // Automatically upload and add asset to world as an alternative manual process initiated by showAssetServerWidget().
    QString mapping;
    QString filename = filenameFromPath(path);
    if (isZip || isBlocks) {
        QString assetName = zipFile.section("/", -1).remove(QRegExp("[.]zip(.*)$"));
        QString assetFolder = path.section("model_repo/", -1);
        mapping = "/" + assetName + "/" + assetFolder;
    } else {
        mapping = "/" + filename;
    }

    // Test repeated because possibly different code paths.
    if (!DependencyManager::get<NodeList>()->getThisNodeCanWriteAssets()) {
        QString errorInfo = "You do not have permissions to write to the Asset Server.";
        qWarning(interfaceapp) << "Error downloading model: " + errorInfo;
        addAssetToWorldError(filename, errorInfo);
        return;
    }

    addAssetToWorldInfo(filename, "Adding " + mapping.mid(1) + " to the Asset Server.");

    addAssetToWorldWithNewMapping(path, mapping, 0, isZip, isBlocks);
}

void Application::addAssetToWorldUnzipFailure(QString filePath) {
    QString filename = filenameFromPath(QUrl(filePath).toLocalFile());
    qWarning(interfaceapp) << "Couldn't unzip file" << filePath;
    addAssetToWorldError(filename, "Couldn't unzip file " + filename + ".");
}

void Application::addAssetToWorldWithNewMapping(QString filePath, QString mapping, int copy, bool isZip, bool isBlocks) {
    auto request = DependencyManager::get<AssetClient>()->createGetMappingRequest(mapping);

    QObject::connect(request, &GetMappingRequest::finished, this, [=](GetMappingRequest* request) mutable {
        const int MAX_COPY_COUNT = 100;  // Limit number of duplicate assets; recursion guard.
        auto result = request->getError();
        if (result == GetMappingRequest::NotFound) {
            addAssetToWorldUpload(filePath, mapping, isZip, isBlocks);
        } else if (result != GetMappingRequest::NoError) {
            QString errorInfo = "Could not map asset name: "
                + mapping.left(mapping.length() - QString::number(copy).length() - 1);
            qWarning(interfaceapp) << "Error downloading model: " + errorInfo;
            addAssetToWorldError(filenameFromPath(filePath), errorInfo);
        } else if (copy < MAX_COPY_COUNT - 1) {
            if (copy > 0) {
                mapping = mapping.remove(mapping.lastIndexOf("-"), QString::number(copy).length() + 1);
            }
            copy++;
            mapping = mapping.insert(mapping.lastIndexOf("."), "-" + QString::number(copy));
            addAssetToWorldWithNewMapping(filePath, mapping, copy, isZip, isBlocks);
        } else {
            QString errorInfo = "Too many copies of asset name: "
                + mapping.left(mapping.length() - QString::number(copy).length() - 1);
            qWarning(interfaceapp) << "Error downloading model: " + errorInfo;
            addAssetToWorldError(filenameFromPath(filePath), errorInfo);
        }
        request->deleteLater();
    });

    request->start();
}

void Application::addAssetToWorldUpload(QString filePath, QString mapping, bool isZip, bool isBlocks) {
    qInfo(interfaceapp) << "Uploading" << filePath << "to Asset Server as" << mapping;
    auto upload = DependencyManager::get<AssetClient>()->createUpload(filePath);
    QObject::connect(upload, &AssetUpload::finished, this, [=](AssetUpload* upload, const QString& hash) mutable {
        if (upload->getError() != AssetUpload::NoError) {
            QString errorInfo = "Could not upload model to the Asset Server.";
            qWarning(interfaceapp) << "Error downloading model: " + errorInfo;
            addAssetToWorldError(filenameFromPath(filePath), errorInfo);
        } else {
            addAssetToWorldSetMapping(filePath, mapping, hash, isZip, isBlocks);
        }

        // Remove temporary directory created by Clara.io market place download.
        int index = filePath.lastIndexOf("/model_repo/");
        if (index > 0) {
            QString tempDir = filePath.left(index);
            qCDebug(interfaceapp) << "Removing temporary directory at: " + tempDir;
            QDir(tempDir).removeRecursively();
        }

        upload->deleteLater();
    });

    upload->start();
}

void Application::addAssetToWorldSetMapping(QString filePath, QString mapping, QString hash, bool isZip, bool isBlocks) {
    auto request = DependencyManager::get<AssetClient>()->createSetMappingRequest(mapping, hash);
    connect(request, &SetMappingRequest::finished, this, [=](SetMappingRequest* request) mutable {
        if (request->getError() != SetMappingRequest::NoError) {
            QString errorInfo = "Could not set asset mapping.";
            qWarning(interfaceapp) << "Error downloading model: " + errorInfo;
            addAssetToWorldError(filenameFromPath(filePath), errorInfo);
        } else {
            // to prevent files that aren't models or texture files from being loaded into world automatically
            if ((filePath.toLower().endsWith(OBJ_EXTENSION) || filePath.toLower().endsWith(FBX_EXTENSION)) ||
                ((filePath.toLower().endsWith(JPG_EXTENSION) || filePath.toLower().endsWith(PNG_EXTENSION)) &&
                ((!isBlocks) && (!isZip)))) {
                addAssetToWorldAddEntity(filePath, mapping);
            } else {
                qCDebug(interfaceapp) << "Zipped contents are not supported entity files";
                addAssetToWorldInfoDone(filenameFromPath(filePath));
            }
        }
        request->deleteLater();
    });

    request->start();
}

void Application::addAssetToWorldAddEntity(QString filePath, QString mapping) {
    EntityItemProperties properties;
    properties.setName(mapping.right(mapping.length() - 1));
    if (filePath.toLower().endsWith(PNG_EXTENSION) || filePath.toLower().endsWith(JPG_EXTENSION)) {
        properties.setType(EntityTypes::Image);
        properties.setImageURL(QString("atp:" + mapping));
        properties.setKeepAspectRatio(false);
    } else {
        properties.setType(EntityTypes::Model);
        properties.setModelURL("atp:" + mapping);
        properties.setShapeType(SHAPE_TYPE_SIMPLE_COMPOUND);
    }
    properties.setCollisionless(true);  // Temporarily set so that doesn't collide with avatar.
    properties.setVisible(false);  // Temporarily set so that don't see at large unresized dimensions.
    bool grabbable = (Menu::getInstance()->isOptionChecked(MenuOption::CreateEntitiesGrabbable));
    properties.setUserData(grabbable ? GRABBABLE_USER_DATA : NOT_GRABBABLE_USER_DATA);
    glm::vec3 positionOffset = getMyAvatar()->getWorldOrientation() * (getMyAvatar()->getSensorToWorldScale() * glm::vec3(0.0f, 0.0f, -2.0f));
    properties.setPosition(getMyAvatar()->getWorldPosition() + positionOffset);
    properties.setRotation(getMyAvatar()->getWorldOrientation());
    properties.setGravity(glm::vec3(0.0f, 0.0f, 0.0f));
    auto entityID = DependencyManager::get<EntityScriptingInterface>()->addEntity(properties);

    // Note: Model dimensions are not available here; model is scaled per FBX mesh in RenderableModelEntityItem::update() later
    // on. But FBX dimensions may be in cm, so we monitor for the dimension change and rescale again if warranted.

    if (entityID == QUuid()) {
        QString errorInfo = "Could not add model " + mapping + " to world.";
        qWarning(interfaceapp) << "Could not add model to world: " + errorInfo;
        addAssetToWorldError(filenameFromPath(filePath), errorInfo);
    } else {
        // Monitor when asset is rendered in world so that can resize if necessary.
        _addAssetToWorldResizeList.insert(entityID, 0);  // List value is count of checks performed.
        if (!_addAssetToWorldResizeTimer.isActive()) {
            _addAssetToWorldResizeTimer.start();
        }

        // Close progress message box.
        addAssetToWorldInfoDone(filenameFromPath(filePath));
    }
}

void Application::handleUnzip(QString zipFile, QStringList unzipFile, bool autoAdd, bool isZip, bool isBlocks) {
    if (autoAdd) {
        if (!unzipFile.isEmpty()) {
            for (int i = 0; i < unzipFile.length(); i++) {
                if (QFileInfo(unzipFile.at(i)).isFile()) {
                    qCDebug(interfaceapp) << "Preparing file for asset server: " << unzipFile.at(i);
                    addAssetToWorld(unzipFile.at(i), zipFile, isZip, isBlocks);
                }
            }
        } else {
            addAssetToWorldUnzipFailure(zipFile);
        }
    } else {
        showAssetServerWidget(unzipFile.first());
    }
}

void Application::packageModel() {
    ModelPackager::package();
}

void Application::addAssetToWorldCheckModelSize() {
    if (_addAssetToWorldResizeList.size() == 0) {
        return;
    }

    auto item = _addAssetToWorldResizeList.begin();
    while (item != _addAssetToWorldResizeList.end()) {
        auto entityID = item.key();

        EntityPropertyFlags propertyFlags;
        propertyFlags += PROP_NAME;
        propertyFlags += PROP_DIMENSIONS;
        auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
        auto properties = entityScriptingInterface->getEntityPropertiesInternal(entityID, propertyFlags, false);
        auto name = properties.getName();
        auto dimensions = properties.getDimensions();

        bool doResize = false;

        const glm::vec3 DEFAULT_DIMENSIONS = glm::vec3(0.1f, 0.1f, 0.1f);
        if (dimensions != DEFAULT_DIMENSIONS) {

            // Scale model so that its maximum is exactly specific size.
            const float MAXIMUM_DIMENSION = getMyAvatar()->getSensorToWorldScale();
            auto previousDimensions = dimensions;
            auto scale = std::min(MAXIMUM_DIMENSION / dimensions.x, std::min(MAXIMUM_DIMENSION / dimensions.y,
                MAXIMUM_DIMENSION / dimensions.z));
            dimensions *= scale;
            qInfo(interfaceapp) << "Model" << name << "auto-resized from" << previousDimensions << " to " << dimensions;
            doResize = true;

            item = _addAssetToWorldResizeList.erase(item);  // Finished with this entity; advance to next.
        } else {
            // Increment count of checks done.
            _addAssetToWorldResizeList[entityID]++;

            const int CHECK_MODEL_SIZE_MAX_CHECKS = 300;
            if (_addAssetToWorldResizeList[entityID] > CHECK_MODEL_SIZE_MAX_CHECKS) {
                // Have done enough checks; model was either the default size or something's gone wrong.

                // Rescale all dimensions.
                const glm::vec3 UNIT_DIMENSIONS = glm::vec3(1.0f, 1.0f, 1.0f);
                dimensions = UNIT_DIMENSIONS;
                qInfo(interfaceapp) << "Model" << name << "auto-resize timed out; resized to " << dimensions;
                doResize = true;

                item = _addAssetToWorldResizeList.erase(item);  // Finished with this entity; advance to next.
            } else {
                // No action on this entity; advance to next.
                ++item;
            }
        }

        if (doResize) {
            EntityItemProperties properties;
            properties.setDimensions(dimensions);
            properties.setVisible(true);
            if (!name.toLower().endsWith(PNG_EXTENSION) && !name.toLower().endsWith(JPG_EXTENSION)) {
                properties.setCollisionless(false);
            }
            bool grabbable = (Menu::getInstance()->isOptionChecked(MenuOption::CreateEntitiesGrabbable));
            properties.setUserData(grabbable ? GRABBABLE_USER_DATA : NOT_GRABBABLE_USER_DATA);
            properties.setLastEdited(usecTimestampNow());
            entityScriptingInterface->editEntity(entityID, properties);
        }
    }

    // Stop timer if nothing in list to check.
    if (_addAssetToWorldResizeList.size() == 0) {
        _addAssetToWorldResizeTimer.stop();
    }
}

void Application::onAssetToWorldMessageBoxClosed() {
    if (_addAssetToWorldMessageBox) {
        // User manually closed message box; perhaps because it has become stuck, so reset all messages.
        qInfo(interfaceapp) << "User manually closed download status message box";
        disconnect(_addAssetToWorldMessageBox);
        _addAssetToWorldMessageBox = nullptr;
        addAssetToWorldMessageClose();
    }
}

void Application::addAssetToWorldInfoTimeout() {
    if (_aboutToQuit) {
        return;
    }

    /*
    If list not empty, display last message in list (may already be displayed ) unless an error is being displayed.
    If list empty, close the message box unless an error is being displayed.
    */

    if (!_addAssetToWorldErrorTimer.isActive() && _addAssetToWorldMessageBox) {
        if (_addAssetToWorldInfoKeys.length() > 0) {
            _addAssetToWorldMessageBox->setProperty("text", "\n" + _addAssetToWorldInfoMessages.last());
        } else {
            disconnect(_addAssetToWorldMessageBox);
            _addAssetToWorldMessageBox->setVisible(false);
            _addAssetToWorldMessageBox->deleteLater();
            _addAssetToWorldMessageBox = nullptr;
        }
    }
}

void Application::addAssetToWorldErrorTimeout() {
    if (_aboutToQuit) {
        return;
    }

    /*
    If list is not empty, display message from last entry.
    If list is empty, close the message box.
    */

    if (_addAssetToWorldMessageBox) {
        if (_addAssetToWorldInfoKeys.length() > 0) {
            _addAssetToWorldMessageBox->setProperty("text", "\n" + _addAssetToWorldInfoMessages.last());
        } else {
            disconnect(_addAssetToWorldMessageBox);
            _addAssetToWorldMessageBox->setVisible(false);
            _addAssetToWorldMessageBox->deleteLater();
            _addAssetToWorldMessageBox = nullptr;
        }
    }
}

bool Application::importJSONFromURL(const QString& urlString) {
    // we only load files that terminate in just .json (not .svo.json and not .ava.json)
    QUrl jsonURL { urlString };

    emit svoImportRequested(urlString);
    return true;
}

bool Application::importSVOFromURL(const QString& urlString) {
    emit svoImportRequested(urlString);
    return true;
}

bool Application::importFromZIP(const QString& filePath) {
    qDebug() << "A zip file has been dropped in: " << filePath;
    QUrl empty;
    // handle Blocks download from Marketplace
    if (filePath.contains("poly.google.com/downloads")) {
        addAssetToWorldFromURL(filePath);
    } else {
        qApp->getFileDownloadInterface()->runUnzip(filePath, empty, true, true, false);
    }
    return true;
}

bool Application::importImage(const QString& urlString) {
    qCDebug(interfaceapp) << "An image file has been dropped in";
    QString filepath(urlString);
#if defined(Q_OS_WIN)
    filepath.remove("file:///");
#else
    filepath.remove("file://");
#endif
    addAssetToWorld(filepath, "", false, false);
    return true;
}
