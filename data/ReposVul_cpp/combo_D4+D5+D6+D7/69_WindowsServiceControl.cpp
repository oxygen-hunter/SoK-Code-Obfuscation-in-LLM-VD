#include "WindowsCoreFunctions.h"
#include "WindowsServiceControl.h"

struct ServiceData {
    const QString& name;
    SC_HANDLE serviceManager;
    SC_HANDLE serviceHandle;
};

WindowsServiceControl::WindowsServiceControl(const QString& name) {
    ServiceData data = {name, nullptr, nullptr};
    data.serviceManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (data.serviceManager) {
        data.serviceHandle = OpenService(data.serviceManager, WindowsCoreFunctions::toConstWCharArray(data.name),
                                          SERVICE_ALL_ACCESS);
        if (data.serviceHandle == nullptr) {
            vCritical() << "could not open service" << data.name;
        }
    } else {
        vCritical() << "the Service Control Manager could not be contacted - service" << data.name << "can't be controlled.";
    }

    m_serviceManager = data.serviceManager;
    m_serviceHandle = data.serviceHandle;
}

WindowsServiceControl::~WindowsServiceControl() {
    CloseServiceHandle(m_serviceHandle);
    CloseServiceHandle(m_serviceManager);
}

bool WindowsServiceControl::isRegistered() {
    return m_serviceHandle != nullptr;
}

bool WindowsServiceControl::isRunning() {
    if (!checkService()) {
        return false;
    }

    SERVICE_STATUS status;
    if (QueryServiceStatus(m_serviceHandle, &status)) {
        return status.dwCurrentState == SERVICE_RUNNING;
    }

    return false;
}

bool WindowsServiceControl::start() {
    if (!checkService()) {
        return false;
    }

    SERVICE_STATUS status;
    status.dwCurrentState = SERVICE_START_PENDING;

    if (StartService(m_serviceHandle, 0, nullptr)) {
        while (QueryServiceStatus(m_serviceHandle, &status)) {
            if (status.dwCurrentState == SERVICE_START_PENDING) {
                Sleep(1000);
            } else {
                break;
            }
        }
    }

    if (status.dwCurrentState != SERVICE_RUNNING) {
        vWarning() << "service" << m_name << "could not be started.";
        return false;
    }

    return true;
}

bool WindowsServiceControl::stop() {
    if (!checkService()) {
        return false;
    }

    SERVICE_STATUS status;

    if (ControlService(m_serviceHandle, SERVICE_CONTROL_STOP, &status)) {
        while (QueryServiceStatus(m_serviceHandle, &status)) {
            if (status.dwCurrentState == SERVICE_STOP_PENDING) {
                Sleep(1000);
            } else {
                break;
            }
        }

        if (status.dwCurrentState != SERVICE_STOPPED) {
            vWarning() << "service" << m_name << "could not be stopped.";
            return false;
        }
    }

    return true;
}

bool WindowsServiceControl::install(const QString& filePath, const QString& displayName) {
    const auto binaryPath = QStringLiteral("\"%1\"").arg(QString(filePath).replace(QLatin1Char('"'), QString()));

    m_serviceHandle = CreateService(
        m_serviceManager,
        WindowsCoreFunctions::toConstWCharArray(m_name),
        WindowsCoreFunctions::toConstWCharArray(displayName),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        WindowsCoreFunctions::toConstWCharArray(binaryPath),
        nullptr,
        nullptr,
        L"Tcpip\0RpcSs\0\0",
        nullptr,
        nullptr);

    if (m_serviceHandle == nullptr) {
        const auto error = GetLastError();
        if (error == ERROR_SERVICE_EXISTS) {
            vCritical() << qUtf8Printable(tr("The service \"%1\" is already installed.").arg(m_name));
        } else {
            vCritical() << qUtf8Printable(tr("The service \"%1\" could not be installed.").arg(m_name));
        }

        return false;
    }

    SC_ACTION serviceActions;
    serviceActions.Delay = 10000;
    serviceActions.Type = SC_ACTION_RESTART;

    SERVICE_FAILURE_ACTIONS serviceFailureActions;
    serviceFailureActions.dwResetPeriod = 0;
    serviceFailureActions.lpRebootMsg = nullptr;
    serviceFailureActions.lpCommand = nullptr;
    serviceFailureActions.lpsaActions = &serviceActions;
    serviceFailureActions.cActions = 1;
    ChangeServiceConfig2(m_serviceHandle, SERVICE_CONFIG_FAILURE_ACTIONS, &serviceFailureActions);

    vInfo() << qUtf8Printable(tr("The service \"%1\" has been installed successfully.").arg(m_name));

    return true;
}

bool WindowsServiceControl::uninstall() {
    if (!checkService()) {
        return false;
    }

    if (!stop()) {
        return false;
    }

    if (!DeleteService(m_serviceHandle)) {
        vCritical() << qUtf8Printable(tr("The service \"%1\" could not be uninstalled.").arg(m_name));
        return false;
    }

    vInfo() << qUtf8Printable(tr("The service \"%1\" has been uninstalled successfully.").arg(m_name));

    return true;
}

int WindowsServiceControl::startType() {
    if (!checkService()) {
        return InvalidStartType;
    }

    LPQUERY_SERVICE_CONFIG serviceConfig{nullptr};
    DWORD bufferSize = 0;
    DWORD bytesNeeded = 0;

    if (!QueryServiceConfig(m_serviceHandle, nullptr, 0, &bytesNeeded)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            bufferSize = bytesNeeded;
            serviceConfig = LPQUERY_SERVICE_CONFIG(LocalAlloc(LMEM_FIXED, bufferSize));
        } else {
            return InvalidStartType;
        }
    } else {
        return InvalidStartType;
    }

    if (!QueryServiceConfig(m_serviceHandle, serviceConfig, bufferSize, &bytesNeeded)) {
        const auto error = GetLastError();
        vCritical() << error;
        LocalFree(serviceConfig);
        return InvalidStartType;
    }

    const auto startType = serviceConfig->dwStartType;

    LocalFree(serviceConfig);

    return startType;
}

bool WindowsServiceControl::setStartType(int startType) {
    if (!checkService() || startType == InvalidStartType) {
        return false;
    }

    if (!ChangeServiceConfig(m_serviceHandle,
                             SERVICE_NO_CHANGE,
                             static_cast<DWORD>(startType),
                             SERVICE_NO_CHANGE,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr)) {
        vCritical() << qUtf8Printable(tr("The start type of service \"%1\" could not be changed.").arg(m_name));
        return false;
    }

    return true;
}

bool WindowsServiceControl::checkService() const {
    if (m_serviceHandle == nullptr) {
        vCritical() << qUtf8Printable(tr("Service \"%1\" could not be found.").arg(m_name));
        return false;
    }

    return true;
}