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

    /*@jsdoc
    * Generates a signature for a given string message.
    * @function crypto.signRSAMessage
    * @param {string} [message] - The string message to provide a signature for.
    * @returns {?String} A valid signature for a given message.
    * @example <caption>Sign a message.</caption>
    */
    Q_INVOKABLE QString signRSAMessage(const QString message, const QString privateKeyPEM);


    Q_INVOKABLE bool validateRSASignature(const QString message, const QString hexSignature, const QString publicKeyPEM);
private: 
    EVP_PKEY* loadPrivateKeyFromPEM(const QString privateKeyPEM);
    EVP_PKEY* loadPublicKeyFromPEM(const QString publicKeyPEM);
    OSSL_LIB_CTX* libctx; // OpenSSL library context
};
