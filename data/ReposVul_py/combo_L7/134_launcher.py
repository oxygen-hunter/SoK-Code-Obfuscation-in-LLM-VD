#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <sys/stat.h>

#define CFG_TEMPLATE "[http]\nclientapi.http.bind_address = localhost\nclientapi.http.port = {port}\nclient_http_base = http://localhost:{port}\nfederation.verifycerts = False\n\n[db]\ndb.file = :memory:\n\n[general]\nserver.name = test.local\nterms.path = {terms_path}\ntemplates.path = {testsubject_path}/res\nbrand.default = is-test\n\n\nip.whitelist = 127.0.0.1\n\n[email]\nemail.tlsmode = 0\nemail.invite.subject = %(sender_display_name)s has invited you to chat\nemail.smtphost = localhost\nemail.from = Sydent Validation <noreply@localhost>\nemail.smtpport = 9925\nemail.subject = Your Validation Token\n"

extern "C" {
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
}

class MatrixIsTestLauncher {
    bool with_terms;
    std::string tmpdir;
    int process_pid;
    std::string _baseUrl;

public:
    MatrixIsTestLauncher(bool with_terms) : with_terms(with_terms) {}

    void launch() {
        std::string sydent_path = realpath(".", nullptr);
        std::string testsubject_path = sydent_path + "/matrix_is_test";
        std::string terms_path = with_terms ? (testsubject_path + "/terms.yaml") : "";
        int port = with_terms ? 8099 : 8098;

        char temp_dir_template[] = "/tmp/sydenttestXXXXXX";
        char *tmpdir_c = mkdtemp(temp_dir_template);
        tmpdir = std::string(tmpdir_c);

        std::ofstream cfgfp(tmpdir + "/sydent.conf");
        cfgfp << std::string(CFG_TEMPLATE)
                 .replace(std::string::npos, std::string::npos, std::string("{testsubject_path}"), testsubject_path)
                 .replace(std::string::npos, std::string::npos, std::string("{terms_path}"), terms_path)
                 .replace(std::string::npos, std::string::npos, std::string("{port}"), std::to_string(port));
        cfgfp.close();

        std::string pythonpath = "PYTHONPATH=" + sydent_path;
        setenv("PYTHONPATH", sydent_path.c_str(), 1);

        std::string stderr_path = testsubject_path + "/sydent.stderr";
        int stderr_fd = open(stderr_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        const char *pybin = std::getenv("SYDENT_PYTHON");
        if (!pybin) pybin = "python";

        process_pid = fork();
        if (process_pid == 0) {
            dup2(stderr_fd, STDERR_FILENO);
            execlp(pybin, pybin, "-m", "sydent.sydent", nullptr);
            perror("execlp");
            exit(EXIT_FAILURE);
        }

        close(stderr_fd);
        sleep(2);

        _baseUrl = "http://localhost:" + std::to_string(port);
    }

    void tearDown() {
        std::cout << "Stopping sydent..." << std::endl;
        kill(process_pid, SIGTERM);
        waitpid(process_pid, nullptr, 0);
        rmdir(tmpdir.c_str());
    }

    std::string get_base_url() {
        return _baseUrl;
    }
};