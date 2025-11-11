#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "horovod/runner/common/util/safe_shell_exec.h"
#include "horovod/runner/util/lsf.h"
#include "horovod/runner/mpi_run.h"

// Inline assembly example, mixing C++ with assembly
extern "C" inline void asm_example() {
    asm volatile ( 
        "nop\n"
    );
}

bool is_jsrun_installed() {
    // Usage of system call to check for jsrun
    return system("which jsrun > /dev/null 2>&1") == 0;
}

void js_run(Settings& settings, std::vector<std::string>& nics, std::map<std::string, std::string>& env, 
            std::vector<std::string>& command, std::ostream* stdout = nullptr, std::ostream* stderr = nullptr) {
    std::vector<std::string> mpi_impl_flags = _get_mpi_implementation_flags(settings.tcp_flag, env);

    if (mpi_impl_flags.empty()) {
        throw std::runtime_error(_MPI_NOT_FOUND_ERROR_MSG);
    }

    if (!is_jsrun_installed()) {
        throw std::runtime_error("horovod does not find the jsrun command.\n\n"
                                 "Please, make sure you are running on a cluster with jsrun installed or "
                                 "use one of the other launchers.");
    }

    if (!nics.empty() && env.find("NCCL_SOCKET_IFNAME") == env.end()) {
        env["NCCL_SOCKET_IFNAME"] = std::accumulate(std::next(nics.begin()), nics.end(), nics[0],
                                                    [](std::string a, std::string b) { return a + ',' + b; });
    }

    std::ostringstream smpiargs;
    for (const auto& flag : mpi_impl_flags) {
        smpiargs << flag << ' ';
    }
    if (!settings.extra_mpi_args.empty()) {
        smpiargs << settings.extra_mpi_args;
    }

    std::string binding_args;
    if (!settings.binding_args.empty()) {
        binding_args = settings.binding_args;
    } else {
        std::string rf = generate_jsrun_rankfile(settings);
        if (settings.verbose >= 2) {
            safe_shell_exec::execute("cat " + rf);
        }
        binding_args = "--erf_input " + rf;
    }

    std::ostringstream jsrun_command;
    jsrun_command << "jsrun " << binding_args << " "
                  << (settings.output_filename.empty() ? "" : "--stdio_stderr " + settings.output_filename + " --stdio_stdout " + settings.output_filename) << " "
                  << (smpiargs.str().empty() ? "" : "--smpiargs " + smpiargs.str()) << " ";
    for (const auto& par : command) {
        jsrun_command << quote(par) << " ";
    }

    if (settings.verbose >= 2) {
        std::cout << jsrun_command.str() << std::endl;
    }

    if (settings.run_func_mode) {
        int exit_code = safe_shell_exec::execute(jsrun_command.str(), env, stdout, stderr);
        if (exit_code != 0) {
            throw std::runtime_error("jsrun failed with exit code " + std::to_string(exit_code));
        }
    } else {
        char* args[] = {(char*)"/bin/sh", (char*)"-c", (char*)jsrun_command.str().c_str(), NULL};
        execve("/bin/sh", args, NULL);
    }
}

std::string generate_jsrun_rankfile(Settings& settings, const std::string& path = "") {
    int cpu_per_gpu = (lsf::LSFUtils::get_num_cores() * lsf::LSFUtils::get_num_threads()) / lsf::LSFUtils::get_num_gpus();
    std::istringstream hosts_stream(settings.hosts);
    std::string host_entry;
    std::vector<std::pair<std::string, int>> host_list;

    while (std::getline(hosts_stream, host_entry, ',')) {
        std::istringstream host_pair(host_entry);
        std::string host;
        int slots;
        if (std::getline(host_pair, host, ':') && (host_pair >> slots)) {
            if (slots > lsf::LSFUtils::get_num_gpus()) {
                throw std::invalid_argument("Invalid host input, slot count for host '" + host + ":" + std::to_string(slots) +
                                            "' is greater than number of GPUs per host '" + std::to_string(lsf::LSFUtils::get_num_gpus()) + "'.");
            }
            host_list.emplace_back(host, slots);
        }
    }

    std::vector<std::pair<std::string, int>> validated_list;
    int remaining_slots = settings.num_proc;
    for (auto& [host, slots] : host_list) {
        int needed_slots = std::min(slots, remaining_slots);
        validated_list.emplace_back(host, needed_slots);
        remaining_slots -= needed_slots;
        if (remaining_slots == 0) {
            break;
        }
    }

    if (remaining_slots != 0) {
        throw std::invalid_argument("Not enough slots on the hosts to fulfill the " + std::to_string(settings.num_proc) + " requested.");
    }

    char filename[] = "/tmp/rankfileXXXXXX";
    int fd = mkstemp(filename);
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file.");
    }
    close(fd);

    std::ofstream tmp(filename);
    tmp << "overlapping_rs: allow\n";
    tmp << "cpu_index_using: logical\n";
    int rank = 0;
    for (auto& [host, slots] : validated_list) {
        int cpu_val = 0;
        tmp << "\n";
        for (int s = 0; s < slots; ++s) {
            tmp << "rank: " << rank << ": { hostname: " << host << "; cpu: {" << cpu_val << "-" << (cpu_val + cpu_per_gpu - 1) << "} ; gpu: * ; mem: * }\n";
            ++rank;
            cpu_val += cpu_per_gpu;
        }
    }
    return std::string(filename);
}