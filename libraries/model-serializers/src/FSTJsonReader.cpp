
#include "FSTJsonReader.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>

Q_LOGGING_CATEGORY(fst_reader_json_logging, "overte.model-serializers.fst.json")


QSet<QString> FSTJsonReader::_jsonFields{"materialMap", "flowPhysicsData", "flowCollisionsData"};

FSTReader::Mapping FSTJsonReader::readMapping(const QByteArray& data) {
    return Mapping();
};


QByteArray FSTJsonReader::writeMapping(const FSTReader::Mapping& mapping) {
    QJsonObject result;


    for (auto key : mapping.uniqueKeys()) {

        auto values = mapping.values(key);
        qInfo() << "Converting " << key << "with" << values.size() << "values: " << values;

        if ( values.count() == 0 ) {
            continue;
        }


        QJsonArray jsonValues;

        for(auto value : values) {
            if ( _jsonFields.contains(key) ) {
                qInfo() << "Parsing JSON key: " << key << ":" << values.at(0);
                QJsonParseError parseError;
                QJsonDocument tempDoc = QJsonDocument::fromJson(values.at(0).toByteArray(), &parseError);
                if ( parseError.error != QJsonParseError::NoError ) {
                    qWarning() << "Failed to parse JSON:" << parseError.errorString();
                } else {
                    qInfo() << "Parse ok, result:" << tempDoc;
                }

                if ( tempDoc.isObject() && !tempDoc.isEmpty() ) {
                    jsonValues.push_back(tempDoc.object());
                } else if (tempDoc.isArray() && !tempDoc.isEmpty() ) {
                    jsonValues.push_back(tempDoc.array());
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
