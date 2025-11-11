#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <Python.h>
}

#include <flask.h>
#include <jinja2.h>
#include <werkzeug/serving.h>

#include "blueprints.h"
#include "config.h"
#include "extensions.h"
#include "filters.h"
#include "globals.h"
#include "handlers.h"
#include "helpers.h"
#include "processors.h"

class App {
    static constexpr auto JINJA_TEMPLATE_GLOBALS = TEMPLATE_GLOBALS;
    static constexpr auto JINJA_TEMPLATE_FILTERS = TEMPLATE_FILTERS;
    static constexpr auto JINJA_CONTEXT_PROCESSORS = CONTEXT_PROCESSORS;
    static constexpr auto FLASK_ERROR_HANDLERS = ERROR_HANDLERS;
    static constexpr auto FLASK_BLUEPRINTS = BLUEPRINTS;
    static constexpr auto FLASK_EXTENSIONS = EXTENSIONS;
    static constexpr auto FLASK_THEMES = THEMES;

    public:
    static void _configure_config(flask::Flask* app, bool develop) {
        auto conf_obj = get_default_config(develop);
        app->config.from_object(conf_obj);
    }

    static void _configure_blueprints(flask::Flask* app, const char* path_prefix) {
        for (auto blueprint : FLASK_BLUEPRINTS) {
            const char* url_prefix = path_prefix ? path_prefix : nullptr;
            app->register_blueprint(blueprint, url_prefix);
        }
    }

    static void _configure_extensions(flask::Flask* app) {
        for (auto extension : FLASK_EXTENSIONS) {
            extension.init_app(app);
        }
    }

    static void _configure_themes(flask::Flask* app, const char* path_prefix = "") {
        for (auto theme : FLASK_THEMES) {
            theme.init_app(app, path_prefix);
        }
    }

    static void _configure_handlers(flask::Flask* app) {
        for (auto [exc, fn] : FLASK_ERROR_HANDLERS) {
            app->register_error_handler(exc, fn);
        }
    }

    static void _configure_json_encoding(flask::Flask* app) {
        app->json_encoder = JSONEncoder;
    }

    static void _configure_templating(flask::Flask* app) {
        auto tempdir = app->config["PYLOAD_API"].get_cachedir();
        auto cache_path = os.path.join(tempdir, "jinja");

        os.makedirs(cache_path, exist_ok=true);

        app->create_jinja_environment();

        app->jinja_env.autoescape = jinja2::select_autoescape(true);
        app->jinja_env.bytecode_cache = jinja2::FileSystemBytecodeCache(cache_path);

        for (auto fn : JINJA_TEMPLATE_FILTERS) {
            app->add_template_filter(fn);
        }

        for (auto fn : JINJA_TEMPLATE_GLOBALS) {
            app->add_template_global(fn);
        }

        for (auto fn : JINJA_CONTEXT_PROCESSORS) {
            app->context_processor(fn);
        }
    }

    static void _configure_session(flask::Flask* app) {
        auto tempdir = app->config["PYLOAD_API"].get_cachedir();
        auto cache_path = os.path.join(tempdir, "flask");
        os.makedirs(cache_path, exist_ok=true);

        app->config["SESSION_FILE_DIR"] = cache_path;
        app->config["SESSION_TYPE"] = "filesystem";
        app->config["SESSION_COOKIE_NAME"] = "pyload_session";
        app->config["SESSION_COOKIE_SECURE"] = app->config["PYLOAD_API"].get_config_value("webui", "use_ssl");
        app->config["SESSION_PERMANENT"] = false;

        auto session_lifetime = std::max(app->config["PYLOAD_API"].get_config_value("webui", "session_lifetime"), 1) * 60;
        app->config["PERMANENT_SESSION_LIFETIME"] = session_lifetime;
    }

    static void _configure_api(flask::Flask* app, pycore_t* pycore) {
        app->config["PYLOAD_API"] = pycore->api;
    }

    static void _configure_logging(flask::Flask* app, pycore_t* pycore) {
        app->logger = pycore->log.getChild("webui");
    }

    public:
    static flask::Flask* create(pycore_t* pycore, bool develop = false, const char* path_prefix = nullptr) {
        flask::Flask* app = new flask::Flask(__FILE__);

        _configure_logging(app, pycore);
        _configure_api(app, pycore);
        _configure_config(app, develop);
        _configure_templating(app);
        _configure_json_encoding(app);
        _configure_session(app);
        _configure_blueprints(app, path_prefix);
        _configure_extensions(app);
        _configure_themes(app, path_prefix ? path_prefix : "");
        _configure_handlers(app);

        WSGIRequestHandler::protocol_version = "HTTP/1.1";

        return app;
    }
};