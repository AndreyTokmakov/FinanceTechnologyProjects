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
    constexpr std::string_view privateKeyPath { R"(/home/andtokm/Temp/Certs/cacert.pem)" };
}

namespace
{
    using ptrBigNumber = std::unique_ptr<BIGNUM, decltype(&::BN_free)>;
    using ptrRSA = std::unique_ptr<RSA, decltype(&::RSA_free)>;
    using ptrBIO = std::unique_ptr<BIO, decltype(&::BIO_free)>;
    using ptrPKEY = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
    using ptrAsnInteger = std::unique_ptr<ASN1_INTEGER, decltype(&::ASN1_INTEGER_free)>;
    using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
    using ASN1_TIME_ptr = std::unique_ptr<ASN1_TIME, decltype(&ASN1_STRING_free)>;

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

    using BIO_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;
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
    std::string bio_to_string(const BIO_ptr& bio,
                              const int max_len)
    {
        std::string buffer(max_len, '\0');
        BIO_read(bio.get(), buffer.data(), max_len);
        return buffer;
    }


    [[nodiscard]]
    std::vector<char8_t> readCertificate(const std::string_view path) noexcept
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
    ptrCert509 loadCertificate(const std::string_view path) noexcept
    {
        const std::vector<char8_t> content = readCertificate(path);
        const unsigned char* data = reinterpret_cast<const unsigned char*>(content.data());
        std::cout << '[' << data << ']' << std::endl;
        return {d2i_X509(nullptr, &data, std::ssize(content)), CertificateDeleter{}};
    }
}

// https://gist.github.com/cseelye/adcd900768ff61f697e603fd41c67625
void ApiClient::TestAll()
{
    BIO_ptr input(BIO_new(BIO_s_file()), BIO_free);
    if (BIO_read_filename(input.get(), privateKeyPath.data()) <= 0) {
        std::cout << "Error reading file" << std::endl;
    }

    X509_ptr cert(PEM_read_bio_X509_AUX(input.get(), nullptr, nullptr, nullptr), X509_free);

    // Create a BIO to hold info from the cert
    BIO_ptr output_bio(BIO_new(BIO_s_mem()), BIO_free);

    X509_NAME *subject = X509_get_subject_name(cert.get());

    X509_NAME_print_ex(output_bio.get(), subject, 0, 0);
    std::string cert_subject = bio_to_string(output_bio, 4096);

    std::cout << "Subject:" << std::endl;
    std::cout << cert_subject << std::endl;
    std::cout << std::endl;

    ASN1_TIME *expires = X509_get_notAfter(cert.get());

    // Construct another ASN1_TIME for the unix epoch, get the difference
    // between them and use that to calculate a unix timestamp representing
    // when the cert expires
    ASN1_TIME_ptr epoch(ASN1_TIME_new(), ASN1_STRING_free);
    ASN1_TIME_set_string(epoch.get(), "700101000000");
    int days, seconds;
    ASN1_TIME_diff(&days, &seconds, epoch.get(), expires);
    time_t expire_timestamp = (days * 24 * 60 * 60) + seconds;

    std::cout << "Expiration timestamp:" << std::endl;
    std::cout << expire_timestamp << std::endl;
    std::cout << std::endl;

}