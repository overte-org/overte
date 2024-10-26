//
//  DomainAccountManager.cpp
//  libraries/networking/src
//
//  Created by David Rowe on 23 Jul 2020.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "DomainAccountManager.h"

#include <QTimer>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <DependencyManager.h>
#include <SettingHandle.h>

#include "NetworkingConstants.h"
#include "NetworkAccessManager.h"
#include "NetworkLogging.h"
#include "NodeList.h"
#include "LDAPAccount.h"

// FIXME: Generalize to other OAuth2 sources for domain login.

const bool VERBOSE_HTTP_REQUEST_DEBUGGING = false;

DomainAccountManager::DomainAccountManager() {
    connect(this, &DomainAccountManager::loginComplete, this, &DomainAccountManager::sendInterfaceAccessTokenToServer);
}

void DomainAccountManager::setDomainURL(const QUrl& domainURL) {
    if (domainURL == _currentAuth.domainURL) {
        return;
    }

    qCDebug(networking) << "DomainAccountManager domain URL has been changed to" << qPrintable(domainURL.toString());

    // Restore OAuth2 authorization if have it for this domain.
    if (_knownAuths.contains(domainURL)) {
        _currentAuth = _knownAuths.value(domainURL);
    } else {
        _currentAuth = DomainAccountDetails();
        _currentAuth.domainURL = domainURL;
    }

    emit hasLogInChanged(hasLogIn());
}

void DomainAccountManager::setAuthURL(const QUrl& authURL) {
    if (authURL == _currentAuth.authURL) {
        return;
    }

    _currentAuth.authURL = authURL;
    qCDebug(networking) << "DomainAccountManager URL for authenticated requests has been changed to"
        << qPrintable(_currentAuth.authURL.toString());

    _currentAuth.accessToken = "";
    _currentAuth.refreshToken = "";

    emit hasLogInChanged(hasLogIn());
}

bool DomainAccountManager::hasLogIn() {
    return !_currentAuth.authURL.isEmpty();
}

bool DomainAccountManager::isLoggedIn() {
    return !_currentAuth.authURL.isEmpty() && hasValidAccessToken();
}

void DomainAccountManager::requestAccessToken(const QString& username, const QString& password, const QString& type) {
    if (type == "wordpress") return requestAccessTokenWordPress(username, password);
    if (type == "ldap") return requestAccessTokenLDAP(username, password);
}

void DomainAccountManager::requestAccessTokenWordPress(const QString& username, const QString& password) {
    _currentAuth.username = username;
    _currentAuth.accessToken = "";
    _currentAuth.refreshToken = "";

    QNetworkRequest request;

    request.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // miniOrange WordPress API Authentication plugin:
    // - Requires "client_id" parameter.
    // - Ignores "state" parameter.
    QByteArray formData;
    formData.append("grant_type=password&");
    formData.append("username=" + QUrl::toPercentEncoding(username) + "&");
    formData.append("password=" + QUrl::toPercentEncoding(password) + "&");
    formData.append("client_id=" + _currentAuth.clientID.toUtf8());

    request.setUrl(_currentAuth.authURL);

    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkAccessManager& networkAccessManager = NetworkAccessManager::getInstance();
    QNetworkReply* requestReply = networkAccessManager.post(request, formData);
    connect(requestReply, &QNetworkReply::finished, this, &DomainAccountManager::requestAccessTokenFinished);
}

void DomainAccountManager::requestAccessTokenLDAP(const QString& username, const QString& password) {
    _currentAuth.username = username;
    _currentAuth.accessToken = "";
    _currentAuth.refreshToken = "";

    LDAPAccount::setLDAPServerURL(_currentAuth.authURL.toString());

    // NOTE: Do not check if the credentials are valid on the client side first. Let the server handle everything.
    // FIXME: Check that this is secure. Worst comes to worse we can splice in cryptography to make it secure at the cost of sanity for future developers. -AD
    // const bool isValidLDAPCredentials = LDAPAccount::isValidCredentials(username, password);
    // if (isValidLDAPCredentials) {
    // Set the password as the access token.
    _currentAuth.accessToken = password;

    // Set the authenticated host name.
    auto nodeList = DependencyManager::get<NodeList>();
    _currentAuth.authedDomainName = nodeList->getDomainHandler().getHostname();

    // Remember domain login for the current Interface session.
    _knownAuths.insert(_currentAuth.domainURL, _currentAuth);

    emit loginComplete();
    return;
    // }

    // TODO: Failure state: This code will not run due to not checking validity of credentials. See notes and fixmes above.
    // Failure.
    // FIXME: QML does not update to show sign in failure.
    // qCDebug(networking) << "LDAP account failed to verify";
}

void DomainAccountManager::requestAccessTokenFinished() {

    QNetworkReply* requestReply = reinterpret_cast<QNetworkReply*>(sender());

    QJsonDocument jsonResponse = QJsonDocument::fromJson(requestReply->readAll());
    const QJsonObject& rootObject = jsonResponse.object();

    auto httpStatus = requestReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (200 <= httpStatus && httpStatus < 300) {

        // miniOrange plugin provides no scope.
        if (rootObject.contains("access_token")) {
            // Success.
            auto nodeList = DependencyManager::get<NodeList>();
            _currentAuth.authedDomainName = nodeList->getDomainHandler().getHostname();
            QUrl rootURL = requestReply->url();
            rootURL.setPath("");
            setTokensFromJSON(rootObject, rootURL);

            // Remember domain login for the current Interface session.
            _knownAuths.insert(_currentAuth.domainURL, _currentAuth);

            // ####### TODO: Handle "keep me logged in".

            emit loginComplete();
        } else {
            // Failure.
            qCDebug(networking) << "Received a response for password grant that is missing one or more expected values.";
            emit loginFailed();
        }

    } else {
        // Failure.
        qCDebug(networking) << "Error in response for password grant -" << httpStatus << requestReply->error()
            << "-" << rootObject["error"].toString() << rootObject["error_description"].toString();
        emit loginFailed();
    }
}

void DomainAccountManager::sendInterfaceAccessTokenToServer() {
    emit newTokens();
}

bool DomainAccountManager::accessTokenIsExpired() {
    // ####### TODO: accessTokenIsExpired()
    return true;
}


bool DomainAccountManager::hasValidAccessToken() {
    // ###### TODO: wire this up to actually retrieve a token (based on session or storage) and confirm that it is in fact valid and relevant to the current domain.
    // QString currentDomainAccessToken = domainAccessToken.get();
    QString currentDomainAccessToken = _currentAuth.accessToken;

    // if (currentDomainAccessToken.isEmpty() || accessTokenIsExpired()) {
    if (currentDomainAccessToken.isEmpty()) {
        if (VERBOSE_HTTP_REQUEST_DEBUGGING) {
            qCDebug(networking) << "An access token is required for requests to"
                                << qPrintable(_currentAuth.authURL.toString());
        }

        return false;
    }

    // ####### TODO

    // if (!_isWaitingForTokenRefresh && needsToRefreshToken()) {
    //     refreshAccessToken();
    // }

    return true;
}

void DomainAccountManager::setTokensFromJSON(const QJsonObject& jsonObject, const QUrl& url) {
    _currentAuth.accessToken = jsonObject["access_token"].toString();
    _currentAuth.refreshToken = jsonObject["refresh_token"].toString();
}

bool DomainAccountManager::checkAndSignalForAccessToken() {
    bool hasToken = hasValidAccessToken();

    // ####### TODO: Handle hasToken == true.
    // It causes the login dialog not to display (OK) but somewhere the domain server needs to be sent it (and if domain server
    // gets error when trying to use it then user should be prompted to login).
    hasToken = false;

    if (!hasToken) {
        // Emit a signal so somebody can call back to us and request an access token given a user name and password.

        // Dialog can be hidden immediately after showing if we've just teleported to the domain, unless the signal is delayed.
        auto domain = _currentAuth.authURL.host();
        QTimer::singleShot(500, this, [this, domain] {
            emit this->authRequired(domain);
        });
    }

    return hasToken;
}
