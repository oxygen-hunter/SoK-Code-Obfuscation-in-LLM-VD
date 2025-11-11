#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>

#define BUFSIZE 4096

extern "C" {
    #include <dlfcn.h>
}

class AnsibleError : public std::runtime_error {
public:
    explicit AnsibleError(const std::string &msg) : std::runtime_error(msg) {}
};

class Connection {
public:
    Connection(std::string runner, std::string host, int port);
    std::vector<std::string> list_jails();
    std::string get_jail_path();
    Connection *connect(int port = 0);
    void put_file(const std::string &in_path, const std::string &out_path);
    void fetch_file(const std::string &in_path, const std::string &out_path);
    void close();
    int exec_command(const std::string &cmd, const std::string &tmp_path, const std::string &become_user = "", bool sudoable = false, const std::string &executable = "/bin/sh", const void* in_data = nullptr);

private:
    std::string _search_executable(const std::string &executable);
    std::vector<std::string> _generate_cmd(const std::string &executable, const std::string &cmd);
    FILE* _buffered_exec_command(const std::string &cmd, const std::string &tmp_path, const std::string &become_user = "", bool sudoable = false, const std::string &executable = "/bin/sh", const void* in_data = nullptr);

    std::string jail;
    std::string runner;
    std::string host;
    bool has_pipelining;
    // Assuming BECOME_METHODS is a vector of strings
    std::vector<std::string> become_methods_supported;
    std::string jls_cmd;
    std::string jexec_cmd;
    int port;
};

Connection::Connection(std::string runner, std::string host, int port) : runner(std::move(runner)), host(std::move(host)), port(port) {
    this->jail = this->host;
    this->has_pipelining = false;
    // Initialize become_methods_supported as needed

    if (geteuid() != 0) {
        throw AnsibleError("jail connection requires running as root");
    }

    this->jls_cmd = this->_search_executable("jls");
    this->jexec_cmd = this->_search_executable("jexec");

    if (std::find(this->list_jails().begin(), this->list_jails().end(), this->jail) == this->list_jails().end()) {
        throw AnsibleError("incorrect jail name " + this->jail);
    }
}

std::string Connection::_search_executable(const std::string &executable) {
    char* path = getenv("PATH");
    if (!path) {
        throw AnsibleError(executable + " command not found in PATH");
    }
    char* result = (char*)dlsym(RTLD_DEFAULT, executable.c_str());
    if (!result) {
        throw AnsibleError(executable + " command not found in PATH");
    }
    return std::string(result);
}

std::vector<std::string> Connection::list_jails() {
    std::vector<std::string> result;
    FILE *pipe = popen((this->jls_cmd + " -q name").c_str(), "r");
    if (!pipe) {
        throw AnsibleError("popen() failed!");
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.emplace_back(buffer);
    }
    pclose(pipe);
    return result;
}

std::string Connection::get_jail_path() {
    FILE *pipe = popen((this->jls_cmd + " -j " + this->jail + " -q path").c_str(), "r");
    if (!pipe) {
        throw AnsibleError("popen() failed!");
    }
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

Connection* Connection::connect(int port) {
    std::cout << "THIS IS A LOCAL JAIL DIR " << this->jail << std::endl;
    return this;
}

std::vector<std::string> Connection::_generate_cmd(const std::string &executable, const std::string &cmd) {
    std::vector<std::string> local_cmd;
    if (!executable.empty()) {
        local_cmd = {this->jexec_cmd, this->jail, executable, "-c", cmd};
    } else {
        local_cmd = {this->jexec_cmd, this->jail, cmd};
    }
    return local_cmd;
}

FILE* Connection::_buffered_exec_command(const std::string &cmd, const std::string &tmp_path, const std::string &become_user, bool sudoable, const std::string &executable, const void* in_data) {
    if (sudoable && this->runner == "become" && std::find(this->become_methods_supported.begin(), this->become_methods_supported.end(), "become_method") == this->become_methods_supported.end()) {
        throw AnsibleError("Internal Error: this module does not support running commands via become_method");
    }

    if (in_data) {
        throw AnsibleError("Internal Error: this module does not support optimized module pipelining");
    }

    std::vector<std::string> local_cmd = this->_generate_cmd(executable, cmd);
    std::cout << "EXEC " << cmd << " on host: " << this->jail << std::endl;
    FILE* pipe = popen((this->jexec_cmd + " " + cmd).c_str(), "r");
    if (!pipe) {
        throw AnsibleError("popen() failed!");
    }
    return pipe;
}

int Connection::exec_command(const std::string &cmd, const std::string &tmp_path, const std::string &become_user, bool sudoable, const std::string &executable, const void* in_data) {
    FILE* pipe = this->_buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data);
    char buffer[128];

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::cout << buffer;
    }

    int status = pclose(pipe);
    return WEXITSTATUS(status);
}

void Connection::put_file(const std::string &in_path, const std::string &out_path) {
    std::cout << "PUT " << in_path << " TO " << out_path << " on host: " << this->jail << std::endl;

    std::ifstream in_file(in_path, std::ios::binary);
    if (!in_file) {
        throw AnsibleError("failed to open input file " + in_path);
    }

    std::string cmd = "dd of=" + out_path;
    FILE* pipe = this->_buffered_exec_command(cmd, "");

    char buffer[BUFSIZE];
    while (in_file.read(buffer, sizeof(buffer))) {
        fwrite(buffer, sizeof(char), in_file.gcount(), pipe);
    }

    int status = pclose(pipe);
    if (status != 0) {
        throw AnsibleError("failed to transfer file to " + out_path);
    }
}

void Connection::fetch_file(const std::string &in_path, const std::string &out_path) {
    std::cout << "FETCH " << in_path << " TO " << out_path << " on host: " << this->jail << std::endl;

    std::string cmd = "dd if=" + in_path + " bs=" + std::to_string(BUFSIZE);
    FILE* pipe = this->_buffered_exec_command(cmd, "");

    std::ofstream out_file(out_path, std::ios::binary);
    if (!out_file) {
        throw AnsibleError("failed to open output file " + out_path);
    }

    char buffer[BUFSIZE];
    while (fread(buffer, sizeof(char), BUFSIZE, pipe) > 0) {
        out_file.write(buffer, BUFSIZE);
    }

    int status = pclose(pipe);
    if (status != 0) {
        throw AnsibleError("failed to fetch file to " + out_path);
    }
}

void Connection::close() {
    // No operation
}