//
//  DiscoverabilityManager.cpp
//  interface/src
//
//  Created by Stephen Birarda on 2015-03-09.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "DiscoverabilityManager.h"

#include <QtCore/QJsonDocument>
#include <QThread>

#include <AccountManager.h>
#include <AddressManager.h>
#include <DomainHandler.h>
#include <NodeList.h>
#include <plugins/PluginManager.h>
#include <plugins/SteamClientPlugin.h>
#include <UserActivityLogger.h>
#include <UUID.h>
#include <crash-handler/CrashHandler.h>

#include "Menu.h"

const Discoverability::Mode DEFAULT_DISCOVERABILITY_MODE = Discoverability::Connections;

DiscoverabilityManager::DiscoverabilityManager() :
    _mode("discoverabilityMode", DEFAULT_DISCOVERABILITY_MODE)
{
    qRegisterMetaType<Discoverability::Mode>("Discoverability::Mode");
}

const QString API_USER_LOCATION_PATH = "/api/v1/user/location";
const QString API_USER_HEARTBEAT_PATH = "/api/v1/user/heartbeat";

const QString SESSION_ID_KEY = "session_id";

void DiscoverabilityManager::updateLocation() {
    // since we store the last location and compare it to
    // the current one in this function, we need to do this in
    // the object's main thread (or use a mutex)
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "updateLocation");
        return;
    }
    auto accountManager = DependencyManager::get<AccountManager>();
    auto addressManager = DependencyManager::get<AddressManager>();
    auto& domainHandler = DependencyManager::get<NodeList>()->getDomainHandler();
    bool discoverable = (_mode.get() != Discoverability::None) && !domainHandler.isServerless();


    if (accountManager->isLoggedIn()) {
        // construct a QJsonObject given the user's current address information
        QJsonObject rootObject;

        QJsonObject locationObject;

        QString pathString = addressManager->currentPath();

        const QString CONNECTED_KEY_IN_LOCATION = "connected";
        locationObject.insert(CONNECTED_KEY_IN_LOCATION, discoverable && domainHandler.isConnected());

        if (discoverable || _lastLocationObject.isEmpty()) { // Don't consider changes to these as update-worthy if we're not discoverable.
            const QString PATH_KEY_IN_LOCATION = "path";
            locationObject.insert(PATH_KEY_IN_LOCATION, pathString);

            if (!addressManager->getRootPlaceID().isNull()) {
                const QString PLACE_ID_KEY_IN_LOCATION = "place_id";
                locationObject.insert(PLACE_ID_KEY_IN_LOCATION,
                                      uuidStringWithoutCurlyBraces(addressManager->getRootPlaceID()));
            }

            if (!domainHandler.getUUID().isNull()) {
                const QString DOMAIN_ID_KEY_IN_LOCATION = "domain_id";
                locationObject.insert(DOMAIN_ID_KEY_IN_LOCATION,
                                      uuidStringWithoutCurlyBraces(domainHandler.getUUID()));
            }

            // in case the place/domain isn't in the database, we send the network address and port
            auto& domainSockAddr = domainHandler.getSockAddr();
            const QString NETWORK_ADDRESS_KEY_IN_LOCATION = "network_address";
            // TODO(IPv6):
            locationObject.insert(NETWORK_ADDRESS_KEY_IN_LOCATION, domainSockAddr.getAddressIPv4().toString());

            const QString NETWORK_ADDRESS_PORT_IN_LOCATION = "network_port";
            locationObject.insert(NETWORK_ADDRESS_PORT_IN_LOCATION, domainSockAddr.getPort());

            const QString NODE_ID_IN_LOCATION = "node_id";
            const int UUID_REAL_LENGTH = 36;
            locationObject.insert(NODE_ID_IN_LOCATION, DependencyManager::get<NodeList>()->getSessionUUID().toString().mid(1, UUID_REAL_LENGTH));
        }

        const QString AVAILABILITY_KEY_IN_LOCATION = "availability";
        locationObject.insert(AVAILABILITY_KEY_IN_LOCATION, findableByString(static_cast<Discoverability::Mode>(_mode.get())));

        JSONCallbackParameters callbackParameters;
        callbackParameters.callbackReceiver = this;
        callbackParameters.jsonCallbackMethod = "handleHeartbeatResponse";

        // figure out if we'll send a fresh location or just a simple heartbeat
        auto apiPath = API_USER_HEARTBEAT_PATH;

        if (locationObject != _lastLocationObject) {
            // we have a changed location, send it now
            _lastLocationObject = locationObject;

            const QString LOCATION_KEY_IN_ROOT = "location";
            rootObject.insert(LOCATION_KEY_IN_ROOT, locationObject);

            apiPath = API_USER_LOCATION_PATH;
        }

        accountManager->sendRequest(apiPath, AccountManagerAuth::Required,
                                   QNetworkAccessManager::PutOperation,
                                   callbackParameters, QJsonDocument(rootObject).toJson());

    } else if (UserActivityLogger::getInstance().isEnabled()) {
        // we still send a heartbeat to the directory server for stats collection

        JSONCallbackParameters callbackParameters;
        callbackParameters.callbackReceiver = this;
        callbackParameters.jsonCallbackMethod = "handleHeartbeatResponse";

        accountManager->sendRequest(API_USER_HEARTBEAT_PATH, AccountManagerAuth::Optional,
                                   QNetworkAccessManager::PutOperation, callbackParameters);
    }

    // Update Steam and crash logger
    QUrl currentAddress = addressManager->currentFacingPublicAddress();
    if (auto steamClient = PluginManager::getInstance()->getSteamClientPlugin()) {
        steamClient->updateLocation(domainHandler.getHostname(), currentAddress);
    }

    CrashHandler::getInstance().setAnnotation("address", currentAddress.toString().toStdString());
}

void DiscoverabilityManager::handleHeartbeatResponse(QNetworkReply* requestReply) {
    auto dataObject = AccountManager::dataObjectFromResponse(requestReply);

    if (!dataObject.isEmpty()) {
        auto sessionID = dataObject[SESSION_ID_KEY].toString();

        // give that session ID to the account manager
        auto accountManager = DependencyManager::get<AccountManager>();
        accountManager->setSessionID(sessionID);
    }
}

void DiscoverabilityManager::removeLocation() {
    auto accountManager = DependencyManager::get<AccountManager>();
    accountManager->sendRequest(API_USER_LOCATION_PATH, AccountManagerAuth::Required, QNetworkAccessManager::DeleteOperation);
}

void DiscoverabilityManager::setDiscoverabilityMode(Discoverability::Mode discoverabilityMode) {
    if (static_cast<Discoverability::Mode>(_mode.get()) != discoverabilityMode) {

        // update the setting to the new value
        _mode.set(static_cast<int>(discoverabilityMode));
        updateLocation();  // update right away

        emit discoverabilityModeChanged(discoverabilityMode);
    }
}


QString DiscoverabilityManager::findableByString(Discoverability::Mode discoverabilityMode) {
    if (discoverabilityMode == Discoverability::None) {
        return "none";
    } else if (discoverabilityMode == Discoverability::Friends) {
        return "friends";
    } else if (discoverabilityMode == Discoverability::Connections) {
        return "connections";
    } else if (discoverabilityMode == Discoverability::All) {
        return "all";
    } else {
        qDebug() << "GlobalServices findableByString called with an unrecognized value.";
        return "";
    }
}


void DiscoverabilityManager::setVisibility() {
    Menu* menu = Menu::getInstance();

    if (menu->isOptionChecked(MenuOption::VisibleToEveryone)) {
        this->setDiscoverabilityMode(Discoverability::All);
    } else if (menu->isOptionChecked(MenuOption::VisibleToFriends)) {
        this->setDiscoverabilityMode(Discoverability::Friends);
    } else if (menu->isOptionChecked(MenuOption::VisibleToNoOne)) {
        this->setDiscoverabilityMode(Discoverability::None);
    } else {
        qDebug() << "ERROR DiscoverabilityManager::setVisibility() called with unrecognized value.";
    }
}

void DiscoverabilityManager::visibilityChanged(Discoverability::Mode discoverabilityMode) {
    Menu* menu = Menu::getInstance();

    if (discoverabilityMode == Discoverability::All) {
        menu->setIsOptionChecked(MenuOption::VisibleToEveryone, true);
    } else if (discoverabilityMode == Discoverability::Friends) {
        menu->setIsOptionChecked(MenuOption::VisibleToFriends, true);
    } else if (discoverabilityMode == Discoverability::None) {
        menu->setIsOptionChecked(MenuOption::VisibleToNoOne, true);
    } else {
        qDebug() << "ERROR DiscoverabilityManager::visibilityChanged() called with unrecognized value.";
    }
}
