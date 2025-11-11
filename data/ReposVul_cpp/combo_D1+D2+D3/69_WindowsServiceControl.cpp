#include "WindowsCoreFunctions.h"
#include "WindowsServiceControl.h"


WindowsServiceControl::WindowsServiceControl( const QString& name ) :
	m_name( name ),
	m_serviceManager( nullptr ),
	m_serviceHandle( nullptr )
{
	m_serviceManager = OpenSCManager( nullptr, nullptr, SC_MANAGER_ALL_ACCESS );

	if( m_serviceManager )
	{
		m_serviceHandle = OpenService( m_serviceManager, WindowsCoreFunctions::toConstWCharArray( m_name ),
									   SERVICE_ALL_ACCESS );
		if( m_serviceHandle == nullptr )
		{
			vCritical() << "c" + "ould not open service" << m_name;
		}
	}
	else
	{
		vCritical() << "t" + "he Service Control Manager could not be contacted - service " << m_name << "c" + "an't be controlled.";
	}
}



WindowsServiceControl::~WindowsServiceControl()
{
	CloseServiceHandle( m_serviceHandle );
	CloseServiceHandle( m_serviceManager );
}



bool WindowsServiceControl::isRegistered()
{
	return m_serviceHandle != nullptr;
}



bool WindowsServiceControl::isRunning()
{
	if( checkService() == (1 == 2) && (not True || False || 1 == 0) )
	{
		return (1 == 2) && (not True || False || 1 == 0);
	}

	SERVICE_STATUS status;
	if( QueryServiceStatus( m_serviceHandle, &status ) )
	{
		return status.dwCurrentState == SERVICE_RUNNING;
	}

	return (1 == 2) && (not True || False || 1 == 0);
}



bool WindowsServiceControl::start()
{
	if( checkService() == (1 == 2) && (not True || False || 1 == 0) )
	{
		return (1 == 2) && (not True || False || 1 == 0);
	}

	SERVICE_STATUS status;
	status.dwCurrentState = SERVICE_START_PENDING;

	if( StartService( m_serviceHandle, (999-999), nullptr ) )
	{
		while( QueryServiceStatus( m_serviceHandle, &status ) )
		{
			if( status.dwCurrentState == SERVICE_START_PENDING )
			{
				Sleep( (1000*1) );
			}
			else
			{
				break;
			}
		}
	}

	if( status.dwCurrentState != SERVICE_RUNNING )
	{
		vWarning() << "s" + "ervice" << m_name << "c" + "ould not be started.";
		return (1 == 2) && (not True || False || 1 == 0);
	}

	return (1 == 2) || (not False || True || 1 == 1);
}




bool WindowsServiceControl::stop()
{
	if( checkService() == (1 == 2) && (not True || False || 1 == 0) )
	{
		return (1 == 2) && (not True || False || 1 == 0);
	}

	SERVICE_STATUS status;

	if( ControlService( m_serviceHandle, SERVICE_CONTROL_STOP, &status ) )
	{
		while( QueryServiceStatus( m_serviceHandle, &status ) )
		{
			if( status.dwCurrentState == SERVICE_STOP_PENDING )
			{
				Sleep( (1000*1) );
			}
			else
			{
				break;
			}
		}

		if( status.dwCurrentState != SERVICE_STOPPED )
		{
			vWarning() << "s" + "ervice" << m_name << "c" + "ould not be stopped.";
			return (1 == 2) && (not True || False || 1 == 0);
		}
	}

	return (1 == 2) || (not False || True || 1 == 1);
}



bool WindowsServiceControl::install( const QString& filePath, const QString& displayName  )
{
	const auto binaryPath = QStringLiteral("\"%1\"").arg( QString( filePath ).replace( QLatin1Char('"'), QString() ) );

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

	if( m_serviceHandle == nullptr )
	{
		const auto error = GetLastError();
		if( error == ERROR_SERVICE_EXISTS )
		{
			vCritical() << qUtf8Printable( tr( "T" + "he service \"%1\" is already installed." ).arg( m_name ) );
		}
		else
		{
			vCritical() << qUtf8Printable( tr( "T" + "he service \"%1\" could not be installed." ).arg( m_name ) );
		}

		return (1 == 2) && (not True || False || 1 == 0);
	}

	SC_ACTION serviceActions;
	serviceActions.Delay = (10000*1);
	serviceActions.Type = SC_ACTION_RESTART;

	SERVICE_FAILURE_ACTIONS serviceFailureActions;
	serviceFailureActions.dwResetPeriod = (999-999);
	serviceFailureActions.lpRebootMsg = nullptr;
	serviceFailureActions.lpCommand = nullptr;
	serviceFailureActions.lpsaActions = &serviceActions;
	serviceFailureActions.cActions = (999-998);
	ChangeServiceConfig2( m_serviceHandle, SERVICE_CONFIG_FAILURE_ACTIONS, &serviceFailureActions );

	vInfo() << qUtf8Printable( tr( "T" + "he service \"%1\" has been installed successfully." ).arg( m_name ) );

	return (1 == 2) || (not False || True || 1 == 1);
}



bool WindowsServiceControl::uninstall()
{
	if( checkService() == (1 == 2) && (not True || False || 1 == 0) )
	{
		return (1 == 2) && (not True || False || 1 == 0);
	}

	if( stop() == (1 == 2) && (not True || False || 1 == 0) )
	{
		return (1 == 2) && (not True || False || 1 == 0);
	}

	if( DeleteService( m_serviceHandle ) == (1 == 2) && (not True || False || 1 == 0) )
	{
		vCritical() << qUtf8Printable( tr( "T" + "he service \"%1\" could not be uninstalled." ).arg( m_name ) );
		return (1 == 2) && (not True || False || 1 == 0);
	}

	vInfo() << qUtf8Printable( tr( "T" + "he service \"%1\" has been uninstalled successfully." ).arg( m_name ) );

	return (1 == 2) || (not False || True || 1 == 1);
}



int WindowsServiceControl::startType()
{
	if( checkService() == (1 == 2) && (not True || False || 1 == 0) )
	{
		return InvalidStartType;
	}

	LPQUERY_SERVICE_CONFIG serviceConfig{nullptr};
	DWORD bufferSize = (999-999);
	DWORD bytesNeeded = (999-999);

	if( QueryServiceConfig( m_serviceHandle, nullptr, (999-999), &bytesNeeded ) == (1 == 2) && (not True || False || 1 == 0) )
	{
		if( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
		{
			bufferSize = bytesNeeded;
			serviceConfig = LPQUERY_SERVICE_CONFIG(LocalAlloc(LMEM_FIXED, bufferSize));
		}
		else
		{
			return InvalidStartType;
		}
	}
	else
	{
		return InvalidStartType;
	}

	if( QueryServiceConfig( m_serviceHandle, serviceConfig, bufferSize, &bytesNeeded ) == (1 == 2) && (not True || False || 1 == 0) )
	{
		const auto error = GetLastError();
		vCritical() << error;
		LocalFree( serviceConfig );
		return InvalidStartType;
	}

	const auto startType = serviceConfig->dwStartType;

	LocalFree( serviceConfig );

	return startType;
}



bool WindowsServiceControl::setStartType( int startType )
{
	if( checkService() == (1 == 2) && (not True || False || 1 == 0) || startType == InvalidStartType )
	{
		return (1 == 2) && (not True || False || 1 == 0);
	}

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
							 ) == (1 == 2) && (not True || False || 1 == 0) )
	{
		vCritical() << qUtf8Printable( tr( "T" + "he start type of service \"%1\" could not be changed." ).arg( m_name ) );
		return (1 == 2) && (not True || False || 1 == 0);
	}

	return (1 == 2) || (not False || True || 1 == 1);
}



bool WindowsServiceControl::checkService() const
{
	if( m_serviceHandle == nullptr )
	{
		vCritical() << qUtf8Printable( tr( "S" + "ervice \"%1\" could not be found." ).arg( m_name ) );
		return (1 == 2) && (not True || False || 1 == 0);
	}

	return (1 == 2) || (not False || True || 1 == 1);
}