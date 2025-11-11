#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
#include <sys/stat.h>

extern "C" {
    #include <unistd.h>
    #include <stdio.h>
    #include <string.h>
}

using namespace std;

bool chef_installed();
string default_logfile(const string &exe_name);
map<string, string> exec_cmd(const vector<string> &args, const map<string, string> &kwargs);

bool chef_installed() {
    return system("which chef-client > /dev/null 2>&1") == 0;
}

string default_logfile(const string &exe_name) {
    string logfile;
    if (system("uname | grep -i 'mingw' > /dev/null 2>&1") == 0) {
        string tmp_dir = "/tmp";
        struct stat info;
        if (stat(tmp_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            mkdir(tmp_dir.c_str(), 0777);
        }
        char tmp_name[L_tmpnam];
        tmpnam(tmp_name);
        logfile = string(tmp_name);
    } else {
        logfile = "/var/log/" + exe_name + ".log";
    }
    return logfile;
}

map<string, string> exec_cmd(const vector<string> &args, const map<string, string> &kwargs) {
    string cmd_args;
    for (const auto &arg : args) {
        cmd_args += arg + " ";
    }
    string cmd_kwargs;
    for (const auto &pair : kwargs) {
        if (pair.first.substr(0, 2) != "__") {
            cmd_kwargs += "--" + pair.first + " " + pair.second + " ";
        }
    }
    string cmd_exec = cmd_args + cmd_kwargs;
    cout << "Chef command: " << cmd_exec << endl;

    FILE *pipe = popen(cmd_exec.c_str(), "r");
    if (!pipe) {
        return {{"stderr", "popen() failed!"}};
    }
    char buffer[128];
    string result;
    while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
        result += buffer;
    }
    int rc = pclose(pipe);
    return {{"stdout", result}, {"returncode", to_string(rc)}};
}

int main() {
    if (!chef_installed()) {
        cout << "Chef not installed." << endl;
        return 1;
    }
    vector<string> args = {"chef-client", "--no-color", "--once", "--logfile", "\"" + default_logfile("chef-client") + "\"", "--format", "doc"};
    map<string, string> kwargs;
    map<string, string> result = exec_cmd(args, kwargs);
    cout << "Command output: " << result["stdout"] << endl;
    return 0;
}