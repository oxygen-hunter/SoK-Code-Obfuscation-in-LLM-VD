#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern "C" {
    #include <sys/signal.h>
}

class DaemonizerException {
public:
    explicit DaemonizerException(const std::string& msg) : msg(msg) {}
    const char* what() const noexcept { return msg.c_str(); }
private:
    std::string msg;
};

class Daemonizer {
public:
    explicit Daemonizer(const std::string& pidfile = "") {
        if (pidfile.empty()) {
            this->pidfile = "/var/run/pyro-" + toLower(typeid(*this).name()) + ".pid";
        } else {
            this->pidfile = pidfile;
        }
    }

    void become_daemon(const std::string& root_dir = "/") {
        if (fork() != 0) {
            _exit(0);
        }
        setsid();
        chdir(root_dir.c_str());
        umask(0);
        if (fork() != 0) {
            _exit(0);
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        for (int fd = 0; fd < 1024; ++fd) {
            close(fd);
        }
    }

    void daemon_start(bool start_as_daemon = true) {
        if (start_as_daemon) {
            become_daemon();
        }
        if (is_process_running()) {
            throw DaemonizerException("Unable to start server. Process is already running.");
        }
        std::ofstream f(pidfile);
        f << getpid();
        main_loop();
    }

    void daemon_stop() {
        pid_t pid = get_pid();
        kill(pid, SIGINT);
        sleep(1);
        unlink(pidfile.c_str());
    }

    pid_t get_pid() {
        std::ifstream f(pidfile);
        pid_t pid;
        f >> pid;
        return pid;
    }

    bool is_process_running() {
        pid_t pid = get_pid();
        return (kill(pid, 0) == 0);
    }

    virtual void main_loop() {
        throw DaemonizerException("main_loop method not implemented in derived class: " + std::string(typeid(*this).name()));
    }

    void process_command_line(int argc, char* argv[]) {
        if (argc < 2) {
            std::cerr << "usage: " << argv[0] << " start | stop | restart | status | debug [--pidfile=...]" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string operation = argv[1];
        if (argc > 2 && std::string(argv[2]).substr(0, 10) == "--pidfile=") {
            pidfile = std::string(argv[2]).substr(10);
        }
        pid_t pid = get_pid();
        if (operation == "status") {
            std::cout << (is_process_running() ? "Server process is running." : "Server is not running.") << std::endl;
        } else if (operation == "start") {
            if (is_process_running()) {
                std::cerr << "Server process is already running." << std::endl;
                exit(EXIT_FAILURE);
            } else {
                std::cout << "Starting server process." << std::endl;
                daemon_start();
            }
        } else if (operation == "stop") {
            if (is_process_running()) {
                daemon_stop();
                std::cout << "Server process stopped." << std::endl;
            } else {
                std::cerr << "Server process is not running." << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (operation == "restart") {
            daemon_stop();
            std::cout << "Restarting server process." << std::endl;
            daemon_start();
        } else if (operation == "debug") {
            daemon_start(false);
        } else {
            std::cerr << "Unknown operation: " << operation << std::endl;
            exit(EXIT_FAILURE);
        }
    }

private:
    std::string pidfile;

    std::string toLower(const std::string& str) {
        std::string result = str;
        for (char & c : result) c = tolower(c);
        return result;
    }
};

class Test : public Daemonizer {
public:
    Test() : Daemonizer() {}

    void main_loop() override {
        while (true) {
            sleep(1);
        }
    }
};

int main(int argc, char* argv[]) {
    Test test;
    test.process_command_line(argc, argv);
    return 0;
}