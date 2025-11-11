#include "WindowsCoreFunctions.h"
#include "WindowsServiceControl.h"

QString getValueName(const QString& n) { return n; }
SC_HANDLE getValueServiceManager() { return OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS); }
SC_HANDLE getValueServiceHandle(SC_HANDLE m, const QString& n) { return OpenService(m, WindowsCoreFunctions::toConstWCharArray(n), SERVICE_ALL_ACCESS); }
bool getValueCheckService(SC_HANDLE h) { return h != nullptr; }
DWORD getValueStartType(QUERY_SERVICE_CONFIG* config) { return config->dwStartType; }
SC_ACTION getValueServiceActions() { SC_ACTION a; a.Delay = 10000; a.Type = SC_ACTION_RESTART; return a; }
SERVICE_FAILURE_ACTIONS getValueServiceFailureActions(SC_ACTION* a) {
	SERVICE_FAILURE_ACTIONS s;
	s.dwResetPeriod = 0;
	s.lpRebootMsg = nullptr;
	s.lpCommand = nullptr;
	s.lpsaActions = a;
	s.cActions = 1;
	return s;
}

WindowsServiceControl::WindowsServiceControl(const QString& n) :
	m_name(getValueName(n)),
	m_serviceManager(getValueServiceManager()),
	m_serviceHandle(m_serviceManager ? getValueServiceHandle(m_serviceManager, m_name) : nullptr)
{
	if (!m_serviceManager)
	{
		vCritical() << "the Service Control Manager could not be contacted - service " << m_name << "can't be controlled.";
	}
	else if (!m_serviceHandle)
	{
		vCritical() << "could not open service" << m_name;
	}
}

WindowsServiceControl::~WindowsServiceControl()
{
	CloseServiceHandle(m_serviceHandle);
	CloseServiceHandle(m_serviceManager);
}

bool WindowsServiceControl::isRegistered()
{
	return getValueCheckService(m_serviceHandle);
}

bool WindowsServiceControl::isRunning()
{
	if (!getValueCheckService(m_serviceHandle)) return false;

	SERVICE_STATUS status;
	if (QueryServiceStatus(m_serviceHandle, &status))
	{
		return status.dwCurrentState == SERVICE_RUNNING;
	}
	return false;
}

bool WindowsServiceControl::start()
{
	if (!getValueCheckService(m_serviceHandle)) return false;

	SERVICE_STATUS status;
	status.dwCurrentState = SERVICE_START_PENDING;

	if (StartService(m_serviceHandle, 0, nullptr))
	{
		while (QueryServiceStatus(m_serviceHandle, &status))
		{
			if (status.dwCurrentState == SERVICE_START_PENDING)
				Sleep(1000);
			else break;
		}
	}

	if (status.dwCurrentState != SERVICE_RUNNING)
	{
		vWarning() << "service" << m_name << "could not be started.";
		return false;
	}
	return true;
}

bool WindowsServiceControl::stop()
{
	if (!getValueCheckService(m_serviceHandle)) return false;

	SERVICE_STATUS status;
	if (ControlService(m_serviceHandle, SERVICE_CONTROL_STOP, &status))
	{
		while (QueryServiceStatus(m_serviceHandle, &status))
		{
			if (status.dwCurrentState == SERVICE_STOP_PENDING)
				Sleep(1000);
			else break;
		}
		if (status.dwCurrentState != SERVICE_STOPPED)
		{
			vWarning() << "service" << m_name << "could not be stopped.";
			return false;
		}
	}
	return true;
}

bool WindowsServiceControl::install(const QString& filePath, const QString& displayName)
{
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

	if (!m_serviceHandle)
	{
		const auto error = GetLastError();
		if (error == ERROR_SERVICE_EXISTS)
		{
			vCritical() << qUtf8Printable(tr("The service \"%1\" is already installed.").arg(m_name));
		}
		else
		{
			vCritical() << qUtf8Printable(tr("The service \"%1\" could not be installed.").arg(m_name));
		}
		return false;
	}

	auto serviceActions = getValueServiceActions();
	auto serviceFailureActions = getValueServiceFailureActions(&serviceActions);
	ChangeServiceConfig2(m_serviceHandle, SERVICE_CONFIG_FAILURE_ACTIONS, &serviceFailureActions);

	vInfo() << qUtf8Printable(tr("The service \"%1\" has been installed successfully.").arg(m_name));
	return true;
}

bool WindowsServiceControl::uninstall()
{
	if (!getValueCheckService(m_serviceHandle)) return false;
	if (!stop()) return false;

	if (!DeleteService(m_serviceHandle))
	{
		vCritical() << qUtf8Printable(tr("The service \"%1\" could not be uninstalled.").arg(m_name));
		return false;
	}

	vInfo() << qUtf8Printable(tr("The service \"%1\" has been uninstalled successfully.").arg(m_name));
	return true;
}

int WindowsServiceControl::startType()
{
	if (!getValueCheckService(m_serviceHandle)) return InvalidStartType;

	LPQUERY_SERVICE_CONFIG serviceConfig{ nullptr };
	DWORD bufferSize = 0;
	DWORD bytesNeeded = 0;

	if (!QueryServiceConfig(m_serviceHandle, nullptr, 0, &bytesNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		bufferSize = bytesNeeded;
		serviceConfig = LPQUERY_SERVICE_CONFIG(LocalAlloc(LMEM_FIXED, bufferSize));
	}
	else
	{
		return InvalidStartType;
	}

	if (!QueryServiceConfig(m_serviceHandle, serviceConfig, bufferSize, &bytesNeeded))
	{
		const auto error = GetLastError();
		vCritical() << error;
		LocalFree(serviceConfig);
		return InvalidStartType;
	}

	const auto startType = getValueStartType(serviceConfig);
	LocalFree(serviceConfig);
	return startType;
}

bool WindowsServiceControl::setStartType(int startType)
{
	if (!getValueCheckService(m_serviceHandle) || startType == InvalidStartType) return false;

	if (!ChangeServiceConfig(m_serviceHandle, SERVICE_NO_CHANGE, static_cast<DWORD>(startType), SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
	{
		vCritical() << qUtf8Printable(tr("The start type of service \"%1\" could not be changed.").arg(m_name));
		return false;
	}
	return true;
}

bool WindowsServiceControl::checkService() const
{
	if (!getValueCheckService(m_serviceHandle))
	{
		vCritical() << qUtf8Printable(tr("Service \"%1\" could not be found.").arg(m_name));
		return false;
	}
	return true;
}