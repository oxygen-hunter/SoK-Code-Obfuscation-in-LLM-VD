#include "socketlinereader.h"
#include <Python.h>
#include <iostream>

static PyObject* enqueue_packet(PyObject* self, PyObject* args) {
    const char* line;
    if (!PyArg_ParseTuple(args, "s", &line)) {
        return NULL;
    }
    std::cout << "Enqueuing packet: " << line << std::endl;
    Py_RETURN_NONE;
}

static PyMethodDef SocketLineReaderMethods[] = {
    {"enqueue_packet", enqueue_packet, METH_VARARGS, "Enqueue a packet"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef socketlinereadermodule = {
    PyModuleDef_HEAD_INIT,
    "socketlinereader",
    NULL,
    -1,
    SocketLineReaderMethods
};

PyMODINIT_FUNC PyInit_socketlinereader(void) {
    return PyModule_Create(&socketlinereadermodule);
}

SocketLineReader::SocketLineReader(QSslSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
{
    connect(m_socket, &QIODevice::readyRead,
            this, &SocketLineReader::dataReceived);
}

void SocketLineReader::dataReceived()
{
    Py_Initialize();
    PyImport_AppendInittab("socketlinereader", PyInit_socketlinereader);
    PyImport_ImportModule("socketlinereader");

    while (m_socket->canReadLine()) {
        const QByteArray line = m_socket->readLine();
        if (line.length() > 1) {
            PyObject* pName = PyUnicode_DecodeFSDefault("socketlinereader");
            PyObject* pModule = PyImport_Import(pName);
            Py_XDECREF(pName);

            if (pModule != NULL) {
                PyObject* pFunc = PyObject_GetAttrString(pModule, "enqueue_packet");
                if (PyCallable_Check(pFunc)) {
                    PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(line.constData()));
                    PyObject_CallObject(pFunc, pArgs);
                    Py_XDECREF(pArgs);
                }
                Py_XDECREF(pFunc);
                Py_XDECREF(pModule);
            }
        }
    }

    if (!m_packets.isEmpty()) {
        Q_EMIT readyRead();
    }

    Py_Finalize();
}