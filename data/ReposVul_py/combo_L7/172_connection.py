#include <Python.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <uuid/uuid.h>
#include <json/json.h>
#include <fstream>

using namespace std;

extern "C" {
    static PyObject* py_write_to_file_descriptor(PyObject* self, PyObject* args) {
        int fd;
        PyObject* obj;
        if (!PyArg_ParseTuple(args, "iO", &fd, &obj)) {
            return nullptr;
        }

        PyObject* pickle_module = PyImport_ImportModule("six.moves.cPickle");
        if (!pickle_module) {
            return nullptr;
        }
        PyObject* dumps_func = PyObject_GetAttrString(pickle_module, "dumps");
        if (!dumps_func) {
            Py_DECREF(pickle_module);
            return nullptr;
        }

        PyObject* pickle_args = Py_BuildValue("(Oi)", obj, 0);
        PyObject* pickled_obj = PyObject_CallObject(dumps_func, pickle_args);
        Py_DECREF(pickle_args);
        Py_DECREF(dumps_func);
        Py_DECREF(pickle_module);

        if (!pickled_obj) {
            return nullptr;
        }

        char* src;
        Py_ssize_t src_len;
        if (PyBytes_AsStringAndSize(pickled_obj, &src, &src_len) == -1) {
            Py_DECREF(pickled_obj);
            return nullptr;
        }

        char hash[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)src, src_len, (unsigned char*)hash);

        dprintf(fd, "%ld\n", src_len);
        write(fd, src, src_len);
        dprintf(fd, "%s\n", hash);

        Py_DECREF(pickled_obj);
        Py_RETURN_NONE;
    }
}

string uuid4() {
    uuid_t uuid;
    char uuid_str[37];
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    return string(uuid_str);
}

class ConnectionError : public exception {
public:
    string message;
    int code;
    ConnectionError(const string& msg, int c = 1) : message(msg), code(c) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class Connection {
private:
    string socket_path;

public:
    Connection(const string& path) {
        if (path.empty()) {
            throw runtime_error("socket_path must be a value");
        }
        socket_path = path;
    }

    string send(const string& data) {
        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            throw ConnectionError("Socket creation error");
        }

        sockaddr_un server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sun_family = AF_UNIX;
        strncpy(server_addr.sun_path, socket_path.c_str(), sizeof(server_addr.sun_path) - 1);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            throw ConnectionError("Unable to connect to socket " + socket_path);
        }

        uint64_t data_len = data.size();
        uint64_t len_net = htobe64(data_len);
        if (send(sock, &len_net, sizeof(len_net), 0) != sizeof(len_net)) {
            close(sock);
            throw ConnectionError("Failed to send data length");
        }

        if (send(sock, data.c_str(), data_len, 0) != data_len) {
            close(sock);
            throw ConnectionError("Failed to send data");
        }

        char buffer[1024];
        ssize_t received = recv(sock, buffer, sizeof(buffer), 0);
        close(sock);

        if (received < 0) {
            throw ConnectionError("Failed to receive data");
        }

        return string(buffer, received);
    }
};

extern "C" {
    static PyObject* py_exec_command(PyObject* self, PyObject* args) {
        PyObject* module;
        const char* command;
        if (!PyArg_ParseTuple(args, "Os", &module, &command)) {
            return nullptr;
        }

        PyObject* socket_path_obj = PyObject_GetAttrString(module, "_socket_path");
        if (!socket_path_obj) {
            return nullptr;
        }

        const char* socket_path = PyUnicode_AsUTF8(socket_path_obj);
        Py_DECREF(socket_path_obj);

        try {
            Connection conn(socket_path);
            string response = conn.send(command);
            return Py_BuildValue("(is)", 0, response.c_str());
        } catch (const ConnectionError& e) {
            return Py_BuildValue("(is)", e.code, e.what());
        }
    }
}

static PyMethodDef Methods[] = {
    {"write_to_file_descriptor", py_write_to_file_descriptor, METH_VARARGS, "Write data to file descriptor"},
    {"exec_command", py_exec_command, METH_VARARGS, "Execute a command"},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "connection",
    "Connection Module",
    -1,
    Methods
};

PyMODINIT_FUNC PyInit_connection(void) {
    return PyModule_Create(&moduledef);
}