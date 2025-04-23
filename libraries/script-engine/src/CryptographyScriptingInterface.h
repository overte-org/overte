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

    /*@jsdoc
    * Generates an RSA key pair based on a given amount of bits.
    * @function crypto.generateRSAKeypair
    * @param {number} [bits=2048] - The total bits of the RSA key to use.
    * @returns {?Object} An object containing both the public and private keys, or null in the event of an error.
    * @example <caption>Generate a new key pair.</caption>
    * const rsaKeyPair = crypto.generateRSAKeypair();
    * if (rsaKeyPair) {
    *     console.log(rsaKeyPair.public);
    *     console.log(rsaKeyPair.private);
    * } else {
    *     console.log("Failed to generate RSA key pair.");
    * }
    */
    Q_INVOKABLE QVariant generateRSAKeypair(const int bits = 2048);
private: 
    OSSL_LIB_CTX* libctx; // OpenSSL library context
};
