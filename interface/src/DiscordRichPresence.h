//
//  DiscordRichPresence.h
//  interface/src
//
//  Created by Julian Groß on 30th October 2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef overte_DiscordPresence_h
#define overte_DiscordPresence_h

#include "discord_rpc.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>

Q_DECLARE_LOGGING_CATEGORY(discord_rich_presence)

class DiscordPresence : public QObject {
    Q_OBJECT
public:
    DiscordPresence();
    void shutdown();

public slots:
    void domainChanged();

private:
    QString currentDomainID;
    DiscordRichPresence discordPresence{};
};

#endif
