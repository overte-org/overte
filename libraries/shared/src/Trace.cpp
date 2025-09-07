//
//  Created by Ryan Huffman on 2016-12-14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Trace.h"

#include <chrono>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryFile>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

#include <BuildInfo.h>

#include "Gzip.h"
#include "PortableHighResolutionClock.h"
#include "SharedLogging.h"
#include "shared/FileUtils.h"
#include "shared/GlobalAppProperties.h"

using namespace tracing;

bool tracing::enabled() {
    return DependencyManager::get<Tracer>()->isEnabled();
}

void Tracer::startTracing() {
    std::lock_guard<std::mutex> guard(_eventsMutex);
    if (_enabled) {
        qWarning() << "Tried to enable tracer, but already enabled";
        return;
    }

    _events.clear();
    _enabled = true;
}

void Tracer::stopTracing() {
    std::lock_guard<std::mutex> guard(_eventsMutex);
    if (!_enabled) {
        qWarning() << "Cannot stop tracing, already disabled";
        return;
    }
    _enabled = false;
}

void TraceEvent::writeJson(QTextStream& out) const {
#if 0
    // FIXME QJsonObject serialization is very slow, so we should be using manual JSON serialization
    out << "{";
    out << "\"name\":\"" << name << "\",";
    out << "\"cat\":\"" << category.categoryName() << "\",";
    out << "\"ph\":\"" << QString(type) << "\",";
    out << "\"ts\":\"" << timestamp << "\",";
    out << "\"pid\":\"" << processID << "\",";
    out << "\"tid\":\"" << threadID << "\"";
    //if (!extra.empty()) {
    //    auto it = extra.begin();
    //    for (; it != extra.end(); it++) {
    //        ev[it.key()] = QJsonValue::fromVariant(it.value());
    //    }
    //}
    //if (!args.empty()) {
    //    out << ",\"args\":'
    //}
    out << '}';
#else
    QJsonObject ev {
        { "name", QJsonValue(name) },
        { "cat", category.categoryName() },
        { "ph", QString(static_cast<char>(type)) },
        { "ts", timestamp },
        { "pid", processID },
        { "tid", threadID }
    };
    if (!id.isEmpty()) {
        ev["id"] = id;
    }
    if (!args.empty()) {
        ev["args"] = QJsonObject::fromVariantMap(args);
    }
    if (!extra.empty()) {
        auto it = extra.begin();
        for (; it != extra.end(); it++) {
            ev[it.key()] = QJsonValue::fromVariant(it.value());
        }
    }
    out << QJsonDocument(ev).toJson(QJsonDocument::Compact);
#endif
}

void Tracer::serialize(const QString& filename) {
    QString fullPath = FileUtils::replaceDateTimeTokens(filename);
    fullPath = FileUtils::computeDocumentPath(fullPath);
    if (!FileUtils::canCreateFile(fullPath)) {
        return;
    }

    std::list<TraceEvent> currentEvents;
    {
        std::lock_guard<std::mutex> guard(_eventsMutex);
        currentEvents.swap(_events);
        for (auto& event : _metadataEvents) {
            currentEvents.push_back(event);
        }
    }

    // If we can't open a temp file for writing, fail early
    QByteArray data;
    {
        QTextStream out(&data);
        out << "[\n";
        bool first = true;
        for (const auto& event : currentEvents) {
            if (first) {
                first = false;
            } else {
                out << ",\n";
            }
            event.writeJson(out);
        }
        out << "\n]";
    }

    if (fullPath.endsWith(".gz")) {
        QByteArray compressed;
        gzip(data, compressed);
        data = compressed;
    }

    {
        QFile file(fullPath);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug(shared) << "failed to open file '" << fullPath << "'";
            return;
        }
        file.write(data);
        file.close();
    }

#if 0
    QByteArray data;
    {

        // "traceEvents":[
        // {"args":{"nv_payload":0},"cat":"hifi.render","name":"render::Scene::processTransactionQueue","ph":"B","pid":14796,"tid":21636,"ts":68795933487}

        QJsonArray traceEvents;

        QJsonDocument document {
            QJsonObject {
                { "traceEvents", traceEvents },
                { "otherData", QJsonObject {
                    { "version", QString { "High Fidelity Interface v1.0" } +BuildInfo::VERSION }
                } }
            }
        };
        data = document.toJson(QJsonDocument::Compact);
    }
#endif
}

int64_t Tracer::now() {
    return std::chrono::duration_cast<std::chrono::microseconds>(p_high_resolution_clock::now().time_since_epoch()).count();
}

void Tracer::traceEvent(const QLoggingCategory& category,
    const QString& name, EventType type,
    qint64 timestamp, qint64 processID, qint64 threadID,
    const QString& id,
    const QVariantMap& args, const QVariantMap& extra) {
    std::lock_guard<std::mutex> guard(_eventsMutex);

    // We always want to store metadata events even if tracing is not enabled so that when
    // tracing is enabled we will be able to associate that metadata with that trace.
    // Metadata events should be used sparingly - as of 12/30/16 the Chrome Tracing
    // spec only supports thread+process metadata, so we should only expect to see metadata
    // events created when a new thread or process is created.
    if (!_enabled && type != Metadata) {
        return;
    }

    if (type == Metadata) {
        _metadataEvents.push_back({
            id,
            name,
            type,
            timestamp,
            processID,
            threadID,
            category,
            args,
            extra
        });
    } else {
        _events.push_back({
            id,
            name,
            type,
            timestamp,
            processID,
            threadID,
            category,
            args,
            extra
        });
    }
}

void Tracer::traceEvent(const QLoggingCategory& category, 
    const QString& name, EventType type, const QString& id, 
    const QVariantMap& args, const QVariantMap& extra) {
    if (!_enabled && type != Metadata) {
        return;
    }

    traceEvent(category, name, type, now(), id, args, extra);
}

void Tracer::traceEvent(const QLoggingCategory& category, 
    const QString& name, EventType type, int64_t timestamp, const QString& id, 
    const QVariantMap& args, const QVariantMap& extra) {
    if (!_enabled && type != Metadata) {
        return;
    }

    auto processID = QCoreApplication::applicationPid();
    auto threadID = int64_t(QThread::currentThreadId());
    traceEvent(category, name, type, timestamp, processID, threadID, id, args, extra);
}
