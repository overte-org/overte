#include "FSTOldReader.h"

//
//  FSTReader.cpp
//
//
//  Created by Clement on 3/26/15.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FSTReader.h"

#include <QBuffer>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <NetworkAccessManager.h>
#include <NetworkingConstants.h>
#include <SharedUtil.h>


Q_LOGGING_CATEGORY(fst_reader_old_logging, "overte.model-serializers.fst.old")


const QStringList SINGLE_VALUE_PROPERTIES{"name", "filename", "texdir", "script", "comment"};

hifi::VariantMultiHash FSTOldReader::parseMapping(QIODevice* device) {
    hifi::VariantMultiHash properties;

    QByteArray line;
    while (!(line = device->readLine()).isEmpty()) {
        if ((line = line.trimmed()).startsWith('#')) {
            continue; // comment
        }
        QList<QByteArray> sections = line.split('=');
        if (sections.size() < 2) {
            continue;
        }

        // We can have URLs like:
        // filename = https://www.dropbox.com/scl/fi/xxx/avatar.fbx?rlkey=xxx&dl=1\n
        // These confuse the parser due to the presence of = in the URL.
        //
        // SINGLE_VALUE_PROPERTIES contains a list of things that may be URLs or contain an =
        // for some other reason, and that we know for sure contain only a single value after
        // the first =.
        //
        // Really though, we should just use JSON instead.
        QByteArray name = sections.at(0).trimmed();
        bool isSingleValue = SINGLE_VALUE_PROPERTIES.contains(name);

        if (sections.size() == 2 || isSingleValue) {
            // This is a simple key = value

            // As per the above, we can have '=' signs inside of URLs, so instead of
            // using the split string, just use everything after the first '='.

            QString value = line.mid(line.indexOf("=")+1).trimmed();
            properties.insert(name, value);
        } else if (sections.size() == 3) {
            // This is a name = key = value
            // Or more normally, a dictionary called 'name', mapping 'key' to 'value'

            QVariantHash heading = properties.value(name).toHash();
            heading.insert(sections.at(1).trimmed(), sections.at(2).trimmed());
            properties.replace(name, heading);
        } else if (sections.size() >= 4) {
            // This is a name = key = newkey = value
            // For example:
            // bs = JawOpen = JawOpen = 0.5

            QVariantHash heading = properties.value(name).toHash();
            QVariantList contents;
            for (int i = 2; i < sections.size(); i++) {
                contents.append(sections.at(i).trimmed());
            }
            heading.insert(sections.at(1).trimmed(), contents);
            properties.replace(name, heading);
        }
    }

    return properties;
}


FSTReader::Mapping FSTOldReader::readMapping(const QByteArray& data) {
    QBuffer buffer(const_cast<QByteArray*>(&data));
    buffer.open(QIODevice::ReadOnly);
    Mapping mapping = parseMapping(&buffer);
    fixUpLegacyBlendshapes(mapping);
    return mapping;
}

void FSTOldReader::writeVariant(QBuffer& buffer, QVariantHash::const_iterator& it) {
    QByteArray key = it.key().toUtf8() + " = ";
    QVariantHash hashValue = it.value().toHash();
    if (hashValue.isEmpty()) {
        buffer.write(key + it.value().toByteArray() + "\n");
        return;
    }
    for (QVariantHash::const_iterator second = hashValue.constBegin(); second != hashValue.constEnd(); second++) {
        QByteArray extendedKey = key + second.key().toUtf8();
        QVariantList listValue = second.value().toList();
        if (listValue.isEmpty()) {
            buffer.write(extendedKey + " = " + second.value().toByteArray() + "\n");
            continue;
        }
        buffer.write(extendedKey);
        for (QVariantList::const_iterator third = listValue.constBegin(); third != listValue.constEnd(); third++) {
            buffer.write(" = " + third->toByteArray());
        }
        buffer.write("\n");
    }
}

QByteArray FSTOldReader::writeMapping(const Mapping& mapping) {
    static const QStringList PREFERED_ORDER = QStringList() << NAME_FIELD << TYPE_FIELD << SCALE_FIELD << FILENAME_FIELD
    << TEXDIR_FIELD << SCRIPT_FIELD << JOINT_FIELD
    << BLENDSHAPE_FIELD << JOINT_INDEX_FIELD;
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    for (auto key : PREFERED_ORDER) {
        auto it = mapping.find(key);
        if (it != mapping.constEnd()) {
            if (key == SCRIPT_FIELD) { // writeVariant does not handle strings added using insertMulti.
                for (auto multi : mapping.values(key)) {
                    buffer.write(key.toUtf8());
                    buffer.write(" = ");
                    buffer.write(multi.toByteArray());
                    buffer.write("\n");
                }
            } else {
                writeVariant(buffer, it);
            }
        }
    }

    for (auto it = mapping.constBegin(); it != mapping.constEnd(); it++) {
        if (!PREFERED_ORDER.contains(it.key())) {
            writeVariant(buffer, it);
        }
    }
    return buffer.data();
}





