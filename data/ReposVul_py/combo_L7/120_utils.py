#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <Python.h>
#include <django/core/exceptions.h>

extern "C" {
#include <curl/curl.h>
}

static std::map<std::string, X509*> _CERT_CACHE;

#define SES_REGEX_CERT_URL "(?i)^https://sns\\.[a-z0-9\\-]+\\.amazonaws\\.com(\\.cn)?/SimpleNotificationService\\-[a-z0-9]+\\.pem$"

void clear_cert_cache() {
    _CERT_CACHE.clear();
}

class EventMessageVerifier {
public:
    EventMessageVerifier(PyObject* notification) : _data(notification), _verified(false) {}

    bool is_verified() {
        if (_verified) {
            return _verified;
        }

        PyObject* py_signature = PyDict_GetItemString(_data, "Signature");
        if (!py_signature) {
            _verified = false;
            return _verified;
        }

        const char* signature = PyBytes_AsString(PyBytes_FromObject(py_signature));
        unsigned char* decoded_signature;
        size_t decoded_length;
        EVP_DecodeBlock(decoded_signature, (const unsigned char*)signature, strlen(signature));
        decoded_length = strlen((char*)decoded_signature);

        unsigned char* sign_bytes = _get_bytes_to_sign();
        if (!sign_bytes) {
            _verified = false;
            return _verified;
        }

        X509* cert = certificate();
        if (!cert) {
            _verified = false;
            return _verified;
        }

        EVP_PKEY* pkey = X509_get_pubkey(cert);
        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        EVP_PKEY_CTX* pctx;
        if (EVP_DigestVerifyInit(mdctx, &pctx, EVP_sha1(), NULL, pkey) <= 0) {
            _verified = false;
        }

        if (EVP_DigestVerify(mdctx, decoded_signature, decoded_length, sign_bytes, strlen((char*)sign_bytes)) <= 0) {
            _verified = false;
        } else {
            _verified = true;
        }

        EVP_MD_CTX_free(mdctx);
        return _verified;
    }

private:
    PyObject* _data;
    bool _verified;

    X509* certificate() {
        PyObject* py_cert_url = PyDict_GetItemString(_data, "SigningCertURL");
        if (!py_cert_url) {
            return nullptr;
        }

        const char* cert_url = PyUnicode_AsUTF8(py_cert_url);
        if (_CERT_CACHE.find(cert_url) != _CERT_CACHE.end()) {
            return _CERT_CACHE[cert_url];
        }

        CURL* curl;
        CURLcode res;
        std::string response_string;

        curl = curl_easy_init();
        if (!curl) {
            return nullptr;
        }

        curl_easy_setopt(curl, CURLOPT_URL, cert_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            _CERT_CACHE[cert_url] = nullptr;
            curl_easy_cleanup(curl);
            return nullptr;
        }

        const char* resp_data = response_string.c_str();
        BIO* bio = BIO_new_mem_buf(resp_data, -1);
        X509* x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        if (!x509) {
            _CERT_CACHE[cert_url] = nullptr;
        } else {
            _CERT_CACHE[cert_url] = x509;
        }

        BIO_free(bio);
        curl_easy_cleanup(curl);
        return x509;
    }

    unsigned char* _get_bytes_to_sign() {
        PyObject* py_type = PyDict_GetItemString(_data, "Type");
        const char* msg_type = PyUnicode_AsUTF8(py_type);

        std::string fields_to_sign;
        if (strcmp(msg_type, "Notification") == 0) {
            fields_to_sign = "Message\nMessageId\nSubject\nTimestamp\nTopicArn\nType\n";
        } else if (strcmp(msg_type, "SubscriptionConfirmation") == 0 ||
                   strcmp(msg_type, "UnsubscribeConfirmation") == 0) {
            fields_to_sign = "Message\nMessageId\nSubscribeURL\nTimestamp\nToken\nTopicArn\nType\n";
        } else {
            return nullptr;
        }

        std::string bytes_to_sign;
        for (const auto& field : fields_to_sign) {
            PyObject* field_value = PyDict_GetItemString(_data, field.c_str());
            if (field_value) {
                const char* value = PyUnicode_AsUTF8(field_value);
                bytes_to_sign += field + std::string("\n") + value + std::string("\n");
            }
        }

        return (unsigned char*)bytes_to_sign.c_str();
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append((char*)contents, totalSize);
        return totalSize;
    }
};

extern "C" {
    PyObject* verify_event_message(PyObject* self, PyObject* args) {
        PyObject* notification;
        if (!PyArg_ParseTuple(args, "O", &notification)) {
            return NULL;
        }

        EventMessageVerifier verifier(notification);
        bool verified = verifier.is_verified();
        return PyBool_FromLong(verified);
    }
}

static PyMethodDef Methods[] = {
    {"verify_event_message", verify_event_message, METH_VARARGS, "Verify an SES/SNS event notification message."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "event_verifier",
    NULL,
    -1,
    Methods
};

PyMODINIT_FUNC PyInit_event_verifier(void) {
    return PyModule_Create(&module);
}