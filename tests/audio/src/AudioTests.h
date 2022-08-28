//
//  AudioTests.h
//  tests/audio/src
//
//  Created by Dale Glass on 27/8/2022
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AudioTests_h
#define hifi_AudioTests_h

#include <QtTest/QtTest>



class AudioTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();

    void listAudioDevices();
};

#endif // hifi_AudioTests_h
