//
//  Created by Bradley Austin Davis on 2016/07/01
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QObject>

class KtxBenchmarks : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void benchmarkPNG_data();
    void benchmarkPNG();
    void benchmarkJPG_data();
    void benchmarkJPG();

    void benchmarkCreateTexture();
    void benchmarkSerializeTexture();
    void benchmarkWriteKTX();
};


