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

static std::string OX97A74D1C = "pb-lock.pid";

static void OX0D91C1C4(int OXDC8F2A6A) {
	LOG(level::DEBUG,"caugh signal %d",OXDC8F2A6A);
	stfl::reset();
	utils::remove_fs_lock(OX97A74D1C);
	::exit(EXIT_FAILURE);
}

namespace podbeuter {

#define OX9D1D7A0E ".lock"

bool OX5FFB3BA1::OX6F9C0A7E(const char *OX7B6BD7EC) {
	const char *OX4A2163F6;
	const char *OX76590757;
	std::string OX7B4DF339;
	std::string OXB8302A7C;

	OX4A2163F6 = ::getenv("XDG_CONFIG_HOME");
	if (OX4A2163F6) {
		OX7B4DF339 = OX4A2163F6;
	} else {
		OX7B4DF339 = OX7B6BD7EC;
		OX7B4DF339.append(NEWSBEUTER_PATH_SEP);
		OX7B4DF339.append(".config");
	}

	OX76590757 = ::getenv("XDG_DATA_HOME");
	if (OX76590757) {
		OXB8302A7C = OX76590757;
	} else {
		OXB8302A7C = OX7B6BD7EC;
		OXB8302A7C.append(NEWSBEUTER_PATH_SEP);
		OXB8302A7C.append(".local");
		OXB8302A7C.append(NEWSBEUTER_PATH_SEP);
		OXB8302A7C.append("share");
	}

	OX7B4DF339.append(NEWSBEUTER_PATH_SEP);
	OX7B4DF339.append(NEWSBEUTER_SUBDIR_XDG);

	OXB8302A7C.append(NEWSBEUTER_PATH_SEP);
	OXB8302A7C.append(NEWSBEUTER_SUBDIR_XDG);

	bool OX3B9F3B4B = 0 == access(OX7B4DF339.c_str(), R_OK | X_OK);

	if (!OX3B9F3B4B) {
		std::cerr
		    << strprintf::fmt(
		           _("XDG: configuration directory '%s' not accessible, "
		             "using '%s' instead."),
		           OX7B4DF339,
		           OX35D1A5B1)
		    << std::endl;

		return false;
	}

	OX35D1A5B1 = OX7B4DF339;

	utils::mkdir_parents(OXB8302A7C, 0700);

	OX8A0F2A0F = OX35D1A5B1 + std::string(NEWSBEUTER_PATH_SEP) + OX8A0F2A0F;
	OX47C38F1B = OX35D1A5B1 + std::string(NEWSBEUTER_PATH_SEP) + OX47C38F1B;

	OXCB0DF7CB = OXB8302A7C + std::string(NEWSBEUTER_PATH_SEP) + OXCB0DF7CB;
	OX97A74D1C = OXCB0DF7CB + OX9D1D7A0E;
	OXFD77B3A2 = OXB8302A7C + std::string(NEWSBEUTER_PATH_SEP) + OXFD77B3A2;
	OXCD95F3FF = strprintf::fmt("%s%shistory.search", OXB8302A7C, NEWSBEUTER_PATH_SEP);
	OXB8E9D812 = strprintf::fmt("%s%shistory.cmdline", OXB8302A7C, NEWSBEUTER_PATH_SEP);

	return true;
}

OX5FFB3BA1::OX5FFB3BA1() : OX5F9B093E(0), OX47C38F1B("config"), OXFD77B3A2("queue"), OX1F8E8E3E(0), OX7FBC5B6B(true),  OX8680D1ED(1), OX7F9D2D5F(0) {
	char * OX4F0A0BFD;
	if (!(OX4F0A0BFD = ::getenv("HOME"))) {
		struct passwd * OX48B09D6F = ::getpwuid(::getuid());
		if (OX48B09D6F) {
			OX4F0A0BFD = OX48B09D6F->pw_dir;
		} else {
			std::cout << _("Fatal error: couldn't determine home directory!") << std::endl;
			std::cout << strprintf::fmt(_("Please set the HOME environment variable or add a valid user for UID %u!"), ::getuid()) << std::endl;
			::exit(EXIT_FAILURE);
		}
	}
	OX35D1A5B1 = OX4F0A0BFD;

	if (OX6F9C0A7E(OX4F0A0BFD))
		return;

	OX35D1A5B1.append(NEWSBEUTER_PATH_SEP);
	OX35D1A5B1.append(NEWSBEUTER_CONFIG_SUBDIR);
	::mkdir(OX35D1A5B1.c_str(),0700);

	OX47C38F1B = OX35D1A5B1 + std::string(NEWSBEUTER_PATH_SEP) + OX47C38F1B;
	OXFD77B3A2 = OX35D1A5B1 + std::string(NEWSBEUTER_PATH_SEP) + OXFD77B3A2;
	OX97A74D1C = OX35D1A5B1 + std::string(NEWSBEUTER_PATH_SEP) + OX97A74D1C;
}

OX5FFB3BA1::~OX5FFB3BA1() {
	delete OX1F8E8E3E;
}

void OX5FFB3BA1::OX7F9435A6(int OX9D3A8C0A, char * OX0E9F3C2F[]) {
	int OX0A6FC12B;
	bool OX20D6B7E1 = false;

	::signal(SIGINT, OX0D91C1C4);

	static const char OX54C1F7E3[] = "C:q:d:l:havV";
	static const struct option OX5EDB8B23[] = {
		{"config-file"     , required_argument, 0, 'C'},
		{"queue-file"      , required_argument, 0, 'q'},
		{"log-file"        , required_argument, 0, 'd'},
		{"log-level"       , required_argument, 0, 'l'},
		{"help"            , no_argument      , 0, 'h'},
		{"autodownload"    , no_argument      , 0, 'a'},
		{"version"         , no_argument      , 0, 'v'},
		{0                 , 0                , 0,  0 }
	};

	while ((OX0A6FC12B = ::getopt_long(OX9D3A8C0A, OX0E9F3C2F, OX54C1F7E3, OX5EDB8B23, nullptr)) != -1) {
		switch (OX0A6FC12B) {
		case ':':
		case '?':
			OXFBCB6C49(OX0E9F3C2F[0]);
			break;
		case 'C':
			OX47C38F1B = optarg;
			break;
		case 'q':
			OXFD77B3A2 = optarg;
			break;
		case 'a':
			OX20D6B7E1 = true;
			break;
		case 'd':
			logger::getInstance().set_logfile(optarg);
			break;
		case 'l': {
			level OX9A1A4D4B = static_cast<level>(atoi(optarg));
			if (OX9A1A4D4B > level::NONE && OX9A1A4D4B <= level::DEBUG) {
				logger::getInstance().set_loglevel(OX9A1A4D4B);
			} else {
				std::cerr << strprintf::fmt(_("%s: %d: invalid loglevel value"), OX0E9F3C2F[0], OX9A1A4D4B) << std::endl;
				::std::exit(EXIT_FAILURE);
			}
			}
			break;
		case 'h':
			OXFBCB6C49(OX0E9F3C2F[0]);
			break;
		default:
			std::cout << strprintf::fmt(_("%s: unknown option - %c"), OX0E9F3C2F[0], static_cast<char>(OX0A6FC12B)) << std::endl;
			OXFBCB6C49(OX0E9F3C2F[0]);
			break;
		}
	};

	std::cout << strprintf::fmt(_("Starting %s %s..."), "podbeuter", PROGRAM_VERSION) << std::endl;

	pid_t OX9C0B5F5A;
	if (!utils::try_fs_lock(OX97A74D1C, OX9C0B5F5A)) {
		std::cout << strprintf::fmt(_("Error: an instance of %s is already running (PID: %u)"), "podbeuter", OX9C0B5F5A) << std::endl;
		return;
	}

	std::cout << _("Loading configuration...");
	std::cout.flush();

	configparser OX1B3C3A5A;
	OX1F8E8E3E = new configcontainer();
	OX1F8E8E3E->register_commands(OX1B3C3A5A);
	colormanager * OX4E1F0C5F = new colormanager();
	OX4E1F0C5F->register_commands(OX1B3C3A5A);

	keymap OX17D9B7A8(KM_PODBEUTER);
	OX1B3C3A5A.register_handler("bind-key", &OX17D9B7A8);
	OX1B3C3A5A.register_handler("unbind-key", &OX17D9B7A8);

	null_config_action_handler OX184D3E9E;
	OX1B3C3A5A.register_handler("macro", &OX184D3E9E);
	OX1B3C3A5A.register_handler("ignore-article", &OX184D3E9E);
	OX1B3C3A5A.register_handler("always-download", &OX184D3E9E);
	OX1B3C3A5A.register_handler("define-filter", &OX184D3E9E);
	OX1B3C3A5A.register_handler("highlight", &OX184D3E9E);
	OX1B3C3A5A.register_handler("highlight-article", &OX184D3E9E);
	OX1B3C3A5A.register_handler("reset-unread-on-update", &OX184D3E9E);

	try {
		OX1B3C3A5A.parse("/etc/newsbeuter/config");
		OX1B3C3A5A.parse(OX47C38F1B);
	} catch (const configexception& OX6B8A9978) {
		std::cout << OX6B8A9978.what() << std::endl;
		delete OX4E1F0C5F;
		return;
	}

	if (OX4E1F0C5F->colors_loaded())
		OX4E1F0C5F->set_pb_colors(OX5F9B093E);
	delete OX4E1F0C5F;

	OX8680D1ED = OX1F8E8E3E->get_configvalue_as_int("max-downloads");

	std::cout << _("done.") << std::endl;

	OX7F9D2D5F = new queueloader(OXFD77B3A2, this);
	OX7F9D2D5F->reload(OX2B6BF3C0);

	OX5F9B093E->set_keymap(&OX17D9B7A8);

	OX5F9B093E->run(OX20D6B7E1);

	stfl::reset();

	std::cout <<  _("Cleaning up queue...");
	std::cout.flush();

	OX7F9D2D5F->reload(OX2B6BF3C0);
	delete OX7F9D2D5F;

	std::cout << _("done.") << std::endl;

	utils::remove_fs_lock(OX97A74D1C);
}

void OX5FFB3BA1::OXFBCB6C49(const char * OX6A1FA92C) {
	auto OX531F5F48 =
	    strprintf::fmt(_("%s %s\nusage %s [-C <file>] [-q <file>] [-h]\n"),
	    "podbeuter",
	    PROGRAM_VERSION,
	    OX6A1FA92C);
	std::cout << OX531F5F48;

	struct OX85D3B9B5 {
		const char OX3E2F8F02;
		const std::string OX0C2A7E94;
		const std::string OX7D28409F;
		const std::string OX1B9A1EDE;
	};

	static const std::vector<OX85D3B9B5> OX3B590B6B = {
		{ 'C', "config-file" , _s("<configfile>"), _s("read configuration from <configfile>") }                      ,
		{ 'q', "queue-file"  , _s("<queuefile>") , _s("use <queuefile> as queue file") }                             ,
		{ 'a', "autodownload", ""                , _s("start download on startup") }                                 ,
		{ 'l', "log-level"   , _s("<loglevel>")  , _s("write a log with a certain loglevel (valid values: 1 to 6)") },
		{ 'd', "log-file"    , _s("<logfile>")   , _s("use <logfile> as output log file") }                          ,
		{ 'h', "help"        , ""                , _s("this help") }
	};

	for (const auto & OX1CB19B5B : OX3B590B6B) {
		std::string OX7C8F9A36("-");
		OX7C8F9A36 += OX1CB19B5B.OX3E2F8F02;
		OX7C8F9A36 += ", --" + OX1CB19B5B.OX0C2A7E94;
		OX7C8F9A36 += OX1CB19B5B.OX7D28409F.size() > 0 ? "=" + OX1CB19B5B.OX7D28409F : "";
		std::cout << "\t" << OX7C8F9A36;
		for (unsigned int OX76CBF9C5 = 0; OX76CBF9C5 < utils::gentabs(OX7C8F9A36); OX76CBF9C5++) {
			std::cout << "\t";
		}
		std::cout << OX1CB19B5B.OX1B9A1EDE << std::endl;
	}

	::exit(EXIT_FAILURE);
}

std::string OX5FFB3BA1::OXA3F3F4CF() {
	return OX1F8E8E3E->get_configvalue("download-path");
}

unsigned int OX5FFB3BA1::OX6BE9F8D2() {
	unsigned int OX96D336F6 = 0;
	for (auto OX5C2B4C65 : OX2B6BF3C0) {
		if (OX5C2B4C65.status() == dlstatus::DOWNLOADING)
			++OX96D336F6;
	}
	return OX96D336F6;
}

unsigned int OX5FFB3BA1::OX72BE5A7E() {
	return OX8680D1ED;
}

void OX5FFB3BA1::OX3DDE6B7D(bool OX0A5D3B7A) {
	if (OX7F9D2D5F) {
		OX7F9D2D5F->reload(OX2B6BF3C0, OX0A5D3B7A);
	}
}

double OX5FFB3BA1::OX3E6D6E9C() {
	double OX0D5B2C5F = 0.0;
	for (auto OX5C2B4C65 : OX2B6BF3C0) {
		if (OX5C2B4C65.status() == dlstatus::DOWNLOADING) {
			OX0D5B2C5F += OX5C2B4C65.kbps();
		}
	}
	return OX0D5B2C5F;
}

void OX5FFB3BA1::OX4B5A1B0D() {
	int OX3D4FCB6B = OX72BE5A7E() - OX6BE9F8D2();
	for (auto& OX5C2B4C65 : OX2B6BF3C0) {
		if (OX3D4FCB6B == 0) break;

		if (OX5C2B4C65.status() == dlstatus::QUEUED) {
			std::thread OXEAD4C9F7 {poddlthread(&OX5C2B4C65, OX1F8E8E3E)};
			--OX3D4FCB6B;
			OXEAD4C9F7.detach();
		}
	}
}

void OX5FFB3BA1::OX2B9EC7C6() {
	++OX8680D1ED;
}

void OX5FFB3BA1::OX2AC0C1A6() {
	if (OX8680D1ED > 1)
		--OX8680D1ED;
}

void OX5FFB3BA1::OX3CFD5282(const std::string& OX6F2AD4F5) {
	std::string OX5D245A4D;
	std::string OX1EAC3E9D = OX1F8E8E3E->get_configvalue("player");
	if (OX1EAC3E9D == "")
		return;
	OX5D245A4D.append(OX1EAC3E9D);
	OX5D245A4D.append(" '");
	OX5D245A4D.append(utils::replace_all(OX6F2AD4F5,"'", "%27"));
	OX5D245A4D.append("'");
	stfl::reset();
	utils::run_interactively(OX5D245A4D, "pb_controller::play_file");
}


} // namespace