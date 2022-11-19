#include <QSignalSpy>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>


#include "CodecTests.h"
#include "AudioClient.h"
#include "DependencyManager.h"
#include "NodeList.h"
#include "plugins/CodecPlugin.h"
#include "plugins/PluginManager.h"

QTEST_MAIN(CodecTests)




void CodecTests::initTestCase() {
    DependencyManager::set<PluginManager>();

    QDir testPath (QCoreApplication::applicationDirPath());
    QDir interfacePluginPath = testPath;



    qDebug() << "Our directory is" << testPath;

    interfacePluginPath.cdUp();
    interfacePluginPath.cdUp();
    interfacePluginPath.cd("interface");
    interfacePluginPath.cd("plugins");
    interfacePluginPath.makeAbsolute();

    QString ourPluginPath = testPath.filePath("plugins");


    qDebug() << "Interface plugins are at" << interfacePluginPath;
    qDebug() << "Our plugins are at" << ourPluginPath;


    QFile::link(interfacePluginPath.path(), ourPluginPath);

}

void CodecTests::loadCodecs() {
    const auto& codecPlugins = PluginManager::getInstance()->getCodecPlugins();

    QVERIFY(codecPlugins.size() > 0);


    for (const auto& plugin : codecPlugins) {
        auto codecName = plugin->getName();
        qDebug() << "Codec:" << codecName << ", supported=" << plugin->isSupported();
    }
}



void CodecTests::testEncoders() {
    const auto& codecPlugins = PluginManager::getInstance()->getCodecPlugins();

    QVERIFY(codecPlugins.size() > 0);


    for (const auto& plugin : codecPlugins) {
        if (!plugin->isSupported()) {
            qWarning() << "Skipping unsupported plugin" << plugin->getName();
            continue;
        }

        Encoder* encoder = plugin->createEncoder(AudioConstants::SAMPLE_RATE, AudioConstants::STEREO);
        QVERIFY(encoder != nullptr);

        QByteArray data(AudioConstants::NETWORK_FRAME_BYTES_STEREO, 0);
        QByteArray encoded;

        encoder->encode(data, encoded);

        QVERIFY(encoded.size() > 0);

        qDebug() << "Codec" << plugin->getName() << "encoded empty buffer of" << data.size() << "bytes into" << encoded.size();
    }
}

void CodecTests::testDecoders () {
    const auto& codecPlugins = PluginManager::getInstance()->getCodecPlugins();

    QVERIFY(codecPlugins.size() > 0);


    for (const auto& plugin : codecPlugins) {
        if (!plugin->isSupported()) {
            qWarning() << "Skipping unsupported plugin" << plugin->getName();
            continue;
        }

        Encoder* encoder = plugin->createEncoder(AudioConstants::SAMPLE_RATE, AudioConstants::STEREO);
        Decoder* decoder = plugin->createDecoder(AudioConstants::SAMPLE_RATE, AudioConstants::STEREO);
        QVERIFY(encoder != nullptr);

        QByteArray data(AudioConstants::NETWORK_FRAME_BYTES_STEREO, 0);
        QByteArray encoded;
        QByteArray decoded;
        QByteArray lost(AudioConstants::NETWORK_FRAME_BYTES_STEREO, 0);


        encoder->encode(data, encoded);
        decoder->decode(encoded, decoded);


        QVERIFY(encoded.size() > 0);
        QVERIFY(decoded.size() > 0);
        QVERIFY(decoded.size() == data.size());
        QVERIFY(lost.size() > 0);


        qDebug() << "Codec" << plugin->getName() << "encoded empty buffer of" << data.size() << "bytes into" << encoded.size() << "and decoded back into" << decoded.size();

        // This is here mostly for valgrind testing -- we can't really validate anything, but we can see if it crashes.
        decoder->lostFrame(lost);
        QVERIFY(lost.size() > 0);
        qDebug() << "Codec" << plugin->getName() << "decoded a lost frame";
    }
}
