#include <distutils/spawn.h>
#include <traceback.h>
#include <os.h>
#include <subprocess.h>
#include <ansible/errors.h>
#include <ansible/utils.h>
#include <ansible/callbacks.h>
#include <ansible/constants.h>

#define BUFSIZE 65536

class Connection {
public:
    Connection(void* runner, const char* host, int port, ...) {
        this->chroot = host;
        this->has_pipelining = false;
        this->become_methods_supported = C::BECOME_METHODS;

        if (os::geteuid() != 0) {
            throw errors::AnsibleError("chroot connection requires running as root");
        }

        if (!os::isdir(this->chroot)) {
            throw errors::AnsibleError("%s is not a directory", this->chroot);
        }

        std::string chrootsh = os::path::join(this->chroot, "/bin/sh");
        if (!utils::is_executable(chrootsh.c_str())) {
            throw errors::AnsibleError("%s does not look like a chrootable dir (/bin/sh missing)", this->chroot);
        }

        this->chroot_cmd = distutils::spawn::find_executable("chroot");
        if (!this->chroot_cmd) {
            throw errors::AnsibleError("chroot command not found in PATH");
        }

        this->runner = runner;
        this->host = host;
        this->port = port;
    }

    Connection* connect(int port = 0) {
        vvv("THIS IS A LOCAL CHROOT DIR", this->chroot);
        return this;
    }

    std::string _generate_cmd(const char* executable, const char* cmd) {
        if (executable) {
            std::string local_cmd[] = { this->chroot_cmd, this->chroot, executable, "-c", cmd };
            return local_cmd;
        } else {
            std::string local_cmd = this->chroot_cmd + " \"" + this->chroot + "\" " + cmd;
            return local_cmd;
        }
    }

    subprocess::Popen _buffered_exec_command(const char* cmd, const char* tmp_path, const char* become_user = nullptr, bool sudoable = false, const char* executable = "/bin/sh", const char* in_data = nullptr, subprocess::PIPE* stdin = subprocess::PIPE) {
        if (sudoable && this->runner->become && this->runner->become_method != this->become_methods_supported) {
            throw errors::AnsibleError("Internal Error: this module does not support running commands via %s", this->runner->become_method);
        }

        if (in_data) {
            throw errors::AnsibleError("Internal Error: this module does not support optimized module pipelining");
        }

        std::string local_cmd = this->_generate_cmd(executable, cmd);

        vvv("EXEC %s", local_cmd, this->chroot);
        subprocess::Popen p = subprocess::Popen(local_cmd.c_str(), subprocess::shell(isinstance(local_cmd, std::string)), this->runner->basedir, stdin, subprocess::PIPE, subprocess::PIPE);

        return p;
    }

    std::tuple<int, std::string, std::string, std::string> exec_command(const char* cmd, const char* tmp_path, const char* become_user = nullptr, bool sudoable = false, const char* executable = "/bin/sh", const char* in_data = nullptr) {
        subprocess::Popen p = this->_buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data);

        std::string stdout, stderr;
        p.communicate(stdout, stderr);
        return std::make_tuple(p.returncode, "", stdout, stderr);
    }

    void put_file(const char* in_path, const char* out_path) {
        vvv("PUT %s TO %s", in_path, out_path, this->chroot);

        try {
            std::ifstream in_file(in_path, std::ios::binary);
            if (!in_file.is_open()) {
                throw std::ios::failure("file or module does not exist at: " + std::string(in_path));
            }

            try {
                subprocess::Popen p = this->_buffered_exec_command(("dd of=" + std::string(out_path)).c_str(), nullptr, &in_file);
                std::string stdout, stderr;
                p.communicate(stdout, stderr);
                if (p.returncode != 0) {
                    throw errors::AnsibleError("failed to transfer file %s to %s:\n%s\n%s", in_path, out_path, stdout, stderr);
                }
            } catch (const OSError&) {
                throw errors::AnsibleError("chroot connection requires dd command in the chroot");
            }
        } catch (const std::ios::failure& e) {
            throw errors::AnsibleError(e.what());
        }
    }

    void fetch_file(const char* in_path, const char* out_path) {
        vvv("FETCH %s TO %s", in_path, out_path, this->chroot);

        try {
            subprocess::Popen p = this->_buffered_exec_command(("dd if=" + std::string(in_path) + " bs=" + std::to_string(BUFSIZE)).c_str(), nullptr);
            std::ofstream out_file(out_path, std::ios::binary | std::ios::trunc);
            if (!out_file.is_open()) {
                throw std::ios::failure("failed to open output file: " + std::string(out_path));
            }

            try {
                char buffer[BUFSIZE];
                while (p.stdout.read(buffer, sizeof(buffer))) {
                    out_file.write(buffer, p.stdout.gcount());
                }
            } catch (...) {
                traceback::print_exc();
                throw errors::AnsibleError("failed to transfer file %s to %s", in_path, out_path);
            }
            std::string stdout, stderr;
            p.communicate(stdout, stderr);
            if (p.returncode != 0) {
                throw errors::AnsibleError("failed to transfer file %s to %s:\n%s\n%s", in_path, out_path, stdout, stderr);
            }
        } catch (const OSError&) {
            throw errors::AnsibleError("chroot connection requires dd command in the jail");
        }
    }

    void close() {}
};