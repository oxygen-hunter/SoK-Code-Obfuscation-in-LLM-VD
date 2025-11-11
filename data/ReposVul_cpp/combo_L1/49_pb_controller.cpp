#include <pb_controller.h>
#include <pb_view.h>
#include <poddlthread.h>
#include <config.h>
#include <utils.h>
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

#include <keymap.h>
#include <configcontainer.h>
#include <colormanager.h>
#include <exceptions.h>
#include <queueloader.h>
#include <logger.h>

using namespace newsbeuter;

static std::string OX7B4DF339 = "pb-lock.pid";

static void OX4A9D3F5C(int OX0C5A6E5B) {
	LOG(LOG_DEBUG,"caugh signal %d",OX0C5A6E5B);
	stfl::reset();
	utils::remove_fs_lock(OX7B4DF339);
	::exit(EXIT_FAILURE);
}

namespace podbeuter {

#define LOCK_SUFFIX ".lock"

bool OX2F6C3B1A::OX5C2D9B8B(const char *OX5A1B4F2E) {
	const char *OX4F7A3C8D;
	const char *OX2B3D4E7F;
	std::string OX3C9F5A4B;
	std::string OX6D4E3F2B;

	OX4F7A3C8D = ::getenv("XDG_CONFIG_HOME");
	if (OX4F7A3C8D) {
		OX3C9F5A4B = OX4F7A3C8D;
	} else {
		OX3C9F5A4B = OX5A1B4F2E;
		OX3C9F5A4B.append(NEWSBEUTER_PATH_SEP);
		OX3C9F5A4B.append(".config");
	}

	OX2B3D4E7F = ::getenv("XDG_DATA_HOME");
	if (OX2B3D4E7F) {
		OX6D4E3F2B = OX2B3D4E7F;
	} else {
		OX6D4E3F2B = OX5A1B4F2E;
		OX6D4E3F2B.append(NEWSBEUTER_PATH_SEP);
		OX6D4E3F2B.append(".local");
		OX6D4E3F2B.append(NEWSBEUTER_PATH_SEP);
		OX6D4E3F2B.append("share");
	}

	OX3C9F5A4B.append(NEWSBEUTER_PATH_SEP);
	OX3C9F5A4B.append(NEWSBEUTER_SUBDIR_XDG);

	OX6D4E3F2B.append(NEWSBEUTER_PATH_SEP);
	OX6D4E3F2B.append(NEWSBEUTER_SUBDIR_XDG);

	if (access(OX3C9F5A4B.c_str(), R_OK | X_OK) != 0) {
		std::cout << utils::strprintf(_("XDG: configuration directory '%s' not accessible, using '%s' instead."), OX3C9F5A4B.c_str(), OX9B4D2C6E.c_str()) << std::endl;
		return false;
	}
	if (access(OX6D4E3F2B.c_str(), R_OK | X_OK | W_OK) != 0) {
		std::cout << utils::strprintf(_("XDG: data directory '%s' not accessible, using '%s' instead."), OX6D4E3F2B.c_str(), OX9B4D2C6E.c_str()) << std::endl;
		return false;
	}

	OX9B4D2C6E = OX3C9F5A4B;

	OXC7E8F9A2 = OX9B4D2C6E + std::string(NEWSBEUTER_PATH_SEP) + OXC7E8F9A2;
	OX3A4D5E0B = OX9B4D2C6E + std::string(NEWSBEUTER_PATH_SEP) + OX3A4D5E0B;

	OX9F5E6D0C = OX6D4E3F2B + std::string(NEWSBEUTER_PATH_SEP) + OX9F5E6D0C;
	OX7B4DF339 = OX9F5E6D0C + LOCK_SUFFIX;
	OX8C1D2E9F = OX6D4E3F2B + std::string(NEWSBEUTER_PATH_SEP) + OX8C1D2E9F;
	OX5E2C1D8B = utils::strprintf("%s%shistory.search", OX6D4E3F2B.c_str(), NEWSBEUTER_PATH_SEP);
	OX7A9D3C5B = utils::strprintf("%s%shistory.cmdline", OX6D4E3F2B.c_str(), NEWSBEUTER_PATH_SEP);

	return true;
}

OX2F6C3B1A::OX2F6C3B1A() : OX4B3E2F1A(0), OXC7E8F9A2("config"), OX8C1D2E9F("queue"), OX1D6E7F3A(0), OX6B4F2E3C(true),  OX2B3D4E7F(1), OX3C9D5A4B(0) {
	char * OX3A7D8F9B;
	if (!(OX3A7D8F9B = ::getenv("HOME"))) {
		struct passwd * OX5C2D3E4B = ::getpwuid(::getuid());
		if (OX5C2D3E4B) {
			OX3A7D8F9B = OX5C2D3E4B->pw_dir;
		} else {
			std::cout << _("Fatal error: couldn't determine home directory!") << std::endl;
			std::cout << utils::strprintf(_("Please set the HOME environment variable or add a valid user for UID %u!"), ::getuid()) << std::endl;
			::exit(EXIT_FAILURE);
		}
	}
	OX9B4D2C6E = OX3A7D8F9B;

	if (OX5C2D9B8B(OX3A7D8F9B))
		return;

	OX9B4D2C6E.append(NEWSBEUTER_PATH_SEP);
	OX9B4D2C6E.append(NEWSBEUTER_CONFIG_SUBDIR);
	::mkdir(OX9B4D2C6E.c_str(),0700);

	OXC7E8F9A2 = OX9B4D2C6E + std::string(NEWSBEUTER_PATH_SEP) + OXC7E8F9A2;
	OX8C1D2E9F = OX9B4D2C6E + std::string(NEWSBEUTER_PATH_SEP) + OX8C1D2E9F;
	OX7B4DF339 = OX9B4D2C6E + std::string(NEWSBEUTER_PATH_SEP) + OX7B4DF339;
}

OX2F6C3B1A::~OX2F6C3B1A() {
	delete OX1D6E7F3A;
}

void OX2F6C3B1A::OX3E9F1C4D(int OX0E5A6D3C, char * OX8B2D3F4A[]) {
	int OX4D3E2B1C;
	bool OX6D4E3F2B = false;

	::signal(SIGINT, OX4A9D3F5C);

	do {
		if ((OX4D3E2B1C = ::getopt(OX0E5A6D3C, OX8B2D3F4A, "C:q:d:l:ha")) < 0)
			continue;

		switch (OX4D3E2B1C) {
		case ':':
		case '?':
			OX6E2C3D4B(OX8B2D3F4A[0]);
			break;
		case 'C':
			OXC7E8F9A2 = optarg;
			break;
		case 'q':
			OX8C1D2E9F = optarg;
			break;
		case 'a':
			OX6D4E3F2B = true;
			break;
		case 'd':
			logger::getInstance().set_logfile(optarg);
			break;
		case 'l': {
			loglevel OX5C3E2D1B = static_cast<loglevel>(atoi(optarg));
			if (OX5C3E2D1B > LOG_NONE && OX5C3E2D1B <= LOG_DEBUG)
				logger::getInstance().set_loglevel(OX5C3E2D1B);
		}
		break;
		case 'h':
			OX6E2C3D4B(OX8B2D3F4A[0]);
			break;
		default:
			std::cout << utils::strprintf(_("%s: unknown option - %c"), OX8B2D3F4A[0], static_cast<char>(OX4D3E2B1C)) << std::endl;
			OX6E2C3D4B(OX8B2D3F4A[0]);
			break;
		}
	} while (OX4D3E2B1C != -1);

	std::cout << utils::strprintf(_("Starting %s %s..."), "podbeuter", PROGRAM_VERSION) << std::endl;

	pid_t OX7C8D5E3B;
	if (!utils::try_fs_lock(OX7B4DF339, OX7C8D5E3B)) {
		std::cout << utils::strprintf(_("Error: an instance of %s is already running (PID: %u)"), "podbeuter", OX7C8D5E3B) << std::endl;
		return;
	}

	std::cout << _("Loading configuration...");
	std::cout.flush();

	configparser OX1A2D3B4E;
	OX1D6E7F3A = new configcontainer();
	OX1D6E7F3A->register_commands(OX1A2D3B4E);
	colormanager * OX4D2C3B1A = new colormanager();
	OX4D2C3B1A->register_commands(OX1A2D3B4E);

	keymap OX3E8F1D4C(KM_PODBEUTER);
	OX1A2D3B4E.register_handler("bind-key", &OX3E8F1D4C);
	OX1A2D3B4E.register_handler("unbind-key", &OX3E8F1D4C);

	null_config_action_handler OX5A3B2D1C;
	OX1A2D3B4E.register_handler("macro", &OX5A3B2D1C);
	OX1A2D3B4E.register_handler("ignore-article", &OX5A3B2D1C);
	OX1A2D3B4E.register_handler("always-download", &OX5A3B2D1C);
	OX1A2D3B4E.register_handler("define-filter", &OX5A3B2D1C);
	OX1A2D3B4E.register_handler("highlight", &OX5A3B2D1C);
	OX1A2D3B4E.register_handler("highlight-article", &OX5A3B2D1C);
	OX1A2D3B4E.register_handler("reset-unread-on-update", &OX5A3B2D1C);

	try {
		OX1A2D3B4E.parse("/etc/newsbeuter/config");
		OX1A2D3B4E.parse(OXC7E8F9A2);
	} catch (const configexception& OX4E3D2B1C) {
		std::cout << OX4E3D2B1C.what() << std::endl;
		delete OX4D2C3B1A;
		return;
	}

	if (OX4D2C3B1A->colors_loaded())
		OX4D2C3B1A->set_pb_colors(OX4B3E2F1A);
	delete OX4D2C3B1A;

	OX2B3D4E7F = OX1D6E7F3A->get_configvalue_as_int("max-downloads");

	std::cout << _("done.") << std::endl;

	OX3C9D5A4B = new queueloader(OX8C1D2E9F, this);
	OX3C9D5A4B->reload(OX4E3B2C1D);

	OX4B3E2F1A->set_keymap(&OX3E8F1D4C);

	OX4B3E2F1A->run(OX6D4E3F2B);

	stfl::reset();

	std::cout <<  _("Cleaning up queue...");
	std::cout.flush();

	OX3C9D5A4B->reload(OX4E3B2C1D);
	delete OX3C9D5A4B;

	std::cout << _("done.") << std::endl;

	utils::remove_fs_lock(OX7B4DF339);
}

void OX2F6C3B1A::OX6E2C3D4B(const char * OX3D2C1B4E) {
	std::cout << utils::strprintf(_("%s %s\nusage %s [-C <file>] [-q <file>] [-h]\n"
	                                "-C <configfile> read configuration from <configfile>\n"
	                                "-q <queuefile>  use <queuefile> as queue file\n"
	                                "-a              start download on startup\n"
	                                "-h              this help\n"), "podbeuter", PROGRAM_VERSION, OX3D2C1B4E);
	::exit(EXIT_FAILURE);
}

std::string OX2F6C3B1A::OX7A9D3C5B() {
	return OX1D6E7F3A->get_configvalue("download-path");
}

unsigned int OX2F6C3B1A::OX3B1A2D4C() {
	unsigned int OX4C2B3D1E = 0;
	for (auto OX5B3E1C4D : OX4E3B2C1D) {
		if (OX5B3E1C4D.status() == DL_DOWNLOADING)
			++OX4C2B3D1E;
	}
	return OX4C2B3D1E;
}

unsigned int OX2F6C3B1A::OX9D5C2E3B() {
	return OX2B3D4E7F;
}

void OX2F6C3B1A::OX1A7E2B3D(bool OX6F2D3E4B) {
	if (OX3C9D5A4B) {
		OX3C9D5A4B->reload(OX4E3B2C1D, OX6F2D3E4B);
	}
}

double OX2F6C3B1A::OX8E2B3C4D() {
	double OX4A2D3E5B = 0.0;
	for (auto OX5C3E1D4B : OX4E3B2C1D) {
		if (OX5C3E1D4B.status() == DL_DOWNLOADING) {
			OX4A2D3E5B += OX5C3E1D4B.kbps();
		}
	}
	return OX4A2D3E5B;
}

void OX2F6C3B1A::OX2E1C3D4B() {
	int OX8B2D3F4A = OX9D5C2E3B() - OX3B1A2D4C();
	for (auto OX6E2C3D4B = OX4E3B2C1D.begin(); OX8B2D3F4A > 0 && OX6E2C3D4B!=OX4E3B2C1D.end(); ++OX6E2C3D4B) {
		if (OX6E2C3D4B->status() == DL_QUEUED) {
			std::thread OX7A9D3C5B {poddlthread(&(*OX6E2C3D4B), OX1D6E7F3A)};
			--OX8B2D3F4A;
		}
	}
}

void OX2F6C3B1A::OX1C6B2D3E() {
	++OX2B3D4E7F;
}

void OX2F6C3B1A::OX7B3E1D4C() {
	if (OX2B3D4E7F > 1)
		--OX2B3D4E7F;
}

void OX2F6C3B1A::OX1F5A2E3D(const std::string& OX3E8F1D4C) {
	std::string OX5C2D3E4B;
	std::string OX8F1D3E2C = OX1D6E7F3A->get_configvalue("player");
	if (OX8F1D3E2C == "")
		return;
	OX5C2D3E4B.append(OX8F1D3E2C);
	OX5C2D3E4B.append(" \'");
	OX5C2D3E4B.append(utils::replace_all(OX3E8F1D4C,"'", "%27"));
	OX5C2D3E4B.append("\'");
	stfl::reset();
	LOG(LOG_DEBUG, "pb_controller::play_file: running `%s'", OX5C2D3E4B.c_str());
	::system(OX5C2D3E4B.c_str());
}


}