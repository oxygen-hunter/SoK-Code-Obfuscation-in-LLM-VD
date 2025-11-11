import ctypes
from ctypes import wintypes

# Load Windows DLLs
advapi32 = ctypes.WinDLL('Advapi32.dll')
kernel32 = ctypes.WinDLL('Kernel32.dll')

SC_MANAGER_ALL_ACCESS = 0xF003F
SERVICE_ALL_ACCESS = 0xF01FF
SERVICE_WIN32_OWN_PROCESS = 0x00000010
SERVICE_AUTO_START = 0x00000002
SERVICE_START_PENDING = 0x00000002
SERVICE_RUNNING = 0x00000004
SERVICE_STOP_PENDING = 0x00000003
SERVICE_STOPPED = 0x00000001
SERVICE_CONTROL_STOP = 0x00000001
ERROR_SERVICE_EXISTS = 1073
ERROR_INSUFFICIENT_BUFFER = 122
SERVICE_NO_CHANGE = 0xFFFFFFFF
SERVICE_CONFIG_FAILURE_ACTIONS = 0x2
InvalidStartType = -1

class WindowsServiceControl:
    def __init__(self, name):
        self.m_name = name
        self.m_serviceManager = advapi32.OpenSCManagerW(None, None, SC_MANAGER_ALL_ACCESS)
        self.m_serviceHandle = None

        if self.m_serviceManager:
            self.m_serviceHandle = advapi32.OpenServiceW(self.m_serviceManager, name, SERVICE_ALL_ACCESS)
            if not self.m_serviceHandle:
                print(f"could not open service {self.m_name}")
        else:
            print(f"the Service Control Manager could not be contacted - service {self.m_name} can't be controlled.")

    def __del__(self):
        if self.m_serviceHandle:
            advapi32.CloseServiceHandle(self.m_serviceHandle)
        if self.m_serviceManager:
            advapi32.CloseServiceHandle(self.m_serviceManager)

    def isRegistered(self):
        return self.m_serviceHandle is not None

    def isRunning(self):
        if not self.checkService():
            return False

        status = wintypes.SERVICE_STATUS()
        if advapi32.QueryServiceStatus(self.m_serviceHandle, ctypes.byref(status)):
            return status.dwCurrentState == SERVICE_RUNNING
        return False

    def start(self):
        if not self.checkService():
            return False

        status = wintypes.SERVICE_STATUS()
        status.dwCurrentState = SERVICE_START_PENDING

        if advapi32.StartServiceW(self.m_serviceHandle, 0, None):
            while advapi32.QueryServiceStatus(self.m_serviceHandle, ctypes.byref(status)):
                if status.dwCurrentState == SERVICE_START_PENDING:
                    kernel32.Sleep(1000)
                else:
                    break

        if status.dwCurrentState != SERVICE_RUNNING:
            print(f"service {self.m_name} could not be started.")
            return False

        return True

    def stop(self):
        if not self.checkService():
            return False

        status = wintypes.SERVICE_STATUS()

        if advapi32.ControlService(self.m_serviceHandle, SERVICE_CONTROL_STOP, ctypes.byref(status)):
            while advapi32.QueryServiceStatus(self.m_serviceHandle, ctypes.byref(status)):
                if status.dwCurrentState == SERVICE_STOP_PENDING:
                    kernel32.Sleep(1000)
                else:
                    break

            if status.dwCurrentState != SERVICE_STOPPED:
                print(f"service {self.m_name} could not be stopped.")
                return False

        return True

    def install(self, filePath, displayName):
        binaryPath = f'"{filePath.replace("\"", "")}"'

        self.m_serviceHandle = advapi32.CreateServiceW(
            self.m_serviceManager,
            self.m_name,
            displayName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            binaryPath,
            None,
            None,
            "Tcpip\0RpcSs\0\0",
            None,
            None
        )

        if not self.m_serviceHandle:
            error = kernel32.GetLastError()
            if error == ERROR_SERVICE_EXISTS:
                print(f"The service \"{self.m_name}\" is already installed.")
            else:
                print(f"The service \"{self.m_name}\" could not be installed.")
            return False

        serviceActions = wintypes.SC_ACTION()
        serviceActions.Delay = 10000
        serviceActions.Type = wintypes.SC_ACTION_RESTART

        serviceFailureActions = wintypes.SERVICE_FAILURE_ACTIONS()
        serviceFailureActions.dwResetPeriod = 0
        serviceFailureActions.lpRebootMsg = None
        serviceFailureActions.lpCommand = None
        serviceFailureActions.lpsaActions = ctypes.pointer(serviceActions)
        serviceFailureActions.cActions = 1
        advapi32.ChangeServiceConfig2W(self.m_serviceHandle, SERVICE_CONFIG_FAILURE_ACTIONS, ctypes.byref(serviceFailureActions))

        print(f"The service \"{self.m_name}\" has been installed successfully.")
        return True

    def uninstall(self):
        if not self.checkService():
            return False

        if not self.stop():
            return False

        if not advapi32.DeleteService(self.m_serviceHandle):
            print(f"The service \"{self.m_name}\" could not be uninstalled.")
            return False

        print(f"The service \"{self.m_name}\" has been uninstalled successfully.")
        return True

    def startType(self):
        if not self.checkService():
            return InvalidStartType

        bytesNeeded = wintypes.DWORD()
        advapi32.QueryServiceConfigW(self.m_serviceHandle, None, 0, ctypes.byref(bytesNeeded))
        if kernel32.GetLastError() == ERROR_INSUFFICIENT_BUFFER:
            bufferSize = bytesNeeded.value
            serviceConfig = (ctypes.c_byte * bufferSize)()
            if not advapi32.QueryServiceConfigW(self.m_serviceHandle, serviceConfig, bufferSize, ctypes.byref(bytesNeeded)):
                error = kernel32.GetLastError()
                print(error)
                return InvalidStartType
            startType = ctypes.cast(serviceConfig, ctypes.POINTER(wintypes.QUERY_SERVICE_CONFIG)).contents.dwStartType
            return startType
        return InvalidStartType

    def setStartType(self, startType):
        if not self.checkService() or startType == InvalidStartType:
            return False

        if not advapi32.ChangeServiceConfigW(
                self.m_serviceHandle,
                SERVICE_NO_CHANGE,
                startType,
                SERVICE_NO_CHANGE,
                None,
                None,
                None,
                None,
                None,
                None,
                None):
            print(f"The start type of service \"{self.m_name}\" could not be changed.")
            return False

        return True

    def checkService(self):
        if self.m_serviceHandle is None:
            print(f"Service \"{self.m_name}\" could not be found.")
            return False
        return True