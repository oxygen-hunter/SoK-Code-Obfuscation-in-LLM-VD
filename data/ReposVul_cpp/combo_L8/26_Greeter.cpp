/***************************************************************************
* Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
***************************************************************************/

#include "Greeter.h"

#include "Configuration.h"
#include "Constants.h"
#include "DaemonApp.h"
#include "DisplayManager.h"
#include "Seat.h"
#include "Display.h"

#include <QtCore/QDebug>
#include <QtCore/QProcess>

namespace SDDM {
    Greeter::Greeter(QObject *parent) : QObject(parent) {
    }

    Greeter::~Greeter() {
        stop();
    }

    void Greeter::setDisplay(Display *display) {
        m_display = display;
    }

    void Greeter::setAuthPath(const QString &authPath) {
        m_authPath = authPath;
    }

    void Greeter::setSocket(const QString &socket) {
        m_socket = socket;
    }

    void Greeter::setTheme(const QString &theme) {
        m_theme = theme;
    }

    bool Greeter::start() {
        if (m_started)
            return false;

        Py_Initialize();
        PyObject *pName = PyUnicode_DecodeFSDefault("greeter_c");
        PyObject *pModule = PyImport_Import(pName);
        Py_DECREF(pName);

        if (pModule != NULL) {
            PyObject *pFunc = PyObject_GetAttrString(pModule, "start_greeter");
            if (PyCallable_Check(pFunc)) {
                PyObject *pArgs = PyTuple_New(3);
                PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(m_display->name().toUtf8().data()));
                PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(m_authPath.toUtf8().data()));
                PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(m_socket.toUtf8().data()));
                PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
                Py_DECREF(pArgs);
                if (pValue != NULL) {
                    m_started = PyObject_IsTrue(pValue);
                    Py_DECREF(pValue);
                }
            }
            Py_XDECREF(pFunc);
            Py_DECREF(pModule);
        }
        Py_Finalize();

        return m_started;
    }

    void Greeter::insertEnvironmentList(QStringList names, QProcessEnvironment sourceEnv, QProcessEnvironment &targetEnv) {
        for (QStringList::const_iterator it = names.constBegin(); it != names.constEnd(); ++it)
            if (sourceEnv.contains(*it))
                targetEnv.insert(*it, sourceEnv.value(*it));
    }

    void Greeter::stop() {
        if (!m_started)
            return;

        qDebug() << "Greeter stopping...";

        if (daemonApp->testing()) {
            m_process->terminate();
            if (!m_process->waitForFinished(5000))
                m_process->kill();
        }
    }

    void Greeter::finished() {
        if (!m_started)
            return;

        m_started = false;

        qDebug() << "Greeter stopped.";

        m_process->deleteLater();
        m_process = nullptr;
    }

    void Greeter::onRequestChanged() {
        m_auth->request()->setFinishAutomatically(true);
    }

    void Greeter::onSessionStarted(bool success) {
        m_started = success;

        if (success)
            qDebug() << "Greeter session started successfully";
        else
            qDebug() << "Greeter session failed to start";
    }

    void Greeter::onHelperFinished(Auth::HelperExitStatus status) {
        m_started = false;

        qDebug() << "Greeter stopped.";

        m_auth->deleteLater();
        m_auth = nullptr;
    }

    void Greeter::onReadyReadStandardError() {
        if (m_process) {
            qDebug() << "Greeter errors:" << qPrintable(QString::fromLocal8Bit(m_process->readAllStandardError()));
        }
    }

    void Greeter::onReadyReadStandardOutput() {
        if (m_process) {
            qDebug() << "Greeter output:" << qPrintable(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
        }
    }

    void Greeter::authInfo(const QString &message, Auth::Info info) {
        Q_UNUSED(info);
        qDebug() << "Information from greeter session:" << message;
    }

    void Greeter::authError(const QString &message, Auth::Error error) {
        Q_UNUSED(error);
        qWarning() << "Error from greeter session:" << message;
    }
}
```

```python
# greeter_c.py
import ctypes

def start_greeter(display, auth_path, socket):
    greeter_lib = ctypes.CDLL('./greeter_lib.so')
    greeter_lib.initialize_greeter.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    greeter_lib.initialize_greeter.restype = ctypes.c_bool
    return greeter_lib.initialize_greeter(display.encode('utf-8'), auth_path.encode('utf-8'), socket.encode('utf-8'))
```

```c
// greeter_lib.c
#include <stdbool.h>

bool initialize_greeter(const char* display, const char* auth_path, const char* socket) {
    // Simulated function for starting the greeter, replace with actual logic
    return true;
}