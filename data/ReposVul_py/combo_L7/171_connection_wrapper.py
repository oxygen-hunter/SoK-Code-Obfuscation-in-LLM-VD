#include <iostream>
#include <json/json.h>
#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include <sstream>

using namespace std;

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

class AirflowException : public runtime_error {
public:
    explicit AirflowException(const string& message) : runtime_error(message) {}
};

void mask_secret(const string& secret) {
    cout << "Secret masked: " << secret << endl;
}

map<string, string> parse_s3_config(const string& config_file_name, const string& config_format = "boto", const string& profile = "") {
    cout << "Warning: Use local credentials file is never documented and well tested.\n"
         << "Obtain credentials by this way deprecated and will be removed in a future releases." << endl;

    FILE* file = fopen(config_file_name.c_str(), "r");
    if (!file) {
        throw AirflowException("Couldn't read " + config_file_name);
    }

    char line[256];
    map<string, string> config;
    string section;
    while (fgets(line, sizeof(line), file)) {
        string str_line(line);
        str_line.erase(str_line.find_last_not_of(" \n\r\t") + 1);
        if (str_line.empty()) continue;

        if (str_line.front() == '[' && str_line.back() == ']') {
            section = str_line.substr(1, str_line.size() - 2);
        } else {
            size_t pos = str_line.find('=');
            if (pos == string::npos) continue;
            string key = str_line.substr(0, pos);
            string value = str_line.substr(pos + 1);
            config[section + "." + key] = value;
        }
    }
    fclose(file);

    string cred_section;
    if (config_format == "boto") {
        cred_section = profile.empty() ? "Credentials" : "profile " + profile;
    } else if (config_format == "aws" && !profile.empty()) {
        cred_section = profile;
    } else {
        cred_section = "default";
    }

    string key_id_option = config_format == "boto" || config_format == "aws" ? "aws_access_key_id" : "access_key";
    string secret_key_option = config_format == "boto" || config_format == "aws" ? "aws_secret_access_key" : "secret_key";

    string access_key = config[cred_section + "." + key_id_option];
    string secret_key = config[cred_section + "." + secret_key_option];
    mask_secret(secret_key);

    return {{"access_key", access_key}, {"secret_key", secret_key}};
}

extern "C" void c_parse_s3_config(const char* config_file_name, const char* config_format, const char* profile) {
    try {
        map<string, string> creds = parse_s3_config(config_file_name, config_format ? string(config_format) : "boto", profile ? string(profile) : "");
        printf("Access Key: %s\n", creds["access_key"].c_str());
        printf("Secret Key: %s\n", creds["secret_key"].c_str());
    } catch (const exception& e) {
        fprintf(stderr, "%s\n", e.what());
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <config_file_name> <config_format> <profile>\n", argv[0]);
        return 1;
    }
    c_parse_s3_config(argv[1], argv[2], argv[3]);
    return 0;
}