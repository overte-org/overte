//
//  EntityScriptServerLogClient.cpp
//  interface/src
//
//  Created by Clement Brisset on 2/1/17.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QJsonDocument>
#include <QJsonArray>
#include "EntityScriptServerLogClient.h"
#include "ScriptMessage.h"
#include "ScriptEngines.h"

EntityScriptServerLogClient::EntityScriptServerLogClient() {
    auto nodeList = DependencyManager::get<NodeList>();
    auto& packetReceiver = nodeList->getPacketReceiver();
    packetReceiver.registerListener(PacketType::EntityServerScriptLog,
        PacketReceiver::makeSourcedListenerReference<EntityScriptServerLogClient>(this, &EntityScriptServerLogClient::handleEntityServerScriptLogPacket));

    QObject::connect(nodeList.data(), &NodeList::nodeActivated, this, &EntityScriptServerLogClient::nodeActivated);
    QObject::connect(nodeList.data(), &NodeList::nodeKilled, this, &EntityScriptServerLogClient::nodeKilled);

    QObject::connect(nodeList.data(), &NodeList::canRezChanged, this, &EntityScriptServerLogClient::canRezChanged);
}

void EntityScriptServerLogClient::connectNotify(const QMetaMethod& signal) {
    // This needs to be delayed or "receivers()" will return completely inconsistent values
    QMetaObject::invokeMethod(this, "connectionsChanged", Qt::QueuedConnection);
}

void EntityScriptServerLogClient::disconnectNotify(const QMetaMethod& signal) {
    // This needs to be delayed or "receivers()" will return completely inconsistent values
    QMetaObject::invokeMethod(this, "connectionsChanged", Qt::QueuedConnection);
}

void EntityScriptServerLogClient::connectionsChanged() {
    auto numReceivers = receivers(SIGNAL(receivedNewLogLines(QString)));
    if (!_subscribed && numReceivers > 0) {
        enableToEntityServerScriptLog(DependencyManager::get<NodeList>()->getThisNodeCanRez());
    } else if (_subscribed && numReceivers == 0) {
        enableToEntityServerScriptLog(false);
    }
}

void EntityScriptServerLogClient::enableToEntityServerScriptLog(bool enable) {
    auto nodeList = DependencyManager::get<NodeList>();

    if (auto node = nodeList->soloNodeOfType(NodeType::EntityScriptServer)) {
        auto packet = NLPacket::create(PacketType::EntityServerScriptLog, sizeof(bool), true);
        packet->writePrimitive(enable);
        nodeList->sendPacket(std::move(packet), *node);

        if (_subscribed != enable) {
            if (enable) {
                emit receivedNewLogLines("====================== Subscribed to the Entity Script Server's log ======================");
            } else {
                emit receivedNewLogLines("==================== Unsubscribed from the Entity Script Server's log ====================");
            }
        }
        _subscribed = enable;
    }
}

void EntityScriptServerLogClient::handleEntityServerScriptLogPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    QString messageText = QString::fromUtf8(message->readAll());
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(messageText.toUtf8(), &error);
    emit receivedNewLogLines(messageText);
    if(document.isNull()) {
        qWarning() << "EntityScriptServerLogClient::handleEntityServerScriptLogPacket: Cannot parse JSON: " << error.errorString()
            << " Contents: " << messageText;
        return;
    }
    // Iterate through contents and emit messages
    if(!document.isArray()) {
        qWarning() << "EntityScriptServerLogClient::handleEntityServerScriptLogPacket: JSON is not an array: " << messageText;
        return;
    }

    auto scriptEngines = DependencyManager::get<ScriptEngines>().data();

    auto array = document.array();
    for (int n = 0; n < array.size(); n++) {
        if (!array[n].isObject()) {
            qWarning() << "EntityScriptServerLogClient::handleEntityServerScriptLogPacket: message is not an object: " << messageText;
            continue;
        }
        ScriptMessage scriptMessage;
        if (!scriptMessage.fromJson(array[n].toObject())) {
            qWarning() << "EntityScriptServerLogClient::handleEntityServerScriptLogPacket: message parsing failed: " << messageText;
            continue;
        }
        switch (scriptMessage.getSeverity()) {
            case ScriptMessage::Severity::SEVERITY_INFO:
                emit scriptEngines->infoEntityMessage(scriptMessage.getMessage(), scriptMessage.getFileName(),
                                                      scriptMessage.getLineNumber(), scriptMessage.getEntityID(), true);
            break;

            case ScriptMessage::Severity::SEVERITY_PRINT:
                emit scriptEngines->printedEntityMessage(scriptMessage.getMessage(), scriptMessage.getFileName(),
                                                  scriptMessage.getLineNumber(), scriptMessage.getEntityID(), true);
            break;

            case ScriptMessage::Severity::SEVERITY_WARNING:
                emit scriptEngines->warningEntityMessage(scriptMessage.getMessage(), scriptMessage.getFileName(),
                                                  scriptMessage.getLineNumber(), scriptMessage.getEntityID(), true);
            break;

            case ScriptMessage::Severity::SEVERITY_ERROR:
                emit scriptEngines->errorEntityMessage(scriptMessage.getMessage(), scriptMessage.getFileName(),
                                                  scriptMessage.getLineNumber(), scriptMessage.getEntityID(), true);
            break;

            default:
            break;
        }
    }
}

void EntityScriptServerLogClient::nodeActivated(SharedNodePointer activatedNode) {
    if (_subscribed && activatedNode->getType() == NodeType::EntityScriptServer) {
        _subscribed = false;
        enableToEntityServerScriptLog(DependencyManager::get<NodeList>()->getThisNodeCanRez());
    }
}

void EntityScriptServerLogClient::nodeKilled(SharedNodePointer killedNode) {
    if (killedNode->getType() == NodeType::EntityScriptServer) {
        emit receivedNewLogLines("====================== Connection to the Entity Script Server lost ======================");
    }
}

void EntityScriptServerLogClient::canRezChanged(bool canRez) {
    auto numReceivers = receivers(SIGNAL(receivedNewLogLines(QString)));
    if (numReceivers > 0) {
        enableToEntityServerScriptLog(canRez);
    }
}
