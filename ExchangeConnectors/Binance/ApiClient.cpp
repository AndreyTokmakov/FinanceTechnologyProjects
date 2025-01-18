/**============================================================================
Name        : ApiClient.cpp
Created on  : 18.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : ApiClient.cpp
============================================================================**/

#include "ApiClient.h"

#include <iostream>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <vector>

#include "FileUtilities.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>

#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>

namespace
{
    constexpr std::string_view apiKey { "9FmOZl0CCPVkzipOv0kXMx0gaL1BSeCuUhzG0CKilr0yjS6mxf037UvqM2nhAuXf" };
    constexpr std::string_view privateKeyPath { R"(/home/andtokm/Documents/Binance/ssh_Key/ed25519.pem)" };
}

namespace
{
    using ptrBigNumber = std::unique_ptr<BIGNUM, decltype(&::BN_free)>;
    using ptrRSA = std::unique_ptr<RSA, decltype(&::RSA_free)>;
    using ptrBIO = std::unique_ptr<BIO, decltype(&::BIO_free)>;
    using ptrPKEY = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
    using ptrAsnInteger = std::unique_ptr<ASN1_INTEGER, decltype(&::ASN1_INTEGER_free)>;

    struct CertificateDeleter
    {
        void operator()(BIO* bio) const {
            if (bio)
                ::BIO_free(bio);
        }

        void operator()(X509* x509) const {
            if (x509)
                ::X509_free(x509);
        }
    };

    using ptrCert509 = std::unique_ptr<X509, CertificateDeleter>;
    using ptrCert509Ex = std::unique_ptr<X509, decltype(&::X509_free)>;
    auto x509Deleter = [] (X509* ptr) {
        if (ptr)
            X509_free(ptr);
    };
}

namespace
{

    [[nodiscard]]
    std::vector<char8_t> readCertificate(std::string_view path) noexcept
    {
        std::vector<char8_t> data {};
        if (std::fstream file(path.data(), std::ios::in | std::ios::binary); file.is_open() && file.good())
        {
            file.seekg(0, std::ios_base::end);
            const auto bytesLength{file.tellg()};
            file.seekg(0, std::ios_base::beg);

            data.resize(bytesLength);
            file.read(reinterpret_cast<char *>(data.data()), bytesLength);
        }
        return data;
    }

    [[nodiscard]]
    ptrCert509 loadCertificate(std::string_view path) noexcept
    {
        const std::vector<char8_t> content = readCertificate(path);
        const unsigned char* data = reinterpret_cast<const unsigned char*>(content.data());
        std::cout << '[' << data << ']' << std::endl;
        return {d2i_X509(nullptr, &data, std::ssize(content)), CertificateDeleter{}};
    }
}

void ApiClient::TestAll()
{

    /*
    std::string privateKey = FileUtilities::ReadFile(privateKeyPath);
    const unsigned char* data = reinterpret_cast<const unsigned char*>(privateKey.data());
    std::cout << privateKey << std::endl;

    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    X509 *cert = d2i_X509(nullptr, &(data), privateKey.length());
    if (!cert) {
        fprintf(stderr, "unable to parse certificate in memory\n");
    }

    X509_free(cert);
    */


    FILE *fp = fopen(privateKeyPath.data(), "r");
    if (!fp) {
        fprintf(stderr, "unable to open: %s\n", privateKeyPath);
    }

    std::cout << 1 << std::endl;

    X509 *cert = PEM_read_X509(fp, nullptr, nullptr, nullptr);
    if (!cert) {
        fprintf(stderr, "unable to parse certificate in: %s\n", privateKeyPath);
        fclose(fp);
    }


    std::cout << 1 << std::endl;

    X509_free(cert);
    fclose(fp);
}