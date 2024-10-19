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

#pragma once

#include <QtTest/QtTest>

class FSTTests : public QObject {
    Q_OBJECT
public:

    enum class FileType {
        Old,
        JSON
    };

    void writeFile(const QString &filename, const QByteArray &data);

private slots:
    void initTestCase();
    void parseFSTOld_data();
    void parseFSTOld();

    void convertToJson_data();
    void convertToJson();

};

Q_DECLARE_METATYPE(FSTTests::FileType)
