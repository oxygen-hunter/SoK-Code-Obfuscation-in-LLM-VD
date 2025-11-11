#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <algorithm>

extern "C" {
    #include <Python.h>
}

#define PY_SSIZE_T_CLEAN
#include <twisted_defer.h>
#include <signedjson_key.h>
#include <signedjson_sign.h>
#include <federation_httpclient.h>

using namespace std;

class NoAuthenticationError : public std::exception {
    const char* what() const throw() { return "No signature could be authenticated"; }
};

class Verifier {
    void* sydent;
    map<string, map<string, string>> cache;

public:
    Verifier(void* sydent) : sydent(sydent) {}

    twisted::Deferred<map<string, map<string, string>>> _getKeysForServer(string server_name) {
        return twisted::inlineCallbacks([this, server_name](){
            if (cache.find(server_name) != cache.end()) {
                auto cached = cache[server_name];
                auto now = static_cast<long>(time(nullptr) * 1000);
                if (std::stol(cached["valid_until_ts"]) > now) {
                    return twisted::defer_returnValue(cache[server_name]["verify_keys"]);
                }
            }

            FederationHttpClient client(sydent);
            auto result = client.get_json("matrix://" + server_name + "/_matrix/key/v2/server/", 1024 * 50).await();
            if (result.find("verify_keys") == result.end()) {
                throw SignatureVerifyException("No key found in response");
            }

            if (result.find("valid_until_ts") != result.end()) {
                cout << "Got keys for " << server_name << ": caching until " << result["valid_until_ts"] << endl;
                cache[server_name] = result;
            }

            return twisted::defer_returnValue(result["verify_keys"]);
        });
    }

    twisted::Deferred<pair<string, string>> verifyServerSignedJson(map<string, any> signed_json, vector<string> acceptable_server_names = {}) {
        return twisted::inlineCallbacks([this, signed_json, acceptable_server_names](){
            if (signed_json.find("signatures") == signed_json.end()) {
                throw SignatureVerifyException("Signature missing");
            }
            for (auto& [server_name, sigs] : any_cast<map<string, map<string, string>>>(signed_json["signatures"])) {
                if (!acceptable_server_names.empty() && find(acceptable_server_names.begin(), acceptable_server_names.end(), server_name) == acceptable_server_names.end()) {
                    continue;
                }

                auto server_keys = _getKeysForServer(server_name).await();
                for (auto& [key_name, sig] : sigs) {
                    if (server_keys.find(key_name) != server_keys.end() && server_keys[key_name].find("key") != server_keys[key_name].end()) {
                        auto key_bytes = decode_base64(server_keys[key_name]["key"]);
                        auto verify_key = signedjson_key::decode_verify_key_bytes(key_name, key_bytes);
                        cout << "verifying sig from key " << key_name << endl;
                        signedjson_sign::verify_signed_json(signed_json, server_name, verify_key);
                        cout << "Verified signature with key " << key_name << " from " << server_name << endl;
                        return twisted::defer_returnValue(make_pair(server_name, key_name));
                    }
                }
                cerr << "No matching key found for signature block in server keys" << endl;
            }
            cerr << "Unable to verify any signatures. Acceptable server names: " << endl;
            throw SignatureVerifyException("No matching signature found");
        });
    }

    twisted::Deferred<string> authenticate_request(void* request, string content) {
        return twisted::inlineCallbacks([this, request, content](){
            map<string, any> json_request = {
                {"method", "GET"},
                {"uri", "/_matrix/key/v2/server/"},
                {"destination_is", sydent},
                {"signatures", map<string, map<string, string>>{}},
            };

            if (!content.empty()) {
                json_request["content"] = content;
            }

            string origin;
            vector<string> auth_headers = request->getRawHeaders("Authorization");

            if (auth_headers.empty()) {
                throw NoAuthenticationError();
            }

            for (auto& auth : auth_headers) {
                if (auth.find("X-Matrix") == 0) {
                    auto [origin, key, sig] = parse_auth_header(auth);
                    json_request["origin"] = origin;
                    json_request["signatures"].emplace(origin, map<string, string>{{key, sig}});
                }
            }

            if (json_request["signatures"].empty()) {
                throw NoAuthenticationError();
            }

            verifyServerSignedJson(json_request, {origin}).await();

            cout << "Verified request from HS " << origin << endl;

            return twisted::defer_returnValue(origin);
        });
    }

private:
    tuple<string, string, string> parse_auth_header(string header_str) {
        vector<string> params;
        stringstream ss(header_str.substr(header_str.find(" ") + 1));
        string item;
        while (getline(ss, item, ',')) {
            params.push_back(item);
        }
        map<string, string> param_dict;
        for (auto& kv : params) {
            auto pos = kv.find("=");
            param_dict[kv.substr(0, pos)] = kv.substr(pos + 1);
        }

        auto strip_quotes = [](string value) {
            if (value[0] == '"') {
                return value.substr(1, value.length() - 2);
            }
            return value;
        };

        return make_tuple(strip_quotes(param_dict["origin"]), strip_quotes(param_dict["key"]), strip_quotes(param_dict["sig"]));
    }
};