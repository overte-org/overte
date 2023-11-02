//
//  DiscordRichPresence.cpp
//  interface/src
//
//  Created by Julian Groß on 30th October 2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>

#include "discord_rpc.h"
#include "DiscordRichPresence.h"
#include "DependencyManager.h"
#include "AddressManager.h"
#include "EntityTreeRenderer.h"

#define DISCORD_APPLICATION_CLIENT_ID "1168082546270163014"
#define STEAM_APPLICATION_ID "1234" // placeholder since we don't have a Steam application ID yet

Q_LOGGING_CATEGORY(discord_rich_presence, "overte.discord_rich_presence")

DiscordPresence::DiscordPresence()
{
    DiscordEventHandlers handlers;
    Discord_Initialize(DISCORD_APPLICATION_CLIENT_ID, &handlers, 1, STEAM_APPLICATION_ID);
    const int64_t startEpoch = QDateTime::currentSecsSinceEpoch();
    discordPresence.startTimestamp = startEpoch;
    auto addressManager = DependencyManager::get<AddressManager>();
    connect(addressManager.data(), &AddressManager::hostChanged, this, &DiscordPresence::domainChanged);
}

void DiscordPresence::shutdown()
{
    Discord_Shutdown();
}

void DiscordPresence::domainChanged()
{
    const auto addressManager = DependencyManager::get<AddressManager>();
    if (!addressManager) return;
    const auto entityTreeRenderer = DependencyManager::get<EntityTreeRenderer>();
    if (!entityTreeRenderer) return;

    // only continue if domain id changed or is serverless
    bool isServerless = false;
    const auto tree = entityTreeRenderer->getTree();
    if (tree) isServerless = tree->isServerlessMode();
    QString domainID = addressManager->getDomainID();
    if (currentDomainID == domainID && !isServerless) return;
    currentDomainID = domainID;

    // get data
    QString state;
    // TODO: switch to getPlaceName once https://github.com/overte-org/overte/issues/684 is fixed
    const QString worldName = addressManager->getHost();
    qCDebug(discord_rich_presence) << "Discord log hostName: " + worldName;
    if (isServerless) {
        state = "In a serverless world";
    } else {
        state = ("In " + worldName);
    }

    // create Discord presence payload
    QByteArray state_data = state.toUtf8();
    discordPresence.state = state_data.constData();
    discordPresence.largeImageKey = "header";
    discordPresence.smallImageKey = "logo";

    // update activity
    Discord_UpdatePresence(&discordPresence);
}
