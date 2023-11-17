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
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

#include "discord_rpc.h"
#include "DiscordRichPresence.h"
#include "DependencyManager.h"
#include "AddressManager.h"
#include "scripting/HMDScriptingInterface.h"
#include "Application.h"
#include "EntityTreeRenderer.h"

#define DISCORD_APPLICATION_CLIENT_ID "1168082546270163014"
#define STEAM_APPLICATION_ID "1234" // placeholder since we don't have a Steam application ID yet

Q_LOGGING_CATEGORY(discord_rich_presence, "overte.discord_rich_presence")

DiscordPresence::DiscordPresence()
{
    DiscordEventHandlers handlers;
    Discord_Initialize(DISCORD_APPLICATION_CLIENT_ID, &handlers, 1, STEAM_APPLICATION_ID);
    discordPresence.largeImageKey = "header";
    discordPresence.smallImageKey = "";
    discordPresence.smallImageText = "";
    const int64_t startEpoch = QDateTime::currentSecsSinceEpoch();
    discordPresence.startTimestamp = startEpoch;

    auto addressManager = DependencyManager::get<AddressManager>();
    connect(addressManager.data(), &AddressManager::hostChanged, this, &DiscordPresence::domainChanged);

    auto hMDScriptingInterface = DependencyManager::get<HMDScriptingInterface>();
    DiscordPresence::vrChanged(hMDScriptingInterface->isHMDMode()); // we don't receive the initial signal, so we ask for the current state
    connect(hMDScriptingInterface.data(), &HMDScriptingInterface::displayModeChanged, this, &DiscordPresence::vrChanged);
}

void DiscordPresence::shutdown()
{
    Discord_Shutdown();
}

void DiscordPresence::domainChanged()
{
    const auto addressManager = DependencyManager::get<AddressManager>();
    if (!addressManager) return;

    QString state;
    // TODO: switch to getPlaceName once https://github.com/overte-org/overte/issues/684 is fixed
    const QString worldName = addressManager->getHost();
    if (worldName != "") {
        state = ("In " + worldName);
        qCDebug(discord_rich_presence) << "Discord log hostName: " + worldName;
    } else {
        state = "";
        qCDebug(discord_rich_presence) << "Discord log: in unknown place";
    }

    QByteArray state_data = state.toUtf8();
    discordPresence.state = state_data.constData();

    Discord_UpdatePresence(&discordPresence);
}

void DiscordPresence::vrChanged(bool isHMDMode = false)
{
    if (isHMDMode) {
        discordPresence.smallImageKey = "hmd";
        discordPresence.smallImageText = "in VR";
    } else {
        discordPresence.smallImageKey = "desktop";
        discordPresence.smallImageText = "in desktop mode";
    }
    qCDebug(discord_rich_presence) << "Discord log HMDMode: " + QString::number(isHMDMode);
    Discord_UpdatePresence(&discordPresence);
}
