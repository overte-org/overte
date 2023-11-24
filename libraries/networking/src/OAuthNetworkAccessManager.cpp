//
//  OAuthNetworkAccessManager.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2014-09-18.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "OAuthNetworkAccessManager.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QThreadStorage>

#include "AccountManager.h"
#include "LimitedNodeList.h"
#include "NetworkingConstants.h"
#include "MetaverseAPI.h"
#include "SharedUtil.h"

QThreadStorage<OAuthNetworkAccessManager*> oauthNetworkAccessManagers;

OAuthNetworkAccessManager* OAuthNetworkAccessManager::getInstance() {
    if (!oauthNetworkAccessManagers.hasLocalData()) {
        oauthNetworkAccessManagers.setLocalData(new OAuthNetworkAccessManager());
    }
    
    return oauthNetworkAccessManagers.localData();
}

QNetworkReply* OAuthNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest& req,
                                                        QIODevice* outgoingData) {
    auto accountManager = DependencyManager::get<AccountManager>();
    
    if (accountManager->hasValidAccessToken()
        && req.url().host() == MetaverseAPI::getCurrentMetaverseServerURL().host()) {
        QNetworkRequest authenticatedRequest(req);
        authenticatedRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        authenticatedRequest.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
        authenticatedRequest.setRawHeader(ACCESS_TOKEN_AUTHORIZATION_HEADER,
                                          accountManager->getAccountInfo().getAccessToken().authorizationHeaderValue());
        
        return QNetworkAccessManager::createRequest(op, authenticatedRequest, outgoingData);
    } else {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }
}
