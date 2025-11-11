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

static std::string lf = "pb-lock.pid";

static void cca(int s) {
	LOG(LOG_DEBUG,"caugh signal %d",s);
	stfl::reset();
	utils::remove_fs_lock(lf);
	::exit(EXIT_FAILURE);
}

namespace podbeuter {

#define LS ".lock"

bool pb_controller::sxdg(const char *eh) {
	const char *exh;
	const char *exd;
	std::string xc, xd;

	exh = ::getenv("XDG_CONFIG_HOME");
	if (exh) {
		xc = exh;
	} else {
		xc = eh;
		xc.append(NEWSBEUTER_PATH_SEP);
		xc.append(".config");
	}

	exd = ::getenv("XDG_DATA_HOME");
	if (exd) {
		xd = exd;
	} else {
		xd = eh;
		xd.append(NEWSBEUTER_PATH_SEP);
		xd.append(".local");
		xd.append(NEWSBEUTER_PATH_SEP);
		xd.append("share");
	}

	xc.append(NEWSBEUTER_PATH_SEP);
	xc.append(NEWSBEUTER_SUBDIR_XDG);

	xd.append(NEWSBEUTER_PATH_SEP);
	xd.append(NEWSBEUTER_SUBDIR_XDG);

	if (access(xc.c_str(), R_OK | X_OK) != 0) {
		std::cout << utils::strprintf(_("XDG: configuration directory '%s' not accessible, using '%s' instead."), xc.c_str(), cd.c_str()) << std::endl;
		return false;
	}
	if (access(xd.c_str(), R_OK | X_OK | W_OK) != 0) {
		std::cout << utils::strprintf(_("XDG: data directory '%s' not accessible, using '%s' instead."), xd.c_str(), cd.c_str()) << std::endl;
		return false;
	}

	cd = xc;

	url_file = cd + std::string(NEWSBEUTER_PATH_SEP) + url_file;
	cf = cd + std::string(NEWSBEUTER_PATH_SEP) + cf;

	cache_file = xd + std::string(NEWSBEUTER_PATH_SEP) + cache_file;
	lf = cache_file + LS;
	qf = xd + std::string(NEWSBEUTER_PATH_SEP) + qf;
	searchfile = utils::strprintf("%s%shistory.search", xd.c_str(), NEWSBEUTER_PATH_SEP);
	cmdlinefile = utils::strprintf("%s%shistory.cmdline", xd.c_str(), NEWSBEUTER_PATH_SEP);

	return true;
}

pb_controller::pb_controller() : v(0), cf("config"), qf("queue"), cfg(0), view_update_(true),  m_dls(1), ql(0) {
	char* c;
	if (!(c = ::getenv("HOME"))) {
		struct passwd * spw = ::getpwuid(::getuid());
		if (spw) {
			c = spw->pw_dir;
		} else {
			std::cout << _("Fatal error: couldn't determine home directory!") << std::endl;
			std::cout << utils::strprintf(_("Please set the HOME environment variable or add a valid user for UID %u!"), ::getuid()) << std::endl;
			::exit(EXIT_FAILURE);
		}
	}
	cd = c;

	if (sxdg(c))
		return;

	cd.append(NEWSBEUTER_PATH_SEP);
	cd.append(NEWSBEUTER_CONFIG_SUBDIR);
	::mkdir(cd.c_str(),0700);

	cf = cd + std::string(NEWSBEUTER_PATH_SEP) + cf;
	qf = cd + std::string(NEWSBEUTER_PATH_SEP) + qf;
	lf = cd + std::string(NEWSBEUTER_PATH_SEP) + lf;
}

pb_controller::~pb_controller() {
	delete cfg;
}

void pb_controller::run(int c, char * av[]) {
	int o;
	bool a_dl = false;

	::signal(SIGINT, cca);

	do {
		if ((o = ::getopt(c, av, "C:q:d:l:ha")) < 0)
			continue;

		switch (o) {
		case ':':
		case '?':
			usage(av[0]);
			break;
		case 'C':
			cf = optarg;
			break;
		case 'q':
			qf = optarg;
			break;
		case 'a':
			a_dl = true;
			break;
		case 'd':
			logger::getInstance().set_logfile(optarg);
			break;
		case 'l': {
			loglevel ll = static_cast<loglevel>(atoi(optarg));
			if (ll > LOG_NONE && ll <= LOG_DEBUG)
				logger::getInstance().set_loglevel(ll);
		}
		break;
		case 'h':
			usage(av[0]);
			break;
		default:
			std::cout << utils::strprintf(_("%s: unknown option - %c"), av[0], static_cast<char>(o)) << std::endl;
			usage(av[0]);
			break;
		}
	} while (o != -1);

	std::cout << utils::strprintf(_("Starting %s %s..."), "podbeuter", PROGRAM_VERSION) << std::endl;

	pid_t pid;
	if (!utils::try_fs_lock(lf, pid)) {
		std::cout << utils::strprintf(_("Error: an instance of %s is already running (PID: %u)"), "podbeuter", pid) << std::endl;
		return;
	}

	std::cout << _("Loading configuration...");
	std::cout.flush();

	configparser cp;
	cfg = new configcontainer();
	cfg->register_commands(cp);
	colormanager * cm = new colormanager();
	cm->register_commands(cp);

	keymap k(KM_PODBEUTER);
	cp.register_handler("bind-key", &k);
	cp.register_handler("unbind-key", &k);

	null_config_action_handler n_cah;
	cp.register_handler("macro", &n_cah);
	cp.register_handler("ignore-article", &n_cah);
	cp.register_handler("always-download", &n_cah);
	cp.register_handler("define-filter", &n_cah);
	cp.register_handler("highlight", &n_cah);
	cp.register_handler("highlight-article", &n_cah);
	cp.register_handler("reset-unread-on-update", &n_cah);

	try {
		cp.parse("/etc/newsbeuter/config");
		cp.parse(cf);
	} catch (const configexception& ex) {
		std::cout << ex.what() << std::endl;
		delete cm;
		return;
	}

	if (cm->colors_loaded())
		cm->set_pb_colors(v);
	delete cm;

	m_dls = cfg->get_configvalue_as_int("max-downloads");

	std::cout << _("done.") << std::endl;

	ql = new queueloader(qf, this);
	ql->reload(d_);

	v->set_keymap(&k);

	v->run(a_dl);

	stfl::reset();

	std::cout <<  _("Cleaning up queue...");
	std::cout.flush();

	ql->reload(d_);
	delete ql;

	std::cout << _("done.") << std::endl;

	utils::remove_fs_lock(lf);
}

void pb_controller::usage(const char * av0) {
	std::cout << utils::strprintf(_("%s %s\nusage %s [-C <file>] [-q <file>] [-h]\n"
	                                "-C <configfile> read configuration from <configfile>\n"
	                                "-q <queuefile>  use <queuefile> as queue file\n"
	                                "-a              start download on startup\n"
	                                "-h              this help\n"), "podbeuter", PROGRAM_VERSION, av0);
	::exit(EXIT_FAILURE);
}

std::string pb_controller::get_dlpath() {
	return cfg->get_configvalue("download-path");
}

unsigned int pb_controller::downloads_in_progress() {
	unsigned int c = 0;
	for (auto d : d_) {
		if (d.status() == DL_DOWNLOADING)
			++c;
	}
	return c;
}

unsigned int pb_controller::get_maxdownloads() {
	return m_dls;
}

void pb_controller::reload_queue(bool r) {
	if (ql) {
		ql->reload(d_, r);
	}
}

double pb_controller::get_total_kbps() {
	double r = 0.0;
	for (auto d : d_) {
		if (d.status() == DL_DOWNLOADING) {
			r += d.kbps();
		}
	}
	return r;
}

void pb_controller::start_downloads() {
	int d2s = get_maxdownloads() - downloads_in_progress();
	for (auto i=d_.begin(); d2s > 0 && i!=d_.end(); ++i) {
		if (i->status() == DL_QUEUED) {
			std::thread t {poddlthread(&(*i), cfg)};
			--d2s;
		}
	}
}

void pb_controller::increase_parallel_downloads() {
	++m_dls;
}

void pb_controller::decrease_parallel_downloads() {
	if (m_dls > 1)
		--m_dls;
}

void pb_controller::play_file(const std::string& f) {
	std::string cl;
	std::string p = cfg->get_configvalue("player");
	if (p == "")
		return;
	cl.append(p);
	cl.append(" \'");
	cl.append(utils::replace_all(f,"'", "%27"));
	cl.append("\'");
	stfl::reset();
	LOG(LOG_DEBUG, "pb_controller::play_file: running `%s'", cl.c_str());
	::system(cl.c_str());
}


} // namespace