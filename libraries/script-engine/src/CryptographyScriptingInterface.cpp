#include "CryptographyScriptingInterface.h"
#include <openssl/err.h>
#include <qvariant.h>
#include <algorithm>
#include <iterator>
#include "ScriptEngineLogging.h"

// TODO: RSA encryption without padding
// TODO: RSA encryption PKCS #1 / legacy padding (for funsies)

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

// Creates a valid EVP_PKEY from a QString PEM formatted private key.
EVP_PKEY* CryptographyScriptingInterface::loadPrivateKeyFromPEM(const QString privateKeyPEM) {
    QString trimmedPrivateKeyPEM = privateKeyPEM.trimmed();
    // Convert QString to QByteArray
    QByteArray pemBytes = trimmedPrivateKeyPEM.toUtf8();

    // Create a BIO to read the PEM data
    BIO* bio = BIO_new_mem_buf(pemBytes.constData(), pemBytes.length());
    if (!bio) {
        qCWarning(scriptengine) << "Failed to create BIO.";
        return nullptr;
    }

    // Read the private key from the BIO
    EVP_PKEY* privateKey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio); // Free the BIO after use

    if (!privateKey) {
        qCWarning(scriptengine) << "Failed to read private key from PEM.";
        return nullptr;
    }

    return privateKey; // Return the loaded private key
}

// Creates a valid EVP_PKEY from a QString PEM formatted public key.
EVP_PKEY* CryptographyScriptingInterface::loadPublicKeyFromPEM(const QString publicKeyPEM) {
    QString trimmedPublicKeyPEM = publicKeyPEM.trimmed();

    // Convert QString to QByteArray
    QByteArray pemBytes = trimmedPublicKeyPEM.toUtf8();

    BIO* bio = BIO_new_mem_buf(pemBytes.constData(), pemBytes.length());
    if (!bio) {
        qCWarning(scriptengine) << "Failed to create BIO.";
        return nullptr;
    }

    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!publicKey) {
        qCWarning(scriptengine) << "Failed to read public key from PEM.";
        return nullptr;
    }

    return publicKey;
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

QString CryptographyScriptingInterface::signRSAMessage(const QString message, const QString privateKeyPEM) {
    EVP_PKEY* privateKey = CryptographyScriptingInterface::loadPrivateKeyFromPEM(privateKeyPEM);

    // Create a context for signing
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        qCWarning(scriptengine) << "Failed to create context for signing.";
        return NULL;
    }

    // Initialize the signing operation
    if (EVP_SignInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        qCWarning(scriptengine) << "Failed to initialize signing.";
        return NULL;
    }

    // Convert QString to expected string type
    QByteArray messageBytes = message.toUtf8();
    const char* messageCStr = messageBytes.constData();

    // Update the context with the message
    if (EVP_SignUpdate(ctx, messageCStr, message.length()) != 1) {
        EVP_MD_CTX_free(ctx);
        qCWarning(scriptengine) << "Failed to update signing context.";
        return NULL;
    }

    int keySizeBits = EVP_PKEY_bits(privateKey);
    unsigned int signatureLength = keySizeBits / 8;
    std::vector<unsigned char> signature(signatureLength); // Create dynamic buffer from the signature length

    // Finalize the signing operation
    if (EVP_SignFinal(ctx, signature.data(), &signatureLength, privateKey) != 1) {
        EVP_MD_CTX_free(ctx);
        qCWarning(scriptengine) << "Failed to finalize signing.";
        return NULL;
    }

    EVP_MD_CTX_free(ctx); // Clean up the context

    // Convert signature to a hex string for easier handling
    QString hexSignature;
    for (unsigned int i = 0; i < signatureLength; i++) {
        char buffer[3];
        snprintf(buffer, sizeof(buffer), "%02x", signature[i]);
        hexSignature += buffer;
    }

    return hexSignature; // Return the signature as a hex string
}

bool CryptographyScriptingInterface::validateRSASignature(const QString message, const QString hexSignature, const QString publicKeyPEM) {
    // Load the public key from the PEM format
    EVP_PKEY* publicKey = loadPublicKeyFromPEM(publicKeyPEM);

    if (!publicKey) {
        qCWarning(scriptengine) << "Invalid public key.";
        return false;
    }

    // Create a context for verification
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        qCWarning(scriptengine) << "Failed to create context for verification.";
        EVP_PKEY_free(publicKey);
        return false;
    }

    // Initialize the verification operation
    if (EVP_VerifyInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        qCWarning(scriptengine) << "Failed to initialize verification.";
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        return false;
    }

    // Convert QString to C-style string
    QByteArray messageBytes = message.toUtf8();
    const char* messageCStr = messageBytes.constData();

    // Update the context with the message
    if (EVP_VerifyUpdate(ctx, messageCStr, messageBytes.length()) != 1) {
        qCWarning(scriptengine) << "Failed to update verification context.";
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        return false;
    }

    int keySizeBits = EVP_PKEY_bits(publicKey);
    unsigned int signatureLength = keySizeBits / 8;
    std::vector<unsigned char> signature(signatureLength); // Create dynamic buffer from the signature length

    for (unsigned int i = 0; i < signatureLength; i++) {
        std::string byteString = hexSignature.mid(i * 2, 2).toStdString();
        signature[i] = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
    }

    // Finalize the verification
    int result = EVP_VerifyFinal(ctx, signature.data(), signatureLength, publicKey);
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(publicKey);

    return result == 1;
}

QString CryptographyScriptingInterface::encryptRSA(const QString message, const QString publicKeyPEM) {
    // Load the public key from the PEM format
    EVP_PKEY* publicKey = loadPublicKeyFromPEM(publicKeyPEM);

    if (!publicKey) {
        qCWarning(scriptengine) << "Failed to read public key from PEM.";
        return NULL;
    }

    // Create a context for encryption
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
    if (!ctx) {
        qCWarning(scriptengine) << "Failed to create context for encryption.";
        EVP_PKEY_free(publicKey);
        return NULL;
    }

    // Initialize the encryption operation
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        qCWarning(scriptengine) << "Failed to initialize encryption.";
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        return NULL;
    }

    // Set padding (if needed)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        qCWarning(scriptengine) << "Failed to set padding.";
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        return NULL;
    }

    // Prepare the message for encryption
    QByteArray messageBytes = message.toUtf8();
    size_t encryptedLength;

    // Determine the size of the encrypted message
    if (EVP_PKEY_encrypt(ctx, nullptr, &encryptedLength, reinterpret_cast<const unsigned char*>(messageBytes.constData()), messageBytes.length()) <= 0) {
        qCWarning(scriptengine) << "Failed to determine encrypted length.";
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        return NULL;
    }

    // Allocate buffer for the encrypted message
    std::vector<unsigned char> encrypted(encryptedLength);

    // Encrypt the message
    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &encryptedLength, reinterpret_cast<const unsigned char*>(messageBytes.constData()), messageBytes.length()) <= 0) {
        qCWarning(scriptengine) << "Failed to encrypt message: " << ERR_error_string(ERR_get_error(), nullptr);
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        return NULL;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(publicKey);

    // Convert encrypted data to Base64 for easier handling
    QByteArray base64Encrypted = QByteArray(reinterpret_cast<const char*>(encrypted.data()), encryptedLength).toBase64();

    return QString(base64Encrypted);; // Return the encrypted message as a hex string
}

QString CryptographyScriptingInterface::decryptRSA(const QString base64Encrypted, const QString privateKeyPEM) {
    // Load the private key from the PEM format
    EVP_PKEY* privateKey = loadPrivateKeyFromPEM(privateKeyPEM);

    if (!privateKey) {
        qCWarning(scriptengine) << "Failed to read private key from PEM.";
        return NULL;
    }

    // Create a context for decryption
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
    if (!ctx) {
        qCWarning(scriptengine) << "Failed to create context for decryption.";
        EVP_PKEY_free(privateKey);
        return NULL;
    }

    // Initialize the decryption operation
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        qCWarning(scriptengine) << "Failed to initialize decryption.";
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(privateKey);
        return NULL;
    }

    // Set padding
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        qCWarning(scriptengine) << "Failed to set padding.";
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(privateKey);
        return NULL;
    }

    // Decode the Base64 string back to binary
    QByteArray encryptedData = QByteArray::fromBase64(base64Encrypted.toUtf8());
    size_t decryptedLength;

    // Determine the size of the decrypted message
    if (EVP_PKEY_decrypt(ctx, nullptr, &decryptedLength, reinterpret_cast<const unsigned char*>(encryptedData.constData()), encryptedData.length()) <= 0) {
        qCWarning(scriptengine) << "Failed to determine decrypted length.";
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(privateKey);
        return NULL;
    }

    // Allocate buffer for the decrypted message
    std::vector<unsigned char> decrypted(decryptedLength);

    // Decrypt the message
    if (EVP_PKEY_decrypt(ctx, decrypted.data(), &decryptedLength, reinterpret_cast<const unsigned char*>(encryptedData.constData()), encryptedData.length()) <= 0) {
        qCWarning(scriptengine) << "Failed to decrypt message: " << ERR_error_string(ERR_get_error(), nullptr);
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(privateKey);
        return NULL;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(privateKey);

    // Convert decrypted data back to QString
    return QString::fromUtf8(reinterpret_cast<const char*>(decrypted.data()), decryptedLength);
}