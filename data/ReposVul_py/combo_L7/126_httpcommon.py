#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace {
    void log_warn(const std::string& message) {
        std::cerr << "WARN: " << message << std::endl;
    }
}

class SslComponents {
public:
    SslComponents(const std::string& certfile, const std::string& cacert)
        : privKeyAndCertFilename(certfile), caCertFilename(cacert) {
        myPrivateCertificate = makeMyCertificate();
        trustRoot = makeTrustRoot();
    }

private:
    std::string privKeyAndCertFilename;
    std::string caCertFilename;
    SSL_CTX* myPrivateCertificate;
    std::vector<X509*> trustRoot;

    SSL_CTX* makeMyCertificate() {
        if (privKeyAndCertFilename.empty()) {
            log_warn("No HTTPS private key / cert found: not starting replication server or doing replication pushes");
            return nullptr;
        }

        try {
            std::ifstream fp(privKeyAndCertFilename);
            if (!fp.is_open()) throw std::ios_base::failure("File not open");
            std::string authData((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
            fp.close();

            SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());
            SSL_CTX_use_certificate_chain_file(ctx, privKeyAndCertFilename.c_str());
            SSL_CTX_use_PrivateKey_file(ctx, privKeyAndCertFilename.c_str(), SSL_FILETYPE_PEM);
            return ctx;
        } catch (...) {
            log_warn("Unable to read private key / cert file from " + privKeyAndCertFilename + ": not starting the replication HTTPS server or doing replication pushes.");
            return nullptr;
        }
    }

    std::vector<X509*> makeTrustRoot() {
        std::vector<X509*> caCerts;
        if (!caCertFilename.empty()) {
            try {
                std::ifstream fp(caCertFilename);
                if (!fp.is_open()) throw std::ios_base::failure("File not open");
                std::string caCertData((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
                fp.close();

                BIO* bio = BIO_new_mem_buf(caCertData.data(), caCertData.size());
                X509* caCert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
                if (caCert) {
                    caCerts.push_back(caCert);
                }
                BIO_free(bio);

            } catch (...) {
                log_warn("Failed to open CA cert file " + caCertFilename);
            }
            log_warn("Using custom CA cert file: " + caCertFilename);
        }
        return caCerts;
    }
};

extern "C" {
    int read_body_with_max_size(void* response, size_t max_size) {
        // Placeholder for mixed C/C++ and inline-assembly
        // This function simulates reading a body with a maximum size
        __asm__(
            "mov $0, %eax\n\t"
            "cmp %rcx, %rdx\n\t"
            "ja discard_body\n\t"
            "read_body:\n\t"
            "ret\n\t"
            "discard_body:\n\t"
            "mov $1, %eax\n\t"
            "ret\n\t"
        );
    }
}