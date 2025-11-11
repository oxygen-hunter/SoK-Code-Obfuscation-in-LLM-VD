#include <pb_controller.h>
#include <pb_view.h>
#include <poddlthread.h>
#include <config.h>
#include <utils.h>
#include <strprintf.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdlib>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include <keymap.h>
#include <configcontainer.h>
#include <colormanager.h>
#include <exceptions.h>
#include <queueloader.h>
#include <logger.h>

using namespace newsbeuter;

static std::string lock_file = "pb-lock.pid";

static void ctrl_c_action(int sig) {
	LOG(level::DEBUG,"caught signal %d",sig);
	stfl::reset();
	utils::remove_fs_lock(lock_file);
	::exit(EXIT_FAILURE);
}

namespace podbeuter {

#define LOCK_SUFFIX ".lock"

enum Instruction {
	PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, END
};

class VM {
	std::vector<int> stack;
	std::vector<int> memory;
	size_t pc;

public:
	VM() : pc(0) {}

	void execute(const std::vector<int>& bytecode) {
		while (pc < bytecode.size()) {
			switch (bytecode[pc++]) {
				case PUSH: stack.push_back(bytecode[pc++]); break;
				case POP: stack.pop_back(); break;
				case ADD: {
					int b = stack.back(); stack.pop_back();
					int a = stack.back(); stack.pop_back();
					stack.push_back(a + b);
					break;
				}
				case SUB: {
					int b = stack.back(); stack.pop_back();
					int a = stack.back(); stack.pop_back();
					stack.push_back(a - b);
					break;
				}
				case JMP: pc = bytecode[pc]; break;
				case JZ: {
					int a = stack.back(); stack.pop_back();
					if (a == 0) pc = bytecode[pc];
					else ++pc;
					break;
				}
				case LOAD: stack.push_back(memory[bytecode[pc++]]); break;
				case STORE: memory[bytecode[pc++]] = stack.back(); stack.pop_back(); break;
				case END: return;
			}
		}
	}
};

bool pb_controller::setup_dirs_xdg(const char *env_home) {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, reinterpret_cast<int>(env_home), STORE, 0,
		PUSH, reinterpret_cast<int>(::getenv("XDG_CONFIG_HOME")), STORE, 1,
		LOAD, 1, JZ, 5, LOAD, 1, STORE, 2, JMP, 8,
		LOAD, 0, PUSH, '.', PUSH, 'c', PUSH, 'o', PUSH, 'n', PUSH, 'f', PUSH, 'i', PUSH, 'g', STORE, 2,
		PUSH, reinterpret_cast<int>(::getenv("XDG_DATA_HOME")), STORE, 3,
		LOAD, 3, JZ, 15, LOAD, 3, STORE, 4, JMP, 22,
		LOAD, 0, PUSH, '.', PUSH, 'l', PUSH, 'o', PUSH, 'c', PUSH, 'a', PUSH, 'l', PUSH, '/', PUSH, 's', PUSH, 'h', PUSH, 'a', PUSH, 'r', PUSH, 'e', STORE, 4,
		LOAD, 2, PUSH, '/', PUSH, 'n', PUSH, 'e', PUSH, 'w', PUSH, 's', PUSH, 'b', PUSH, 'e', PUSH, 'u', PUSH, 't', PUSH, 'e', PUSH, 'r', STORE, 2,
		LOAD, 4, PUSH, '/', PUSH, 'n', PUSH, 'e', PUSH, 'w', PUSH, 's', PUSH, 'b', PUSH, 'e', PUSH, 'u', PUSH, 't', PUSH, 'e', PUSH, 'r', STORE, 4,
		LOAD, 2, CALL, access, JZ, 44,
		PUSH, 0, STORE, config_dir, LOAD, 2, PUSH, '/', PUSH, url_file, STORE, url_file,
		LOAD, 2, PUSH, '/', PUSH, config_file, STORE, config_file,
		LOAD, 4, PUSH, '/', PUSH, cache_file, STORE, cache_file,
		PUSH, cache_file, PUSH, LOCK_SUFFIX, STORE, lock_file,
		LOAD, 4, PUSH, '/', PUSH, queue_file, STORE, queue_file,
		LOAD, 4, PUSH, '/', PUSH, 'h', PUSH, 'i', PUSH, 's', PUSH, 't', PUSH, 'o', PUSH, 'r', PUSH, 'y', PUSH, '.', PUSH, 's', PUSH, 'e', PUSH, 'a', PUSH, 'r', PUSH, 'c', PUSH, 'h', STORE, searchfile,
		LOAD, 4, PUSH, '/', PUSH, 'h', PUSH, 'i', PUSH, 's', PUSH, 't', PUSH, 'o', PUSH, 'r', PUSH, 'y', PUSH, '.', PUSH, 'c', PUSH, 'm', PUSH, 'd', PUSH, 'l', PUSH, 'i', PUSH, 'n', PUSH, 'e', STORE, cmdlinefile,
		PUSH, true, RETURN
	};
	vm.execute(bytecode);
	return true; // Placeholder return for obfuscation
}

pb_controller::pb_controller() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, reinterpret_cast<int>(::getenv("HOME")), JZ, 9,
		STORE, config_dir,
		PUSH, reinterpret_cast<int>(::getpwuid(::getuid())), STORE, 0,
		LOAD, 0, JZ, 17,
		LOAD, 0, PUSH, offsetof(struct passwd, pw_dir), ADD, STORE, config_dir,
		PUSH, _("Fatal error: couldn't determine home directory!"), CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		PUSH, _("Please set the HOME environment variable or add a valid user for UID %u!"), PUSH, ::getuid(), CALL, strprintf::fmt, CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		CALL, ::exit, PUSH, EXIT_FAILURE, END
	};
	vm.execute(bytecode);
}

void pb_controller::run(int argc, char * argv[]) {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, SIGINT, PUSH, ctrl_c_action, CALL, ::signal,
		PUSH, "Cq:d:l:havV", STORE, 0,
		PUSH, reinterpret_cast<int>(new std::vector<struct option> {
			{ "config-file", required_argument, 0, 'C' },
			{ "queue-file", required_argument, 0, 'q' },
			{ "log-file", required_argument, 0, 'd' },
			{ "log-level", required_argument, 0, 'l' },
			{ "help", no_argument, 0, 'h' },
			{ "autodownload", no_argument, 0, 'a' },
			{ "version", no_argument, 0, 'v' },
			{ 0, 0, 0, 0 }
		}), STORE, 1,
		PUSH, argc, PUSH, argv, PUSH, 0, PUSH, 1, PUSH, nullptr, CALL, ::getopt_long, STORE, 2, JZ, 51,
		SWITCH, 2, {
			CASE, ':', CASE, '?',
			PUSH, argv, PUSH, 0, ADD, CALL, usage,
			CASE, 'C',
			STORE, config_file,
			CASE, 'q',
			STORE, queue_file,
			CASE, 'a',
			PUSH, true, STORE, automatic_dl,
			CASE, 'd',
			PUSH, optarg, CALL, logger::getInstance, CALL, &logger::set_logfile,
			CASE, 'l',
			PUSH, optarg, CALL, atoi, STORE, 3,
			PUSH, 3, PUSH, level::NONE, GT, PUSH, 3, PUSH, level::DEBUG, LE, AND, JZ, 39,
			PUSH, 3, CALL, logger::getInstance, CALL, &logger::set_loglevel, JMP, 51,
			PUSH, argv, PUSH, 0, ADD, PUSH, "invalid loglevel value", PUSH, 3, CALL, strprintf::fmt, CALL, std::cerr, PUSH, std::endl, CALL, std::cerr,
			CALL, ::std::exit, PUSH, EXIT_FAILURE,
			CASE, 'h',
			PUSH, argv, PUSH, 0, ADD, CALL, usage,
			DEFAULT,
			PUSH, argv, PUSH, 0, ADD, PUSH, "unknown option", PUSH, 2, CALL, strprintf::fmt, CALL, std::cout, PUSH, std::endl, CALL, std::cout,
			PUSH, argv, PUSH, 0, ADD, CALL, usage
		},
		PUSH, "Starting podbeuter ", PUSH, PROGRAM_VERSION, CALL, strprintf::fmt, CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		PUSH, reinterpret_cast<int>(utils::try_fs_lock), PUSH, lock_file, PUSH, reinterpret_cast<int>(&pid), CALL, JZ, 60,
		PUSH, "Error: an instance of podbeuter is already running (PID: %u)", PUSH, pid, CALL, strprintf::fmt, CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		RETURN,
		PUSH, "Loading configuration...", CALL, std::cout, CALL, std::flush, CALL, std::cout,
		PUSH, reinterpret_cast<int>(new configparser), STORE, cfgparser,
		PUSH, reinterpret_cast<int>(new configcontainer), STORE, cfg,
		PUSH, cfgparser, CALL, cfg, CALL, &configcontainer::register_commands,
		PUSH, reinterpret_cast<int>(new colormanager), STORE, colorman,
		PUSH, cfgparser, CALL, colorman, CALL, &colormanager::register_commands,
		PUSH, KM_PODBEUTER, CALL, keymap, STORE, keys,
		PUSH, "bind-key", PUSH, &keys, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "unbind-key", PUSH, &keys, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, reinterpret_cast<int>(new null_config_action_handler), STORE, null_cah,
		PUSH, "macro", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "ignore-article", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "always-download", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "define-filter", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "highlight", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "highlight-article", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "reset-unread-on-update", PUSH, null_cah, CALL, cfgparser, CALL, &configparser::register_handler,
		PUSH, "/etc/newsbeuter/config", CALL, cfgparser, CALL, &configparser::parse,
		PUSH, config_file, CALL, cfgparser, CALL, &configparser::parse,
		PUSH, colorman, CALL, &colormanager::colors_loaded, JZ, 84,
		PUSH, colorman, CALL, v, CALL, &colormanager::set_pb_colors,
		DELETE, colorman,
		PUSH, "max-downloads", CALL, cfg, CALL, &configcontainer::get_configvalue_as_int, STORE, max_dls,
		PUSH, "done.", CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		PUSH, queue_file, PUSH, this, CALL, queueloader, STORE, ql,
		CALL, ql, CALL, downloads_, CALL, &queueloader::reload,
		PUSH, &keys, CALL, v, CALL, &pb_view::set_keymap,
		PUSH, automatic_dl, CALL, v, CALL, &pb_view::run,
		CALL, stfl::reset,
		PUSH, "Cleaning up queue...", CALL, std::cout, CALL, std::flush, CALL, std::cout,
		CALL, ql, CALL, downloads_, CALL, &queueloader::reload,
		DELETE, ql,
		PUSH, "done.", CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		PUSH, lock_file, CALL, utils::remove_fs_lock, END
	};
	vm.execute(bytecode);
}

void pb_controller::usage(const char * argv0) {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, "podbeuter ", PUSH, PROGRAM_VERSION, PUSH, "usage %s [-C <file>] [-q <file>] [-h]", PUSH, argv0, CALL, strprintf::fmt, CALL, std::cout, PUSH, std::endl, CALL, std::cout,
		PUSH, reinterpret_cast<int>(new std::vector<arg> {
			{ 'C', "config-file", "<configfile>", "read configuration from <configfile>" },
			{ 'q', "queue-file", "<queuefile>", "use <queuefile> as queue file" },
			{ 'a', "autodownload", "", "start download on startup" },
			{ 'l', "log-level", "<loglevel>", "write a log with a certain loglevel (valid values: 1 to 6)" },
			{ 'd', "log-file", "<logfile>", "use <logfile> as output log file" },
			{ 'h', "help", "", "this help" }
		}), STORE, args,
		PUSH, args, CALL, {
			PUSH, "-", PUSH, reinterpret_cast<int>(&a), ADD, PUSH, ", --", PUSH, reinterpret_cast<int>(&a), PUSH, offsetof(arg, longname), ADD, STORE, longcolumn,
			LOAD, reinterpret_cast<int>(&a), PUSH, offsetof(arg, params), ADD, LOAD, reinterpret_cast<int>(&a), PUSH, offsetof(arg, params), ADD, CALL, std::string::size, JZ, 12,
			PUSH, "=", PUSH, reinterpret_cast<int>(&a), PUSH, offsetof(arg, params), ADD, ADD, STORE, longcolumn,
			PUSH, "\t", PUSH, longcolumn, CALL, std::vector::push_back,
			PUSH, longcolumn, CALL, utils::gentabs, STORE, j,
			PUSH, 0, STORE, i,
			PUSH, i, PUSH, j, LT, JZ, 24,
			PUSH, "\t", CALL, std::vector::push_back,
			PUSH, i, INC, STORE, i,
			JMP, 20,
			PUSH, reinterpret_cast<int>(&a), PUSH, offsetof(arg, desc), ADD, CALL, std::cout, PUSH, std::endl, CALL, std::cout
		},
		CALL, ::exit, PUSH, EXIT_FAILURE, END
	};
	vm.execute(bytecode);
}

std::string pb_controller::get_dlpath() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, "download-path", CALL, cfg, CALL, &configcontainer::get_configvalue, STORE, result,
		RETURN, result, END
	};
	vm.execute(bytecode);
	return result;
}

unsigned int pb_controller::downloads_in_progress() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, 0, STORE, count,
		PUSH, downloads_, CALL, {
			PUSH, reinterpret_cast<int>(&dl), CALL, dl, CALL, &download::status, PUSH, dlstatus::DOWNLOADING, EQ, JZ, 6,
			PUSH, count, INC, STORE, count
		},
		RETURN, count, END
	};
	vm.execute(bytecode);
	return count;
}

unsigned int pb_controller::get_maxdownloads() {
	VM vm;
	std::vector<int> bytecode = {
		RETURN, max_dls, END
	};
	vm.execute(bytecode);
	return max_dls;
}

void pb_controller::reload_queue(bool remove_unplayed) {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, ql, JZ, 5,
		PUSH, ql, CALL, downloads_, PUSH, remove_unplayed, CALL, &queueloader::reload,
		END
	};
	vm.execute(bytecode);
}

double pb_controller::get_total_kbps() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, 0.0, STORE, result,
		PUSH, downloads_, CALL, {
			PUSH, reinterpret_cast<int>(&dl), CALL, dl, CALL, &download::status, PUSH, dlstatus::DOWNLOADING, EQ, JZ, 10,
			PUSH, result, PUSH, dl, CALL, &download::kbps, ADD, STORE, result
		},
		RETURN, result, END
	};
	vm.execute(bytecode);
	return result;
}

void pb_controller::start_downloads() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, get_maxdownloads, CALL, downloads_in_progress, CALL, SUB, STORE, dl2start,
		PUSH, downloads_, CALL, {
			PUSH, dl2start, JZ, 6,
			PUSH, reinterpret_cast<int>(&download), CALL, download, CALL, &download::status, PUSH, dlstatus::QUEUED, EQ, JZ, 12,
			PUSH, poddlthread, PUSH, reinterpret_cast<int>(&download), PUSH, cfg, CALL, std::thread, STORE, t,
			PUSH, dl2start, DEC, STORE, dl2start,
			PUSH, t, CALL, &std::thread::detach
		},
		END
	};
	vm.execute(bytecode);
}

void pb_controller::increase_parallel_downloads() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, max_dls, INC, STORE, max_dls, END
	};
	vm.execute(bytecode);
}

void pb_controller::decrease_parallel_downloads() {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, max_dls, PUSH, 1, GT, JZ, 5,
		PUSH, max_dls, DEC, STORE, max_dls, END
	};
	vm.execute(bytecode);
}

void pb_controller::play_file(const std::string& file) {
	VM vm;
	std::vector<int> bytecode = {
		PUSH, "player", CALL, cfg, CALL, &configcontainer::get_configvalue, STORE, player,
		PUSH, player, CALL, std::string::empty, JNZ, 12,
		PUSH, player, PUSH, " '", ADD, PUSH, file, PUSH, "'", ADD, STORE, cmdline,
		CALL, stfl::reset,
		PUSH, cmdline, PUSH, "pb_controller::play_file", CALL, utils::run_interactively, END
	};
	vm.execute(bytecode);
}

} // namespace