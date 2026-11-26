#pragma once

#include <QtNetwork>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <NetworkAccessManager.h>
#include <SettingHandle.h>

void sendWebhookMessage(const QString& webookURL, QJsonObject& json);
void sendDiscordMessage(QString& message);
void sendMatrixMessage(QString& message, QString& username);
