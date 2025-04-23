#include "CryptographyScriptingInterface.h"
#include <qvariant.h>
#include <algorithm>
#include <iterator>
#include "ScriptEngineLogging.h"

QUuid CryptographyScriptingInterface::randomUUID() {
    return QUuid::createUuid();
}

CryptographyScriptingInterface::CryptographyScriptingInterface() : libctx(nullptr) {
    libctx = OSSL_LIB_CTX_new();
    if (!libctx) {
        qCritical(scriptengine) << "Failed to create OpenSSL library context.";
    }

    if (!OSSL_PROVIDER_load(libctx, "default")) {
        OSSL_LIB_CTX_free(libctx);
        qCritical(scriptengine) << "Failed to load default provider.";
    }
}

CryptographyScriptingInterface::~CryptographyScriptingInterface() {
    if (libctx) {
        OSSL_LIB_CTX_free(libctx);
    }
}

QVariant CryptographyScriptingInterface::generateRSAKeypair(const int bits) {
    QVariantMap keypair; // Map to hold the generated keys
    const int VALID_KEY_LENGTHS[] = {512, 1024, 2048, 3072, 4096};
    const bool IS_VALID_KEY_LENGTH = std::any_of(std::begin(VALID_KEY_LENGTHS), std::end(VALID_KEY_LENGTHS), [&](int i){return i==bits;});

    // Create a new context for RSA key generation
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_name(libctx, "RSA", nullptr);
    if (!ctx) {
        qCWarning(scriptengine) << "Failed to create RSA context.";
        return QVariant::fromValue(nullptr); // Return null
    }

    if (!IS_VALID_KEY_LENGTH) {
        qCWarning(scriptengine) << "Key length of '" << bits << "' is invalid.";
        return QVariant::fromValue(nullptr); // Return null
    }

    // Initialize the key generation process
    if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        qCWarning(scriptengine) << "Failed to initialize RSA key generation.";
        EVP_PKEY_CTX_free(ctx);
        return QVariant::fromValue(nullptr); // Return null
    }

    EVP_PKEY* pkey = nullptr; // Pointer to hold the generated key pair

    // Generate the RSA key pair
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        qCWarning(scriptengine) << "Failed to generate RSA key pair.";
        EVP_PKEY_CTX_free(ctx);
        return QVariant::fromValue(nullptr); // Return null
    }

    // Free the context after key generation
    EVP_PKEY_CTX_free(ctx);

    // Convert the public key to PEM format
    BIO* bioPublic = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bioPublic, pkey);
    BUF_MEM* bufferPublic;
    BIO_get_mem_ptr(bioPublic, &bufferPublic);
    keypair["public"] = QString::fromStdString(std::string(bufferPublic->data, bufferPublic->length));
    BIO_free(bioPublic);

    // Convert the private key to PEM format
    BIO* bioPrivate = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(bioPrivate, pkey, nullptr, nullptr, 0, nullptr, nullptr);
    BUF_MEM* bufferPrivate;
    BIO_get_mem_ptr(bioPrivate, &bufferPrivate);
    keypair["private"] = QString::fromStdString(std::string(bufferPrivate->data, bufferPrivate->length));
    BIO_free(bioPrivate);

    EVP_PKEY_free(pkey);

    return keypair; // Return the map containing the keys
}