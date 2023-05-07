#pragma once

#include <QtNetwork>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <NetworkAccessManager.h>

void sendWebhookMessage(const QString& webhookUrl, const QJsonObject& json);
void sendDiscordMessage(QString& webookUrl, QString& message);
void sendMatrixMessage(QString& webhookUrl, QString& message, QString& username);
