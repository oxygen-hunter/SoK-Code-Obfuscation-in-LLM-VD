/* -*- coding: utf-8 -*- */
/*
Execute chef in server or solo mode
*/

// Import Python libs
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>

// Import Salt libs
#include "salt/utils.h"
#include "salt/utils/decorators.h"

// Inline Assembly Example
std::string get_logfile_tempname(const std::string& exe_name) {
    std::ostringstream filename;
    #ifdef _WIN32
    filename << std::getenv("TMP") << "\\" << exe_name << ".log";
    #else
    filename << "/var/log/" << exe_name << ".log";
    #endif
    return filename.str();
}

bool __virtual__() {
    /*
    Only load if chef is installed
    */
    return salt_utils::which("chef-client");
}

std::string client(bool whyrun = false, bool localmode = false, const std::string& logfile = get_logfile_tempname("chef-client"), const std::map<std::string, std::string>& kwargs = {}) {
    /*
    Execute a chef client run and return a dict with the stderr, stdout,
    return code, and pid.

    CLI Example:

    .. code-block:: bash

        salt '*' chef.client server=https://localhost
    */
    std::vector<std::string> args = {
        "chef-client",
        "--no-color",
        "--once",
        "--logfile " + logfile,
        "--format doc"
    };

    if (whyrun) {
        args.push_back("--why-run");
    }

    if (localmode) {
        args.push_back("--local-mode");
    }

    return _exec_cmd(args, kwargs);
}

std::string solo(bool whyrun = false, const std::string& logfile = get_logfile_tempname("chef-solo"), const std::map<std::string, std::string>& kwargs = {}) {
    /*
    Execute a chef solo run and return a dict with the stderr, stdout,
    return code, and pid.

    CLI Example:

    .. code-block:: bash

        salt '*' chef.solo override-runlist=test
    */
    std::vector<std::string> args = {
        "chef-solo",
        "--no-color",
        "--logfile " + logfile,
        "--format doc"
    };

    if (whyrun) {
        args.push_back("--why-run");
    }

    return _exec_cmd(args, kwargs);
}

std::string _exec_cmd(const std::vector<std::string>& args, const std::map<std::string, std::string>& kwargs) {
    // Compile the command arguments
    std::ostringstream cmd_args;
    for (const auto& arg : args) {
        cmd_args << " " << arg;
    }
    std::ostringstream cmd_kwargs;
    for (const auto& [k, v] : kwargs) {
        if (k.find("__") != 0) {
            cmd_kwargs << " --" << k << " " << v;
        }
    }
    std::string cmd_exec = cmd_args.str() + cmd_kwargs.str();
    std::cout << "Chef command: " << cmd_exec << std::endl;

    return salt_utils::cmd::run_all(cmd_exec, false);
}