#pragma once

#include <QtCore/QObject>
#include <QUuid>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/provider.h>

class CryptographyScriptingInterface : public QObject {
    Q_OBJECT
public:
    CryptographyScriptingInterface();
    ~CryptographyScriptingInterface();

    Q_INVOKABLE static QUuid randomUUID();
    Q_INVOKABLE QVariant generateRSAKeypair(const int bits = 2048);
private: 
    OSSL_LIB_CTX* libctx; // OpenSSL library context
};
