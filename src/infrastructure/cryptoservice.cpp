#include "cryptoservice.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <argon2.h>


static constexpr int IV_SIZE = 12;
static constexpr int TAG_SIZE = 16;
static constexpr int KEY_SIZE = 32;


QByteArray CryptoService::encrypt(const QByteArray& plaintext, const QByteArray& key)
{
    QByteArray iv(IV_SIZE, Qt::Uninitialized);

    if (RAND_bytes(
            reinterpret_cast<unsigned char*>(iv.data()),
            iv.size()) != 1)
    {
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        return {};
    }

    // init
    if (EVP_EncryptInit_ex(
            ctx,
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);

        return {};
    }

    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_SET_IVLEN,
        IV_SIZE,
        nullptr);

    // set key + iv
    EVP_EncryptInit_ex(
        ctx,
        nullptr,
        nullptr,
        reinterpret_cast<const unsigned char*>(key.data()),
        reinterpret_cast<const unsigned char*>(iv.data()));

    // encrypt data
    QByteArray ciphertext;
    ciphertext.resize(plaintext.size());

    int len = 0;
    int outLen = 0;

    EVP_EncryptUpdate(
        ctx,
        reinterpret_cast<unsigned char*>(ciphertext.data()),
        &len,
        reinterpret_cast<const unsigned char*>(plaintext.data()),
        plaintext.size());

    outLen = len;

    // finalize
    EVP_EncryptFinal_ex(
        ctx,
        reinterpret_cast<unsigned char*>(ciphertext.data()) + len,
        &len);

    outLen += len;

    // tag
    QByteArray tag(TAG_SIZE, Qt::Uninitialized);

    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_GET_TAG,
        TAG_SIZE,
        tag.data());

    // cleanup
    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(outLen);

    return iv + tag + ciphertext;
}

QByteArray CryptoService::decrypt(const QByteArray& data, const QByteArray& key)
{
    if (data.size() < IV_SIZE + TAG_SIZE)
    {
        return {};
    }

    QByteArray iv = data.left(IV_SIZE);
    QByteArray tag = data.mid(IV_SIZE, TAG_SIZE);
    QByteArray ciphertext = data.mid(IV_SIZE + TAG_SIZE);

    // ctx
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
    {
        return {};
    }

    // init
    if (!EVP_DecryptInit_ex(
            ctx,
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr))
    {
        EVP_CIPHER_CTX_free(ctx);

        return {};
    }

    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_SET_IVLEN,
        IV_SIZE,
        nullptr);

    // key + iv
    EVP_DecryptInit_ex(
        ctx,
        nullptr,
        nullptr,
        reinterpret_cast<const unsigned char*>(key.data()),
        reinterpret_cast<const unsigned char*>(iv.data()));

    // decrypt
    QByteArray plaintext;
    plaintext.resize(ciphertext.size());

    int len = 0;
    int outLen = 0;

    EVP_DecryptUpdate(
        ctx,
        reinterpret_cast<unsigned char*>(plaintext.data()),
        &len,
        reinterpret_cast<const unsigned char*>(ciphertext.data()),
        ciphertext.size());

    outLen = len;

    // tag
    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_SET_TAG,
        TAG_SIZE,
        const_cast<char*>(tag.data()));

    // final verify
    int ret = EVP_DecryptFinal_ex(
        ctx,
        reinterpret_cast<unsigned char*>(plaintext.data()) + len,
        &len);

    EVP_CIPHER_CTX_free(ctx);

    if (ret <= 0)
    {
        return {};
    }

    outLen += len;
    plaintext.resize(outLen);

    return plaintext;
}

QByteArray CryptoService::deriveKey(const QString& password, const VaultHeader& vaultHeader)
{
    QByteArray key(KEY_SIZE, Qt::Uninitialized);

    QByteArray passUtf8 = password.toUtf8();

    int result = argon2id_hash_raw(
        vaultHeader.iterations,
        vaultHeader.memory,
        vaultHeader.parallelism,

        passUtf8.constData(),
        passUtf8.size(),

        vaultHeader.salt.constData(),
        vaultHeader.salt.size(),

        key.data(),
        key.size()
    );

    if (result != ARGON2_OK)
    {
        return {};
    }

    return key;
}

QByteArray CryptoService::generateSalt()
{
    QByteArray salt(16, Qt::Uninitialized);

    if (RAND_bytes(
            reinterpret_cast<unsigned char*>(salt.data()),
            salt.size()) != 1)
    {
        return {};
    }

    return salt;
}