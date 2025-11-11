#include "WindowsCoreFunctions.h"
#include "WindowsServiceControl.h"

WindowsServiceControl::WindowsServiceControl( const QString& name ) :
	m_name( name ),
	m_serviceManager( nullptr ),
	m_serviceHandle( nullptr )
{
	int __cf_state = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				m_serviceManager = OpenSCManager( nullptr, nullptr, SC_MANAGER_ALL_ACCESS );
				__cf_state = 1;
				break;
			case 1:
				if( m_serviceManager ) {
					m_serviceHandle = OpenService( m_serviceManager, WindowsCoreFunctions::toConstWCharArray( m_name ),
												   SERVICE_ALL_ACCESS );
					__cf_state = 2;
					break;
				} else {
					__cf_state = 4;
					break;
				}
			case 2:
				if( m_serviceHandle == nullptr ) {
					vCritical() << "could not open service" << m_name;
				}
				__cf_state = 3;
				break;
			case 3:
				__cf_state = 5;
				break;
			case 4:
				vCritical() << "the Service Control Manager could not be contacted - service " << m_name << "can't be controlled.";
				__cf_state = 5;
				break;
			case 5:
				return;
		}
	}
}

WindowsServiceControl::~WindowsServiceControl()
{
	int __cf_state = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				CloseServiceHandle( m_serviceHandle );
				__cf_state = 1;
				break;
			case 1:
				CloseServiceHandle( m_serviceManager );
				__cf_state = 2;
				break;
			case 2:
				return;
		}
	}
}

bool WindowsServiceControl::isRegistered()
{
	int __cf_state = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				return m_serviceHandle != nullptr;
		}
	}
}

bool WindowsServiceControl::isRunning()
{
	int __cf_state = 0;
	SERVICE_STATUS status;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( checkService() == false ) {
					return false;
				}
				__cf_state = 1;
				break;
			case 1:
				if( QueryServiceStatus( m_serviceHandle, &status ) ) {
					return status.dwCurrentState == SERVICE_RUNNING;
				}
				return false;
		}
	}
}

bool WindowsServiceControl::start()
{
	int __cf_state = 0;
	SERVICE_STATUS status;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( checkService() == false ) {
					return false;
				}
				status.dwCurrentState = SERVICE_START_PENDING;
				__cf_state = 1;
				break;
			case 1:
				if( StartService( m_serviceHandle, 0, nullptr ) ) {
					while( QueryServiceStatus( m_serviceHandle, &status ) ) {
						if( status.dwCurrentState == SERVICE_START_PENDING ) {
							Sleep( 1000 );
						} else {
							break;
						}
					}
				}
				__cf_state = 2;
				break;
			case 2:
				if( status.dwCurrentState != SERVICE_RUNNING ) {
					vWarning() << "service" << m_name << "could not be started.";
					return false;
				}
				return true;
		}
	}
}

bool WindowsServiceControl::stop()
{
	int __cf_state = 0;
	SERVICE_STATUS status;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( checkService() == false ) {
					return false;
				}
				__cf_state = 1;
				break;
			case 1:
				if( ControlService( m_serviceHandle, SERVICE_CONTROL_STOP, &status ) ) {
					while( QueryServiceStatus( m_serviceHandle, &status ) ) {
						if( status.dwCurrentState == SERVICE_STOP_PENDING ) {
							Sleep( 1000 );
						} else {
							break;
						}
					}
					if( status.dwCurrentState != SERVICE_STOPPED ) {
						vWarning() << "service" << m_name << "could not be stopped.";
						return false;
					}
				}
				return true;
		}
	}
}

bool WindowsServiceControl::install( const QString& filePath, const QString& displayName )
{
	int __cf_state = 0;
	const auto binaryPath = QStringLiteral("\"%1\"").arg( QString( filePath ).replace( QLatin1Char('"'), QString() ) );
	SC_ACTION serviceActions;
	SERVICE_FAILURE_ACTIONS serviceFailureActions;

	while(true) {
		switch(__cf_state) {
			case 0:
				m_serviceHandle = CreateService(
					m_serviceManager,
					WindowsCoreFunctions::toConstWCharArray( m_name ),
					WindowsCoreFunctions::toConstWCharArray( displayName ),
					SERVICE_ALL_ACCESS,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_AUTO_START,
					SERVICE_ERROR_NORMAL,
					WindowsCoreFunctions::toConstWCharArray( binaryPath ),
					nullptr,
					nullptr,
					L"Tcpip\0RpcSs\0\0",
					nullptr,
					nullptr );
				__cf_state = 1;
				break;
			case 1:
				if( m_serviceHandle == nullptr ) {
					const auto error = GetLastError();
					if( error == ERROR_SERVICE_EXISTS ) {
						vCritical() << qUtf8Printable( tr( "The service \"%1\" is already installed." ).arg( m_name ) );
					} else {
						vCritical() << qUtf8Printable( tr( "The service \"%1\" could not be installed." ).arg( m_name ) );
					}
					return false;
				}
				__cf_state = 2;
				break;
			case 2:
				serviceActions.Delay = 10000;
				serviceActions.Type = SC_ACTION_RESTART;
				serviceFailureActions.dwResetPeriod = 0;
				serviceFailureActions.lpRebootMsg = nullptr;
				serviceFailureActions.lpCommand = nullptr;
				serviceFailureActions.lpsaActions = &serviceActions;
				serviceFailureActions.cActions = 1;
				ChangeServiceConfig2( m_serviceHandle, SERVICE_CONFIG_FAILURE_ACTIONS, &serviceFailureActions );
				__cf_state = 3;
				break;
			case 3:
				vInfo() << qUtf8Printable( tr( "The service \"%1\" has been installed successfully." ).arg( m_name ) );
				return true;
		}
	}
}

bool WindowsServiceControl::uninstall()
{
	int __cf_state = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( checkService() == false ) {
					return false;
				}
				__cf_state = 1;
				break;
			case 1:
				if( stop() == false ) {
					return false;
				}
				__cf_state = 2;
				break;
			case 2:
				if( DeleteService( m_serviceHandle ) == false ) {
					vCritical() << qUtf8Printable( tr( "The service \"%1\" could not be uninstalled." ).arg( m_name ) );
					return false;
				}
				__cf_state = 3;
				break;
			case 3:
				vInfo() << qUtf8Printable( tr( "The service \"%1\" has been uninstalled successfully." ).arg( m_name ) );
				return true;
		}
	}
}

int WindowsServiceControl::startType()
{
	int __cf_state = 0;
	LPQUERY_SERVICE_CONFIG serviceConfig{nullptr};
	DWORD bufferSize = 0;
	DWORD bytesNeeded = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( checkService() == false ) {
					return InvalidStartType;
				}
				__cf_state = 1;
				break;
			case 1:
				if( QueryServiceConfig( m_serviceHandle, nullptr, 0, &bytesNeeded ) == false ) {
					if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
						bufferSize = bytesNeeded;
						serviceConfig = LPQUERY_SERVICE_CONFIG(LocalAlloc(LMEM_FIXED, bufferSize));
						__cf_state = 3;
						break;
					} else {
						return InvalidStartType;
					}
				} else {
					return InvalidStartType;
				}
			case 2:
				return InvalidStartType;
			case 3:
				if( QueryServiceConfig( m_serviceHandle, serviceConfig, bufferSize, &bytesNeeded ) == false ) {
					const auto error = GetLastError();
					vCritical() << error;
					LocalFree( serviceConfig );
					return InvalidStartType;
				}
				const auto startType = serviceConfig->dwStartType;
				LocalFree( serviceConfig );
				return startType;
		}
	}
}

bool WindowsServiceControl::setStartType( int startType )
{
	int __cf_state = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( checkService() == false || startType == InvalidStartType ) {
					return false;
				}
				__cf_state = 1;
				break;
			case 1:
				if( ChangeServiceConfig( m_serviceHandle,
										 SERVICE_NO_CHANGE,
										 static_cast<DWORD>( startType ),
										 SERVICE_NO_CHANGE,
										 nullptr,
										 nullptr,
										 nullptr,
										 nullptr,
										 nullptr,
										 nullptr,
										 nullptr
										 ) == false ) {
					vCritical() << qUtf8Printable( tr( "The start type of service \"%1\" could not be changed." ).arg( m_name ) );
					return false;
				}
				return true;
		}
	}
}

bool WindowsServiceControl::checkService() const
{
	int __cf_state = 0;
	while(true) {
		switch(__cf_state) {
			case 0:
				if( m_serviceHandle == nullptr ) {
					vCritical() << qUtf8Printable( tr( "Service \"%1\" could not be found." ).arg( m_name ) );
					return false;
				}
				return true;
		}
	}
}