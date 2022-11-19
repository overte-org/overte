#include <QSignalSpy>
#include <QDebug>

#include "AudioTests.h"
#include "AudioClient.h"
#include "DependencyManager.h"
#include "NodeList.h"
#include "plugins/CodecPlugin.h"
#include "plugins/PluginManager.h"

QTEST_MAIN(AudioTests)


Q_DECLARE_METATYPE(QList<HifiAudioDeviceInfo>)




void AudioTests::initTestCase() {
    // AudioClient starts networking, but for the purposes of the tests here we don't care,
    // so just got to use some port.
    int listenPort = 10000;

    DependencyManager::registerInheritance<LimitedNodeList, NodeList>();
    DependencyManager::set<NodeList>(NodeType::Agent, listenPort);
    DependencyManager::set<AudioClient>();
    DependencyManager::set<PluginManager>();
    QSharedPointer<AudioClient> ac = DependencyManager::get<AudioClient>();

    qRegisterMetaType<QList<HifiAudioDeviceInfo>>();

    ac->startThread();
}

void AudioTests::listAudioDevices() {
    QSharedPointer<AudioClient> ac = DependencyManager::get<AudioClient>();
    QVERIFY(!ac.isNull());

/*
    // AudioClient::devicesChanged is declared as:
    // void devicesChanged(QAudio::Mode mode, const QList<HifiAudioDeviceInfo>& devices);
    //
    // Unfortunately with QSignalSpy we have to use the old SIGNAL() syntax, so it was a bit tricky
    // to figure out how to get the signal to connect. The snippet below lists signals in the format
    // that Qt understands. Turns out we lose the 'const &'.

    const QMetaObject *mo = ac->metaObject();
    QList<QString> signalSignatures;

    // Start from MyClass members
    for(int methodIdx = mo->methodOffset(); methodIdx < mo->methodCount(); ++methodIdx) {
        QMetaMethod mmTest = mo->method(methodIdx);
        switch((int)mmTest.methodType()) {
            case QMetaMethod::Signal:
            signalSignatures.append(QString(mmTest.methodSignature()));
            qDebug() << "SIG: " << QString(mmTest.methodSignature());
            break;
        }
    }
*/

    QSignalSpy spy(ac.get(), SIGNAL(devicesChanged(QAudio::Mode,QList<HifiAudioDeviceInfo>)));

    QVERIFY(spy.isValid()); // This checks that the signal has connected
    spy.wait(15000);

    // We always get two events here, one for audio input, and one for output,
    // but signals keep coming and we could potentially get more repetitions.
    QVERIFY(spy.count() > 0);
    qDebug() << "Received" << spy.count() << "device events";

    // QSignalSpy is a QList, which stores the received signals. We can then examine it to see
    // what we got.
    for(auto event : spy) {
        QAudio::Mode mode = qvariant_cast<QAudio::Mode>(event.at(0));
        QList<HifiAudioDeviceInfo> devs = qvariant_cast<QList<HifiAudioDeviceInfo>>(event.at(1));

        QVERIFY(devs.count() > 0);

        qDebug() << "Mode:" << mode;
        for(auto dev : devs) {
            qDebug() << "\t" << dev.deviceName();
        }
    }


}
