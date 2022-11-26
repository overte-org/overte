//
//  AABoxCubeTests.h
//  tests/octree/src
//
//  Created by Brad Hefta-Gaub on 06/04/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ModelSerializersTests.h"
#include "GLTFSerializer.h"
#include "FBXSerializer.h"
#include "OBJSerializer.h"

#include "Gzip.h"
#include "model-networking/ModelLoader.h"
#include <hfm/ModelFormatRegistry.h>
#include "DependencyManager.h"
#include "ResourceManager.h"
#include "AssetClient.h"
#include "LimitedNodeList.h"
#include "NodeList.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QDebug>


QTEST_MAIN(ModelSerializersTests)

void ModelSerializersTests::initTestCase() {
    qRegisterMetaType<QNetworkReply*>("QNetworkReply*");

    DependencyManager::registerInheritance<LimitedNodeList, NodeList>();
    DependencyManager::set<NodeList>(NodeType::Agent, INVALID_PORT);

    DependencyManager::set<ModelFormatRegistry>(); // ModelFormatRegistry must be defined before ModelCache. See the ModelCache constructor.
    DependencyManager::set<ResourceManager>();
    DependencyManager::set<AssetClient>();



    auto modelFormatRegistry = DependencyManager::get<ModelFormatRegistry>();
    modelFormatRegistry->addFormat(FBXSerializer());
    modelFormatRegistry->addFormat(OBJSerializer());
    modelFormatRegistry->addFormat(GLTFSerializer());
}

void ModelSerializersTests::loadGLTF_data() {
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("expectWarnings");
    QTest::addColumn<bool>("expectErrors");

    QTest::newRow("crash1") << "635d84711260644e7e393e0b.glb.gz" << false << true;
    QTest::newRow("crash2") << "dude.glb.gz" << false << true;

}

void ModelSerializersTests::loadGLTF() {
    QFETCH(QString, filename);
    QFETCH(bool, expectWarnings);
    QFETCH(bool, expectErrors);


    QFile gltf_file(filename);
    QVERIFY(gltf_file.open(QIODevice::ReadOnly));

    QByteArray data = gltf_file.readAll();
    QByteArray uncompressedData;
    QUrl url("https://example.com");

    qInfo() << "URL: " << url;

    if (filename.toLower().endsWith(".gz")) {
        url.setPath("/" + filename.chopped(3));

        if (gunzip(data, uncompressedData)) {
            qInfo() << "Uncompressed into" << uncompressedData.length();
        } else {
            qCritical() << "Failed to uncompress";
        }
    } else {
        url.setPath("/" + filename);
        uncompressedData = data;
    }


    ModelLoader loader;
    QMultiHash<QString, QVariant> serializerMapping;
    std::string webMediaType;

    serializerMapping.insert("combineParts", true);
    serializerMapping.insert("deduplicateIndices", true);

    qInfo() << "Loading model from" << uncompressedData.length() << "bytes data, url" << url;

    // Check that we can find a serializer for this
    auto serializer = DependencyManager::get<ModelFormatRegistry>()->getSerializerForMediaType(uncompressedData, url, webMediaType);
    QVERIFY(serializer);



    hfm::Model::Pointer model = loader.load(uncompressedData, serializerMapping, url, webMediaType);
    QVERIFY(model);
    QVERIFY(!model->meshes.empty());
    QVERIFY(!model->joints.empty());

    qInfo() << "Model was loaded with" << model->meshes.count() << "meshes and" << model->joints.count() << "joints. Found" << model->loadWarningCount << "warnings and" << model->loadErrorCount << "errors";

    // Some models we test are expected to be broken. We're testing that we can load the model without blowing up,
    // so loading it with errors is still a successful test.
    QVERIFY(expectWarnings == (model->loadWarningCount>0));
    QVERIFY(expectErrors == (model->loadErrorCount>0));
}
