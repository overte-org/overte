//
//  FSTReader.h
//
//
//  Created by Clement on 3/26/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_FSTReader_h
#define hifi_FSTReader_h

#include <QBuffer>
#include <QVariantHash>
#include <memory>
#include <QDebug>
#include <QLoggingCategory>

#include "shared/HifiTypes.h"

Q_DECLARE_LOGGING_CATEGORY(fst_reader_logging)

static const unsigned int FST_VERSION = 1;
static const QString FST_VERSION_FIELD = "version";
static const QString NAME_FIELD = "name";
static const QString TYPE_FIELD = "type";
static const QString FILENAME_FIELD = "filename";
static const QString TEXDIR_FIELD = "texdir";
static const QString LOD_FIELD = "lod";
static const QString JOINT_INDEX_FIELD = "jointIndex";
static const QString SCALE_FIELD = "scale";
static const QString TRANSLATION_X_FIELD = "tx";
static const QString TRANSLATION_Y_FIELD = "ty";
static const QString TRANSLATION_Z_FIELD = "tz";
static const QString JOINT_FIELD = "joint";
static const QString BLENDSHAPE_FIELD = "bs";
static const QString SCRIPT_FIELD = "script";
static const QString JOINT_NAME_MAPPING_FIELD = "jointMap";
static const QString MATERIAL_MAPPING_FIELD = "materialMap";
static const QString COMMENT_FIELD = "comment";


/**
 * @brief Reads and writes avatar metadata files
 * 
 * There are two formats: The old .fst key/value pair (sort of) format, and
 * the new, JSON based format.
 * 
 * The old one is a non-standard format with parsing problems and should be
 * avoided for future usage.
 * 
 * The mapping is stored in a variant multi-hash, and the same structure for
 * every type. The methods for dealing with it are all in this class.
 * 
 * The derived classes only deal with file format specifics.
*/
class FSTReader {
public:

    FSTReader() {};

    virtual ~FSTReader() {};

    enum class ModelType {
        ENTITY_MODEL = 0,
        HEAD_MODEL,
        BODY_ONLY_MODEL,
        HEAD_AND_BODY_MODEL,
        ATTACHMENT_MODEL
    };



    // TODO: Probably move this to FST.h
    using Mapping = hifi::VariantMultiHash;



    /**
     * @brief Constructs a reader for the detected format in the byte array.
     * 
     * The detection is based on checking whether the contents look like
     * JSON (the first non-whitepace character is a { or a [)
     * 
     * CBOR is supported by recognizing the magic d9d9f7 sequence:
     * https://en.wikipedia.org/wiki/CBOR#Semantic_tag_(type_6)
     * 
     * Valid CBOR may not be recognized because the presence of this tag
     * is optional.
     * 
     * @param data 
     * @return std::shared_ptr<FSTReader> 
     */
    static std::shared_ptr<FSTReader> getReader(const QByteArray &data);
    


    /**
     * @brief  Reads an FST mapping from the supplied data.
     * 
     * @param data 
     * @return Mapping
     */
    virtual Mapping readMapping(const QByteArray& data) { return Mapping(); };


    /**
     * @brief Writes an FST mapping to a byte array.
     * 
     * @param mapping 
     * @return QByteArray 
     */
    virtual QByteArray writeMapping(const Mapping& mapping) { return QByteArray(); };

    /**
     * @brief Predicts the type of model by examining the mapping
     * 
     * @param mapping Mapping to examine
     * @return ModelType Likely model type. Defaults to ModelType::ENTITY_MODEL if nothing fits.
     */
    static ModelType predictModelType(const Mapping& mapping);

    // TODO: Why not a QStringList?

    /**
     * @brief Get a list of script paths based on a base URL and a mapping
     * 
     * @param fstUrl Mapping's URL
     * @param mapping Mapping. If not specified, the URL will be downloaded and parsed.
     * @return QVector<QString> List of URLs 
     */
    static QVector<QString> getScripts(const QUrl& fstUrl, const Mapping& mapping = QVariantHash());


    /**
     * @brief Get a human-readable name for a model type
     * 
     * @param modelType 
     * @return QString 
     */
    static QString getNameFromType(ModelType modelType) { return _typesToNames.value(modelType, QString()); };

    /**
     * @brief Get the model type from a human readable name
     * 
     * @param name 
     * @return FSTReader::ModelType 
     */
    static FSTReader::ModelType getTypeFromName(const QString& name) { return _namesToTypes.value(name); }

    /**
     * @brief Downloads a mapping from the specified URL
     * 
     * Automatically parses the result with either parser.
     * 
     * @param url URL to download 
     * @return Mapping Resulting mapping, or an empty one on error.
     */
    static Mapping downloadMapping(const QString& url);

protected:
    /**
     * @brief Convert legacy blendshapes to ARKit blendshapes
     * 
     * See https://arkit-face-blendshapes.com/
     * @param properties 
     */
    void fixUpLegacyBlendshapes(Mapping &properties);

private:
    static const QHash<FSTReader::ModelType, QString> _typesToNames;
    static const QHash<QString, FSTReader::ModelType> _namesToTypes;   
};


    inline uint qHash(FSTReader::ModelType key, uint seed) {
        return ::qHash(static_cast<uint>(key), seed);
    }


#endif // hifi_FSTReader_h