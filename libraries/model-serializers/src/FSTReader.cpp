#include <cctype>
#include <memory>


#include "FSTJsonReader.h"
#include "FSTOldReader.h"
#include "FSTReader.h"
#include "NetworkAccessManager.h"
#include "NetworkingConstants.h"

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>


Q_LOGGING_CATEGORY(fst_reader_logging, "overte.model-serializers.fst")


const QHash<FSTReader::ModelType, QString> FSTReader::_typesToNames =
{
    {ModelType::ENTITY_MODEL, "entity"},
    {ModelType::HEAD_MODEL, "head"},
    {ModelType::BODY_ONLY_MODEL, "body"},
    {ModelType::HEAD_AND_BODY_MODEL, "body+head"},
    {ModelType::ATTACHMENT_MODEL, "attachment"}
};

const QHash<QString, FSTReader::ModelType> FSTReader::_namesToTypes =
{
    {"entity", ModelType::ENTITY_MODEL},
    {"head", ModelType::HEAD_MODEL},
    {"body", ModelType::BODY_ONLY_MODEL},
    {"body+head",  ModelType::HEAD_AND_BODY_MODEL},
    // NOTE: this is not yet implemented, but will be used to allow you to attach fully independent models to your avatar
    {"attachment", ModelType::ATTACHMENT_MODEL}
};


const QStringList FSTReader::_preferredFieldOrder{NAME_FIELD, TYPE_FIELD, SCALE_FIELD, FILENAME_FIELD, TEXDIR_FIELD, SCRIPT_FIELD, JOINT_FIELD, BLENDSHAPE_FIELD, JOINT_INDEX_FIELD};

std::shared_ptr<FSTReader> FSTReader::getReader(const QByteArray &data) {
    if (data.length() >= 3) {
        if (data.at(0) == (char)0xd9 && data.at(1) == (char)0xd9 && data.at(2) == (char)0xf7) {
            // Looks like CBOR (Binary) JSON
            // https://en.wikipedia.org/wiki/CBOR
            return std::make_shared<FSTJsonReader>();
        }
    }

    for(int i=0;i<data.length();i++) {
        char c = data[i];

        if ( !std::isspace(c) && (c == '{' || c == '[') ) {
            // Looks like JSON.
            //
            // The old format is a 'key = value' type format, so there's no way
            // a valid file will ever begin with a [ or a {.
            return std::make_shared<FSTJsonReader>();
        } else {
            return std::make_shared<FSTOldReader>();
        }
    }

    qCWarning(fst_reader_logging) << "Failed to detect type of FST. Data:" << data.left(100);
    return nullptr;
}

FSTReader::ModelType FSTReader::predictModelType(const Mapping& mapping) {

    QVariantHash joints;

    if (mapping.contains("joint") && mapping.value("joint").type() == QVariant::Hash) {
        joints = mapping.value("joint").toHash();
    }

    // if the mapping includes the type hint... then we trust the mapping
    if (mapping.contains(TYPE_FIELD)) {
        return FSTReader::getTypeFromName(mapping.value(TYPE_FIELD).toString());
    }

    // check for blendshapes
    bool hasBlendshapes = mapping.contains(BLENDSHAPE_FIELD);

    // a Head needs to have these minimum fields...
    //joint = jointEyeLeft = EyeL = 1
    //joint = jointEyeRight = EyeR = 1
    //joint = jointNeck = Head = 1
    bool hasHeadMinimum = joints.contains("jointNeck") && joints.contains("jointEyeLeft") && joints.contains("jointEyeRight");

    // a Body needs to have these minimum fields...
    //joint = jointRoot = Hips
    //joint = jointLean = Spine
    //joint = jointNeck = Neck
    //joint = jointHead = HeadTop_End

    bool hasBodyMinimumJoints = joints.contains("jointRoot") && joints.contains("jointLean") && joints.contains("jointNeck")
                                        && joints.contains("jointHead");

    bool isLikelyHead = hasBlendshapes || hasHeadMinimum;

    if (isLikelyHead && hasBodyMinimumJoints) {
        return ModelType::HEAD_AND_BODY_MODEL;
    }

    if (isLikelyHead) {
        return ModelType::HEAD_MODEL;
    }

    if (hasBodyMinimumJoints) {
        return ModelType::BODY_ONLY_MODEL;
    }

    return ModelType::ENTITY_MODEL;
}


QVector<QString> FSTReader::getScripts(const QUrl& url, const Mapping& mapping) {

    auto fstMapping = mapping.isEmpty() ? downloadMapping(url.toString()) : mapping;
    QVector<QString> scriptPaths;
    if (!fstMapping.value(SCRIPT_FIELD).isNull()) {
        auto scripts = fstMapping.values(SCRIPT_FIELD).toVector();
        for (auto &script : scripts) {
            QString scriptPath = script.toString();
            if (QUrl(scriptPath).isRelative()) {
                if (scriptPath.at(0) == '/') {
                    scriptPath = scriptPath.right(scriptPath.length() - 1);
                }
                scriptPath = url.resolved(QUrl(scriptPath)).toString();
            }
            scriptPaths.push_back(scriptPath);
        }
    }
    return scriptPaths;
}


FSTReader::Mapping FSTReader::downloadMapping(const QString& url) {
    QNetworkAccessManager& networkAccessManager = NetworkAccessManager::getInstance();
    QNetworkRequest networkRequest = QNetworkRequest(url);
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
    QNetworkReply* reply = networkAccessManager.get(networkRequest);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    QByteArray fstContents = reply->readAll();
    delete reply;

    auto reader = getReader(fstContents);
    if (reader) {
        return reader->readMapping(fstContents);
    } else {
        qWarning(fst_reader_logging) << "Failed to identify FST format for" << url;
        return FSTReader::Mapping();
    }
}


static void removeBlendshape(QVariantHash& bs, const QString& key) {
    if (bs.contains(key)) {
        bs.remove(key);
    }
}

static void splitBlendshapes(hifi::VariantMultiHash& bs, const QString& key, const QString& leftKey, const QString& rightKey) {
    if (bs.contains(key) && !(bs.contains(leftKey) || bs.contains(rightKey))) {
        // key has been split into leftKey and rightKey blendshapes
        QVariantList origShapes = bs.values(key);
        QVariantList halfShapes;
        for (int i = 0; i < origShapes.size(); i++) {
            QVariantList origShape = origShapes[i].toList();
            QVariantList halfShape;
            halfShape.append(origShape[0]);
            halfShape.append(QVariant(0.5f * origShape[1].toFloat()));
            bs.insert(leftKey, halfShape);
            bs.insert(rightKey, halfShape);
        }
    }
}

void FSTReader::fixUpLegacyBlendshapes(Mapping &properties) {
    hifi::VariantMultiHash bs = properties.value("bs").toHash();

    // These blendshapes have no ARKit equivalent, so we remove them.
    removeBlendshape(bs, "JawChew");
    removeBlendshape(bs, "ChinLowerRaise");
    removeBlendshape(bs, "ChinUpperRaise");
    removeBlendshape(bs, "LipsUpperOpen");
    removeBlendshape(bs, "LipsLowerOpen");

    // These blendshapes are split in ARKit, we replace them with their left and right sides with a weight of 1/2.
    splitBlendshapes(bs, "LipsUpperUp", "MouthUpperUp_L", "MouthUpperUp_R");
    splitBlendshapes(bs, "LipsLowerDown", "MouthLowerDown_L", "MouthLowerDown_R");
    splitBlendshapes(bs, "Sneer", "NoseSneer_L", "NoseSneer_R");

    // re-insert new mutated bs hash into mapping properties.
    properties.insert("bs", bs);
}
