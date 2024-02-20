//
//  FSTTests.h
//  tests/model-serializers/src
//
//  Created by Dale Glass on 7/02/2024.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


// This test checks a large amount of files. To debug more comfortably and avoid going through
// a lot of uninteresting data, QTest allows us to narrow down what gets run with command line
// arguments, like this:
//
//     ./model-serializers-ModelSerializersTests loadGLTF:gltf2.0-RecursiveSkeletons.glb
//
// This will run only the loadGLTF test, and only on the gltf2.0-RecursiveSkeletons.glb file.

#include "FSTTests.h"
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

#include "FSTReader.h"
#include "FSTOldReader.h"
#include "FSTJsonReader.h"

QTEST_MAIN(FSTTests)

void FSTTests::initTestCase() {
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

void FSTTests::parseFSTOld_data() {
    // We feed a large amount of files into the test. Some we expect to fail, some we expect to load with issues. The
    // added columns below indicate our expectations for each file.

    QTest::addColumn<QString>("filename");
    QTest::addColumn<FileType>("fileType");
    QTest::addColumn<bool>("expectParseFail");

    QTest::newRow("avatar1")   << "fst/avatarStand.fst"                         << FileType::Old << false;
    QTest::newRow("object1")   << "fst/dome.fst"                                << FileType::Old << false;
    QTest::newRow("object2")   << "fst/logo_overte_white-color-emissive.fst"    << FileType::Old << false;
    QTest::newRow("object3")   << "fst/engery-bowl.fst"                         << FileType::Old << false;
    QTest::newRow("object4")   << "fst/standAngle_Applications.fst"             << FileType::Old << false;
    QTest::newRow("object5")   << "fst/standAngle_Avatar.fst"                   << FileType::Old << false;
    QTest::newRow("object6")   << "fst/standAngle_ConfigWizard.fst"             << FileType::Old << false;
    QTest::newRow("object7")   << "fst/standAngle_Controls.fst"                 << FileType::Old << false;
    QTest::newRow("object8")   << "fst/standAngle_TabletAndToolbar.fst"         << FileType::Old << false;
    QTest::newRow("object9")   << "fst/teleporter.fst"                          << FileType::Old << false;
    QTest::newRow("object10")  << "fst/test_area.fst"                           << FileType::Old << false;
    QTest::newRow("object11")  << "fst/avatar.fst"                              << FileType::Old << false;
    QTest::newRow("object12")  << "fst/FenFen.fst"                              << FileType::Old << false;
}

void FSTTests::parseFSTOld() {
    QFETCH(QString, filename);
    QFETCH(FileType, fileType);
    QFETCH(bool, expectParseFail);


    QFile gltf_file(filename);
    QVERIFY(gltf_file.open(QIODevice::ReadOnly));
    QByteArray data = gltf_file.readAll();
    QVERIFY(data.length() > 0);


    auto reader = FSTReader::getReader(data);
    QVERIFY(reader);

    // Verify that we detected the file type correctly by checking which derived
    // class we got.
    auto *readerPtr = reader.get();
    switch(fileType) {
        case FileType::Old:
            {
                auto *old = dynamic_cast<FSTOldReader*>(readerPtr);
                QVERIFY(old);
            }
            break;
        case FileType::JSON:
            {
                auto *json = dynamic_cast<FSTJsonReader*>(readerPtr);
                QVERIFY(json);
            }
            break;
    }


    FSTReader::Mapping mapping = reader->readMapping(data);
    QVERIFY((mapping.count() > 0) == (!expectParseFail));



    qInfo() << "Model:" << mapping;
}

void FSTTests::convertToJson_data() {
    // We feed a large amount of files into the test. Some we expect to fail, some we expect to load with issues. The
    // added columns below indicate our expectations for each file.

    QTest::addColumn<QString>("filename");

    QTest::newRow("avatarStand.fst")                        << "fst/avatarStand.fst";
    QTest::newRow("dome.fst")                               << "fst/dome.fst";
    QTest::newRow("logo_overte_white-color-emissive.fst")   << "fst/logo_overte_white-color-emissive.fst";
    QTest::newRow("objecengery-bowl.fstt3")                 << "fst/engery-bowl.fst";
    QTest::newRow("standAngle_Applications")                << "fst/standAngle_Applications.fst";
    QTest::newRow("standAngle_Avatar")                      << "fst/standAngle_Avatar.fst";
    QTest::newRow("standAngle_ConfigWizard")                << "fst/standAngle_ConfigWizard.fst";
    QTest::newRow("standAngle_Controls")                    << "fst/standAngle_Controls.fst";
    QTest::newRow("standAngle_TabletAndToolbar")            << "fst/standAngle_TabletAndToolbar.fst";
    QTest::newRow("teleporter.fst")                         << "fst/teleporter.fst";
    QTest::newRow("test_area.fst")                          << "fst/test_area.fst";
    QTest::newRow("avatar.fst")                             << "fst/avatar.fst";
    QTest::newRow("FenFen.fst")                             << "fst/FenFen.fst";
    QTest::newRow("test.fst")                               << "fst/test.fst";
}


void FSTTests::convertToJson() {
    QFETCH(QString, filename);

    QFile gltf_file(filename);
    QVERIFY(gltf_file.open(QIODevice::ReadOnly));
    QByteArray data = gltf_file.readAll();
    QVERIFY(data.length() > 0);


    auto reader = FSTReader::getReader(data);
    QVERIFY(reader);


    FSTReader::Mapping mapping = reader->readMapping(data);
    QVERIFY(mapping.count() > 0);


    QDir app_dir(QCoreApplication::applicationDirPath());
    QFileInfo fi(filename);

    app_dir.mkdir("out");


    FSTOldReader oldReader;
    QByteArray oldResult = oldReader.writeMapping(mapping);

    QFile oldout_file(QCoreApplication::applicationDirPath() + "/out/" + fi.baseName() + ".fst");
    oldout_file.open(QIODevice::WriteOnly);
    oldout_file.write(oldResult);
    oldout_file.close();



    FSTJsonReader jsonReader;
    QByteArray jsonResult = jsonReader.writeMapping(mapping);




    QFile out_file(QCoreApplication::applicationDirPath() + "/out/" + fi.baseName() + ".json");
    out_file.open(QIODevice::WriteOnly);
    out_file.write(jsonResult);
    out_file.close();

    qInfo() << "JSON version: " << jsonResult;


    auto mapping2 = jsonReader.readMapping(jsonResult);
    auto jsonResult2 = jsonReader.writeMapping(mapping2);

    QFile out_file2(QCoreApplication::applicationDirPath() + "/out/" + fi.baseName() + ".json2");
    out_file2.open(QIODevice::WriteOnly);
    out_file2.write(jsonResult2);
    out_file2.close();
}