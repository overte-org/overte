//
//  GLTFSerializer.h
//  libraries/model-serializers/src
//
//  Created by Luis Cuenca on 8/30/17.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2023-2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_GLTFSerializer_h
#define hifi_GLTFSerializer_h

#include <memory.h>
#include <QtNetwork/QNetworkReply>
#include <hfm/ModelFormatLogging.h>
#include <hfm/HFMSerializer.h>

float atof_locale_independent(char* str);

#define CGLTF_ATOF(str) atof_locale_independent(str)

#include "cgltf.h"


class GLTFSerializer : public QObject, public HFMSerializer {
    Q_OBJECT
public:
    MediaType getMediaType() const override;
    std::unique_ptr<hfm::Serializer::Factory> getFactory() const override;

    HFMModel::Pointer read(const hifi::ByteArray& data, const hifi::VariantHash& mapping, const hifi::URL& url = hifi::URL()) override;
    ~GLTFSerializer();
private:
    cgltf_data* _data {nullptr};
    hifi::URL _url;
    QVector<hifi::ByteArray> _externalData;

    glm::mat4 getModelTransform(const cgltf_node& node);
    bool getSkinInverseBindMatrices(std::vector<std::vector<float>>& inverseBindMatrixValues);
    bool generateTargetData(cgltf_accessor *accessor, float weight, QVector<glm::vec3>& returnVector);

    bool buildGeometry(HFMModel& hfmModel, const hifi::VariantHash& mapping, const hifi::URL& url);

    bool readBinary(const QString& url, cgltf_buffer &buffer);

    void retriangulate(const QVector<int>& in_indices, const QVector<glm::vec3>& in_vertices,
                       const QVector<glm::vec3>& in_normals, QVector<int>& out_indices,
                       QVector<glm::vec3>& out_vertices, QVector<glm::vec3>& out_normals);

    std::tuple<bool, hifi::ByteArray> requestData(hifi::URL& url);
    hifi::ByteArray requestEmbeddedData(const QString& url);

    QNetworkReply* request(hifi::URL& url, bool isTest);

    void setHFMMaterial(HFMMaterial& hfmMat, const cgltf_material& material);
    HFMTexture getHFMTexture(const cgltf_texture *texture, cgltf_int texCoordSet);
};

#endif // hifi_GLTFSerializer_h
