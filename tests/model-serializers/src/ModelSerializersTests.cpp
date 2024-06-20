//
//  ModelSerializersTests.cpp
//  tests/model-serializers/src
//
//  Created by Dale Glass on 20/11/2022.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html


// This test checks a large amount of files. To debug more comfortably and avoid going through
// a lot of uninteresting data, QTest allows us to narrow down what gets run with command line
// arguments, like this:
//
//     ./model-serializers-ModelSerializersTests loadGLTF:gltf2.0-RecursiveSkeletons.glb
//
// This will run only the loadGLTF test, and only on the gltf2.0-RecursiveSkeletons.glb file.

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
#include <QDirIterator>

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
    // We feed a large amount of files into the test. Some we expect to fail, some we expect to load with issues. The
    // added columns below indicate our expectations for each file.


    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("expectParseFail");
    QTest::addColumn<bool>("expectWarnings");
    QTest::addColumn<bool>("expectErrors");

    QTest::newRow("ready-player-me-good1")   << "models/src/DragonAvatar1.glb.gz"              << false << false << false;
    QTest::newRow("ready-player-me-good2")   << "models/src/UkraineFranny.glb.gz"              << false << false << false;
    QTest::newRow("ready-player-me-good3")   << "models/src/Franny.glb.gz"                     << false << false << false;
    QTest::newRow("ready-player-me-good4")   << "models/src/womanInTShirt.glb.gz"              << false << false << false;
    QTest::newRow("ready-player-me-good5")   << "models/src/female-avatar-with-swords.glb.gz"  << false << false << false;
    QTest::newRow("ready-player-me-broken1") << "models/src/broken-2022-11-27.glb.gz" << false << false << false;


    // We can't parse GLTF 1.0 at present, and probably not ever. We're expecting all these to fail.
    /*QDirIterator it("models/src/gltf_samples/1.0", QStringList() << "*.glb", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString filename = it.next();
        QFileInfo fi(filename);
        QString testname = "gltf1.0-" + fi.fileName();
        QTest::newRow(testname.toUtf8().data()) << filename << true << false << false;
    }*/

    QDirIterator it2("models/src/gltf_samples/2.0", QStringList() << "*.glb", QDir::Files, QDirIterator::Subdirectories);
    while(it2.hasNext()) {
        QString filename = it2.next();
        QFileInfo fi(filename);
        QString testname = "gltf2.0-" + fi.fileName();
        QTest::newRow(testname.toUtf8().data()) << filename << false << false << false;
    }

}

void ModelSerializersTests::loadGLTF() {
    QFETCH(QString, filename);
    QFETCH(bool, expectParseFail);
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
    QVERIFY(expectParseFail == !model);

    if (!model) {
        // We expected this parse to fail, so nothing more to do here.
        return;
    }

    QVERIFY(!model->meshes.empty());
    QVERIFY(!model->joints.empty());

    qInfo() << "Model was loaded with" << model->meshes.count() << "meshes and" << model->joints.count() << "joints. Found" << model->loadWarningCount << "warnings and" << model->loadErrorCount << "errors";

    // Some models we test are expected to be broken. We're testing that we can load the model without blowing up,
    // so loading it with errors is still a successful test.
    QVERIFY(expectWarnings == (model->loadWarningCount>0));
    QVERIFY(expectErrors == (model->loadErrorCount>0));
}
