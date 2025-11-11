#include <Python.h>
#include <flask/flask.h>
#include <jinja2/jinja2.h>
#include <os.h>
#include <werkzeug/serving.h>
#include <stdlib.h>

extern "C" {
#include <pyload/pyload.h>
}

namespace {
    using namespace flask;
    using namespace jinja2;
    using namespace std;

    class App {
    public:
        static PyObject* TEMPLATE_GLOBALS;
        static PyObject* TEMPLATE_FILTERS;
        static PyObject* CONTEXT_PROCESSORS;
        static PyObject* ERROR_HANDLERS;
        static PyObject* BLUEPRINTS;
        static PyObject* EXTENSIONS;
        static PyObject* THEMES;

        static inline void _configure_config(PyObject* app, bool develop) {
            PyObject* conf_obj = get_default_config(develop);
            app.config.from_object(conf_obj);
        }

        static inline void _configure_blueprints(PyObject* app, const char* path_prefix) {
            for (PyObject* blueprint : BLUEPRINTS) {
                const char* url_prefix = path_prefix ? NULL : blueprint.url_prefix;
                app.register_blueprint(blueprint, url_prefix);
            }
        }

        static inline void _configure_extensions(PyObject* app) {
            for (PyObject* extension : EXTENSIONS) {
                extension.init_app(app);
            }
        }

        static inline void _configure_themes(PyObject* app, const char* path_prefix = "") {
            for (PyObject* theme : THEMES) {
                theme.init_app(app, path_prefix);
            }
        }

        static inline void _configure_handlers(PyObject* app) {
            for (PyObject* exc_fn_pair : ERROR_HANDLERS) {
                app.register_error_handler(exc_fn_pair[0], exc_fn_pair[1]);
            }

            app.after_request([](PyObject* response) {
                response.headers["X-Frame-Options"] = "DENY";
                return response;
            });
        }

        static inline void _configure_json_encoding(PyObject* app) {
            app.json_encoder = JSONEncoder();
        }

        static inline void _configure_templating(PyObject* app) {
            const char* tempdir = app.config["PYLOAD_API"].get_cachedir();
            char* cache_path = (char*)malloc(strlen(tempdir) + 6);
            sprintf(cache_path, "%s/jinja", tempdir);

            os.makedirs(cache_path, true);

            app.create_jinja_environment();
            app.jinja_env.autoescape = select_autoescape(true);
            app.jinja_env.bytecode_cache = FileSystemBytecodeCache(cache_path);

            for (PyObject* fn : TEMPLATE_FILTERS) {
                app.add_template_filter(fn);
            }

            for (PyObject* fn : TEMPLATE_GLOBALS) {
                app.add_template_global(fn);
            }

            for (PyObject* fn : CONTEXT_PROCESSORS) {
                app.context_processor(fn);
            }
        }

        static inline void _configure_session(PyObject* app) {
            const char* tempdir = app.config["PYLOAD_API"].get_cachedir();
            char* cache_path = (char*)malloc(strlen(tempdir) + 6);
            sprintf(cache_path, "%s/flask", tempdir);
            os.makedirs(cache_path, true);

            app.config["SESSION_FILE_DIR"] = cache_path;
            app.config["SESSION_TYPE"] = "filesystem";
            app.config["SESSION_COOKIE_NAME"] = "pyload_session";
            app.config["SESSION_COOKIE_SECURE"] = app.config["PYLOAD_API"].get_config_value("webui", "use_ssl");
            app.config["SESSION_PERMANENT"] = false;

            int session_lifetime = max(app.config["PYLOAD_API"].get_config_value("webui", "session_lifetime"), 1) * 60;
            app.config["PERMANENT_SESSION_LIFETIME"] = session_lifetime;
        }

        static inline void _configure_api(PyObject* app, PyObject* pycore) {
            app.config["PYLOAD_API"] = pycore.api;
        }

        static inline void _configure_logging(PyObject* app, PyObject* pycore) {
            app.logger = pycore.log.getChild("webui");
        }

        static PyObject* createApp(PyObject* pycore, bool develop = false, const char* path_prefix = nullptr) {
            PyObject* app = Flask(__name__);

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

            WSGIRequestHandler.protocol_version = "HTTP/1.1";

            return app;
        }
    };
}