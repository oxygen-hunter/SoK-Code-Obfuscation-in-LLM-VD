#include "WindowsCoreFunctions.h"
#include "WindowsServiceControl.h"

OX7B4DF339::OX7B4DF339( const QString& OX7D3E9C93 ) :
	OX86E66E46( OX7D3E9C93 ),
	OXD2A1E5EC( nullptr ),
	OXB9A8F4B3( nullptr )
{
	OXD2A1E5EC = OpenSCManager( nullptr, nullptr, SC_MANAGER_ALL_ACCESS );

	if( OXD2A1E5EC )
	{
		OXB9A8F4B3 = OpenService( OXD2A1E5EC, WindowsCoreFunctions::toConstWCharArray( OX86E66E46 ),
									   SERVICE_ALL_ACCESS );
		if( OXB9A8F4B3 == nullptr )
		{
			vCritical() << "could not open service" << OX86E66E46;
		}
	}
	else
	{
		vCritical() << "the Service Control Manager could not be contacted - service " << OX86E66E46 << "can't be controlled.";
	}
}

OX7B4DF339::~OX7B4DF339()
{
	CloseServiceHandle( OXB9A8F4B3 );
	CloseServiceHandle( OXD2A1E5EC );
}

bool OX7B4DF339::OX2D4A3F6F()
{
	return OXB9A8F4B3 != nullptr;
}

bool OX7B4DF339::OX3F8E938E()
{
	if( OX08D94653() == false )
	{
		return false;
	}

	SERVICE_STATUS OX9C6F7D7A;
	if( QueryServiceStatus( OXB9A8F4B3, &OX9C6F7D7A ) )
	{
		return OX9C6F7D7A.dwCurrentState == SERVICE_RUNNING;
	}

	return false;
}

bool OX7B4DF339::OXA2B1F5EF()
{
	if( OX08D94653() == false )
	{
		return false;
	}

	SERVICE_STATUS OX9C6F7D7A;
	OX9C6F7D7A.dwCurrentState = SERVICE_START_PENDING;

	if( StartService( OXB9A8F4B3, 0, nullptr ) )
	{
		while( QueryServiceStatus( OXB9A8F4B3, &OX9C6F7D7A ) )
		{
			if( OX9C6F7D7A.dwCurrentState == SERVICE_START_PENDING )
			{
				Sleep( 1000 );
			}
			else
			{
				break;
			}
		}
	}

	if( OX9C6F7D7A.dwCurrentState != SERVICE_RUNNING )
	{
		vWarning() << "service" << OX86E66E46 << "could not be started.";
		return false;
	}

	return true;
}

bool OX7B4DF339::OX1F2E4B1D()
{
	if( OX08D94653() == false )
	{
		return false;
	}

	SERVICE_STATUS OX9C6F7D7A;

	if( ControlService( OXB9A8F4B3, SERVICE_CONTROL_STOP, &OX9C6F7D7A ) )
	{
		while( QueryServiceStatus( OXB9A8F4B3, &OX9C6F7D7A ) )
		{
			if( OX9C6F7D7A.dwCurrentState == SERVICE_STOP_PENDING )
			{
				Sleep( 1000 );
			}
			else
			{
				break;
			}
		}

		if( OX9C6F7D7A.dwCurrentState != SERVICE_STOPPED )
		{
			vWarning() << "service" << OX86E66E46 << "could not be stopped.";
			return false;
		}
	}

	return true;
}

bool OX7B4DF339::OXA0F1E4C0( const QString& OX7E3A2C9A, const QString& OX5C2D7B9F  )
{
	const auto OX1A4F2C3B = QStringLiteral("\"%1\"").arg( QString( OX7E3A2C9A ).replace( QLatin1Char('"'), QString() ) );

	OXB9A8F4B3 = CreateService(
				OXD2A1E5EC,
				WindowsCoreFunctions::toConstWCharArray( OX86E66E46 ),
				WindowsCoreFunctions::toConstWCharArray( OX5C2D7B9F ),
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START,
				SERVICE_ERROR_NORMAL,
				WindowsCoreFunctions::toConstWCharArray( OX1A4F2C3B ),
				nullptr,
				nullptr,
				L"Tcpip\0RpcSs\0\0",
				nullptr,
				nullptr );

	if( OXB9A8F4B3 == nullptr )
	{
		const auto OX3F6A8C4D = GetLastError();
		if( OX3F6A8C4D == ERROR_SERVICE_EXISTS )
		{
			vCritical() << qUtf8Printable( tr( "The service \"%1\" is already installed." ).arg( OX86E66E46 ) );
		}
		else
		{
			vCritical() << qUtf8Printable( tr( "The service \"%1\" could not be installed." ).arg( OX86E66E46 ) );
		}

		return false;
	}

	SC_ACTION OX7D9E6C2B;
	OX7D9E6C2B.Delay = 10000;
	OX7D9E6C2B.Type = SC_ACTION_RESTART;

	SERVICE_FAILURE_ACTIONS OX3D8A5F1E;
	OX3D8A5F1E.dwResetPeriod = 0;
	OX3D8A5F1E.lpRebootMsg = nullptr;
	OX3D8A5F1E.lpCommand = nullptr;
	OX3D8A5F1E.lpsaActions = &OX7D9E6C2B;
	OX3D8A5F1E.cActions = 1;
	ChangeServiceConfig2( OXB9A8F4B3, SERVICE_CONFIG_FAILURE_ACTIONS, &OX3D8A5F1E );

	vInfo() << qUtf8Printable( tr( "The service \"%1\" has been installed successfully." ).arg( OX86E66E46 ) );

	return true;
}

bool OX7B4DF339::OX5F7E8D3A()
{
	if( OX08D94653() == false )
	{
		return false;
	}

	if( OX1F2E4B1D() == false )
	{
		return false;
	}

	if( DeleteService( OXB9A8F4B3 ) == false )
	{
		vCritical() << qUtf8Printable( tr( "The service \"%1\" could not be uninstalled." ).arg( OX86E66E46 ) );
		return false;
	}

	vInfo() << qUtf8Printable( tr( "The service \"%1\" has been uninstalled successfully." ).arg( OX86E66E46 ) );

	return true;
}

int OX7B4DF339::OX6D4C2A9F()
{
	if( OX08D94653() == false )
	{
		return OX5C9F1E2B;
	}

	LPQUERY_SERVICE_CONFIG OX0E9C7D3A{nullptr};
	DWORD OX7A6F4E2D = 0;
	DWORD OX3D1E2F4A = 0;

	if( QueryServiceConfig( OXB9A8F4B3, nullptr, 0, &OX3D1E2F4A ) == false )
	{
		if( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
		{
			OX7A6F4E2D = OX3D1E2F4A;
			OX0E9C7D3A = LPQUERY_SERVICE_CONFIG(LocalAlloc(LMEM_FIXED, OX7A6F4E2D));
		}
		else
		{
			return OX5C9F1E2B;
		}
	}
	else
	{
		return OX5C9F1E2B;
	}

	if( QueryServiceConfig( OXB9A8F4B3, OX0E9C7D3A, OX7A6F4E2D, &OX3D1E2F4A ) == false )
	{
		const auto OX3F6A8C4D = GetLastError();
		vCritical() << OX3F6A8C4D;
		LocalFree( OX0E9C7D3A );
		return OX5C9F1E2B;
	}

	const auto OX3A8F9E2D = OX0E9C7D3A->dwStartType;

	LocalFree( OX0E9C7D3A );

	return OX3A8F9E2D;
}

bool OX7B4DF339::OX9C7F1E5A( int OX3A8F9E2D )
{
	if( OX08D94653() == false || OX3A8F9E2D == OX5C9F1E2B )
	{
		return false;
	}

	if( ChangeServiceConfig( OXB9A8F4B3,
							 SERVICE_NO_CHANGE,
							 static_cast<DWORD>( OX3A8F9E2D ),
							 SERVICE_NO_CHANGE,
							 nullptr,
							 nullptr,
							 nullptr,
							 nullptr,
							 nullptr,
							 nullptr,
							 nullptr
							 ) == false )
	{
		vCritical() << qUtf8Printable( tr( "The start type of service \"%1\" could not be changed." ).arg( OX86E66E46 ) );
		return false;
	}

	return true;
}

bool OX7B4DF339::OX08D94653() const
{
	if( OXB9A8F4B3 == nullptr )
	{
		vCritical() << qUtf8Printable( tr( "Service \"%1\" could not be found." ).arg( OX86E66E46 ) );
		return false;
	}

	return true;
}