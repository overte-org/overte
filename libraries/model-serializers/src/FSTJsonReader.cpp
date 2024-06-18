
#include "FSTJsonReader.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <algorithm>


Q_LOGGING_CATEGORY(fst_reader_json_logging, "overte.model-serializers.fst.json")


QSet<QString> FSTJsonReader::_jsonFields{"materialMap", "flowPhysicsData", "flowCollisionsData"};

FSTReader::Mapping FSTJsonReader::readMapping(const QByteArray& data) {
    FSTReader::Mapping mapping;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if ( parseError.error != QJsonParseError::NoError ) {
        logError(data, parseError);
        //qCWarning(fst_reader_json_logging) << "Failed to parse JSON:" << parseError.errorString() << "at" << parseError.offset;
        return Mapping();
    } else {
        //qCDebug(fst_reader_json_logging) << "Parse ok, result:" << doc;
    }


    if (!doc.isObject()) {
        qCWarning(fst_reader_json_logging) << "FST JSON must contain a hash at top level";
        return Mapping();
    }

    QJsonObject obj = doc.object();

    for (auto key : obj.keys() ) {
        mapping.insert(key, obj[key].toVariant());
    }

    return mapping;
};


QByteArray FSTJsonReader::writeMapping(const FSTReader::Mapping& mapping) {
    QJsonObject result;


    for (auto key : mapping.uniqueKeys()) {

        auto values = mapping.values(key);
        //qCDebug(fst_reader_json_logging) << "Converting " << key << "with" << values.size() << "values: " << values;

        if ( values.count() == 0 ) {
            continue;
        }


        QJsonArray jsonValues;

        for(auto value : values) {
            if ( _jsonFields.contains(key)) {
                //qCDebug(fst_reader_json_logging) << "Parsing JSON key: " << key << ":" << values.at(0);
                QJsonParseError parseError;
                QByteArray jsonData = values.at(0).toByteArray();

                if (jsonData.length() > 0) {
                    // Field may be present but empty, we ignore it in this case.

                    QJsonDocument tempDoc = QJsonDocument::fromJson(jsonData, &parseError);
                    if ( parseError.error != QJsonParseError::NoError ) {
                        logError(jsonData, parseError, key);
                        //qCWarning(fst_reader_json_logging)<< "Failed to parse JSON:" << parseError.errorString() << "at" << parseError.offset;
                    } else {
                        //qCDebug(fst_reader_json_logging) << "Parse ok, result:" << tempDoc;
                    }

                    if ( tempDoc.isObject() && !tempDoc.isEmpty() ) {
                        jsonValues.push_back(tempDoc.object());
                    } else if (tempDoc.isArray() && !tempDoc.isEmpty() ) {
                        jsonValues.push_back(tempDoc.array());
                    }
                }
            } else {
                if (! value.isNull() ) {
                    jsonValues.push_back(QJsonValue::fromVariant(value));
                }
            }
        }

        if ( values.count() > 1) {
            result.insert(key, jsonValues);
        } else {
            result.insert(key, jsonValues[0]);
        }
    }

    QJsonDocument doc;
    doc.setObject(result);
    return doc.toJson();
};


void FSTJsonReader::logError(const QByteArray &data, const QJsonParseError &error, const QString &field) {
    if ( error.error == QJsonParseError::NoError ) {
        qCWarning(fst_reader_json_logging) << "Trying to log error without an error!";
        return;
    }

    if (data.isEmpty()) {
        qCWarning(fst_reader_json_logging) << "Parse error" << error.errorString()
                                           << "in JSON at position" << error.offset
                                           << ( field.isEmpty() ? "" : "in field " + field )
                                           << ", because the JSON input was empty";
        return;
    }

    int min_start = std::max(error.offset - ERROR_CONTEXT_CHARACTERS, 0);
    int max_end   = std::min(error.offset + ERROR_CONTEXT_CHARACTERS, data.length());
    int start = error.offset;
    int end = error.offset;


    // Find start and end contextual info, limiting it at line boundaries
    for(start = error.offset; start>min_start; start--) {
        if ( data[start] == '\n' || data[start] == '\r' )
            break;
    }

    for(end = error.offset; end<max_end; end++) {
        if ( data[start] == '\n' || data[start] == '\r' )
            break;
    }

    QByteArray context = data.mid(start, end-start);
    qCWarning(fst_reader_json_logging) << "Parse error" << error.errorString()
                                       << "in " << data.length()
                                       << "bytes of JSON at position" << error.offset
                                       << ( field.isEmpty() ? "" : "in field " + field )
                                       << ", here:";
    qCWarning(fst_reader_json_logging) << context;
    qCWarning(fst_reader_json_logging) << QByteArray(" ", error.offset - start) + "^";
}