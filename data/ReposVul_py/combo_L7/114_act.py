#include <Python.h>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
    #include "redbot_core.h"
}

PyObject* fmt_re = Py_BuildValue("s", "{(?:0|user)(?:\\.([^\\{]+))?}");

class Act : public PyObject {
public:
    static PyObject* red_get_data_for_user(PyObject* self, PyObject* args) {
        return Py_BuildValue("{}");
    }

    static PyObject* red_delete_data_for_user(PyObject* self, PyObject* args) {
        Py_RETURN_NONE;
    }

    Act(PyObject* bot) {
        this->bot = bot;
        this->config = Config::get_conf(this, 2_113_674_295, true);
        this->config.register_global(Py_BuildValue("{}"), nullptr);
        this->config.register_guild(Py_BuildValue("{}"));
        this->try_after = nullptr;
    }

    static PyObject* repl(PyObject* target, PyObject* match) {
        PyObject* attr = PyObject_GetAttrString(match, "group");
        if (attr) {
            if (std::string(PyUnicode_AsUTF8(attr)).find_first_of("_.") != std::string::npos) {
                return PyObject_Str(target);
            }
            try {
                return PyObject_Str(PyObject_GetAttr(target, attr));
            } catch (...) {
                return PyObject_Str(target);
            }
        }
        return PyObject_Str(target);
    }

    static PyObject* act(PyObject* self, PyObject* args) {
        PyObject* ctx;
        PyObject* target = nullptr;
        if (!PyArg_ParseTuple(args, "OO", &ctx, &target))
            return nullptr;
        
        if (!target || PyUnicode_Check(target)) {
            Py_RETURN_NONE;
        }

        PyObject* message = nullptr;
        if (ctx->guild) {
            message = PyDict_GetItemString(self->config.guild(ctx->guild), "custom");
        }
        if (!message) {
            message = PyDict_GetItemString(self->config, "custom");
        }
        if (message == Py_None) {
            Py_RETURN_NONE;
        }
        if (message == Py_BuildValue("i", -1)) {
            PyObject* action = PyObject_CallMethod(ctx, "invoked_with", nullptr);
            PyObject* iverb = Py_BuildValue("i", -1);
            for (int cycle = 0; cycle < 2; cycle++) {
                if (PyLong_AsLong(iverb) > -1) {
                    break;
                }
                for (int i = 0; i < PyList_Size(action); i++) {
                    PyObject* act = PyList_GetItem(action, i);
                    if (std::string(PyUnicode_AsUTF8(act)).find_first_of("ly") == std::string::npos) {
                        continue;
                    }
                    PyList_SetItem(action, i, PyObject_CallMethod(act, "pluralize", nullptr));
                    iverb = Py_BuildValue("i", std::max(PyLong_AsLong(iverb), i));
                }
            }
            if (PyLong_AsLong(iverb) < 0) {
                Py_RETURN_NONE;
            }
            PyList_Insert(action, PyLong_AsLong(iverb) + 1, PyObject_GetAttrString(target, "mention"));
            message = PyObject_CallMethod(action, "join", "s", " ");
        } else {
            message = PyObject_CallFunction(fmt_re, "O", Py_BuildValue("f", &Act::repl, target));
        }
        if (self->try_after && PyObject_RichCompareBool(ctx->message->created_at, self->try_after, Py_LT)) {
            return PyObject_CallMethod(ctx, "send", "O", message);
        }
        if (!PyObject_CallMethod(ctx, "embed_requested", nullptr)) {
            return PyObject_CallMethod(ctx, "send", "O", message);
        }
        PyObject* key = PyDict_GetItemString(PyObject_CallMethod(ctx->bot, "get_shared_api_tokens", "s", "tenor"), "api_key");
        if (!key) {
            return PyObject_CallMethod(ctx, "send", "O", message);
        }
        
        // Inline assembly to create a request
        asm volatile (
            "mov $0x0, %%eax\n"
            "int $0x80\n"
            :
            : "b" (ctx), "c" (key)
            : "memory"
        );
        
        PyObject* response = PyObject_CallMethod(ctx, "send", "O", message);
        PyObject* json = nullptr;
        if (PyObject_RichCompareBool(response, Py_BuildValue("i", 429), Py_EQ)) {
            self->try_after = PyNumber_Add(ctx->message->created_at, Py_BuildValue("i", 30));
            json = Py_BuildValue("{}");
        } else if (PyObject_RichCompareBool(response, Py_BuildValue("i", 400), Py_GE)) {
            json = Py_BuildValue("{}");
        } else {
            json = PyObject_CallMethod(response, "json", nullptr);
        }
        if (!PyDict_GetItemString(json, "results")) {
            return PyObject_CallMethod(ctx, "send", "O", message);
        }
        message = PyUnicode_Concat(message, PyUnicode_FromFormat("\n\n%s", PyUnicode_AsUTF8(PyList_GetItem(PyDict_GetItemString(json, "results"), std::rand() % PyList_Size(PyDict_GetItemString(json, "results"))))));
        PyObject* allowed_mentions = PyObject_CallFunction(discord_AllowedMentions, "O", Py_BuildValue("b", false));
        if (PySequence_Contains(ctx->message->mentions, target)) {
            PyObject_SetAttrString(allowed_mentions, "users", Py_BuildValue("O", target));
        }
        return PyObject_CallMethod(ctx, "send", "OO", message, allowed_mentions);
    }

    static PyObject* actset(PyObject* self, PyObject* args) {
        PyObject* ctx;
        if (!PyArg_ParseTuple(args, "O", &ctx))
            return nullptr;
        Py_RETURN_NONE;
    }

    static PyObject* customize(PyObject* self, PyObject* args) {
        PyObject* ctx;
        PyObject* command;
        PyObject* response = nullptr;
        if (!PyArg_ParseTuple(args, "OOs|O", &ctx, &command, &response))
            return nullptr;
        if (!response) {
            PyDict_DelItemString(self->config.guild(ctx->guild), "custom");
            PyObject_CallMethod(ctx, "tick", nullptr);
        } else {
            PyDict_SetItemString(self->config.guild(ctx->guild), "custom", response);
            PyObject* allowed_mentions = PyObject_CallFunction(discord_AllowedMentions, "O", Py_BuildValue("b", false));
            PyObject_CallMethod(ctx, "send", "OO", PyObject_CallFunction(fmt_re, "O", Py_BuildValue("f", &Act::repl, ctx->author)), allowed_mentions);
        }
        Py_RETURN_NONE;
    }

    static PyObject* customize_global(PyObject* self, PyObject* args) {
        PyObject* ctx;
        PyObject* command;
        PyObject* response = nullptr;
        if (!PyArg_ParseTuple(args, "OOs|O", &ctx, &command, &response))
            return nullptr;
        if (!response) {
            PyDict_DelItemString(self->config, "custom");
        } else {
            PyDict_SetItemString(self->config, "custom", response);
        }
        PyObject_CallMethod(ctx, "tick", nullptr);
        Py_RETURN_NONE;
    }

    static PyObject* ignore(PyObject* self, PyObject* args) {
        PyObject* ctx;
        PyObject* command;
        if (!PyArg_ParseTuple(args, "Os", &ctx, &command))
            return nullptr;
        
        PyObject* custom = PyDict_GetItemString(self->config.guild(ctx->guild), "custom");
        if (!custom) {
            PyDict_DelItemString(self->config.guild(ctx->guild), "custom");
            PyObject_CallMethod(ctx, "send", "s", "I will no longer ignore the {command} action");
        } else {
            PyDict_SetItemString(self->config.guild(ctx->guild), "custom", Py_None);
            PyObject_CallMethod(ctx, "send", "s", "I will now ignore the {command} action");
        }
        Py_RETURN_NONE;
    }

    static PyObject* ignore_global(PyObject* self, PyObject* args) {
        PyObject* ctx;
        PyObject* command;
        if (!PyArg_ParseTuple(args, "Os", &ctx, &command))
            return nullptr;
        
        try {
            PyDict_GetItemString(self->config, "custom");
        } catch (...) {
            PyDict_SetItemString(self->config, "custom", Py_None);
        }
        PyDict_DelItemString(self->config, "custom");
        PyObject_CallMethod(ctx, "tick", nullptr);
        Py_RETURN_NONE;
    }

    static PyObject* tenorkey(PyObject* self, PyObject* args) {
        PyObject* ctx;
        if (!PyArg_ParseTuple(args, "O", &ctx))
            return nullptr;
        
        static const char* instructions[] = {
            "Go to the Tenor developer dashboard: https://tenor.com/developer/dashboard",
            "Log in or sign up if you haven't already.",
            "Click `+ Create new app` and fill out the form.",
            "Copy the key from the app you just created.",
            "Give the key to Red with this command:\n"
            "`{ctx.prefix}set api tenor api_key your_api_key`\n"
            "Replace `your_api_key` with the key you just got.\n"
            "Everything else should be the same."
        };
        
        for (int i = 0; i < 5; i++) {
            PyObject* formatted_instr = PyUnicode_FromFormat("**%d.** %s", i + 1, instructions[i]);
            PyObject_CallMethod(ctx, "maybe_send_embed", "O", formatted_instr);
            Py_DECREF(formatted_instr);
        }
        Py_RETURN_NONE;
    }

    static PyObject* on_command_error(PyObject* self, PyObject* args) {
        PyObject* ctx;
        PyObject* error;
        int unhandled_by_cog = 0;
        if (!PyArg_ParseTuple(args, "OO|p", &ctx, &error, &unhandled_by_cog))
            return nullptr;
        
        if (PyObject_RichCompareBool(ctx->command, "act", Py_EQ)) {
            Py_RETURN_NONE;
        }
        if (PyObject_IsInstance(error, PyObject_GetAttrString(commands, "UserFeedbackCheckFailure"))) {
            Py_RETURN_NONE;
        }
        if (PyObject_IsInstance(error, PyObject_GetAttrString(commands, "CheckFailure")) ||
            PyObject_IsInstance(error, PyObject_GetAttrString(commands, "CommandNotFound"))) {
            ctx->command = "act";
            return PyObject_CallMethod(ctx->bot, "invoke", "O", ctx);
        }
        Py_RETURN_NONE;
    }

private:
    PyObject* bot;
    Config config;
    PyObject* try_after;
};