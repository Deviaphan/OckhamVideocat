// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "GlobalSettings.h"
#include <Rpcdce.h>
#include <fstream>
#include "VideoCat.h"
#include "LoadJpg.h"
#include "ResString.h"
#include "resource.h"
#include <omp.h>

enum
{
	UNICODE_BUFFER_SIZE = 32767,
};

GlobalSettings & GetGlobal()
{
	static GlobalSettings global;
	return global;
}


bool CanAccessFolder( LPCTSTR folderName, DWORD genericAccessRights )
{
	bool bRet = false;
	DWORD length = 0;
	if( !::GetFileSecurity( folderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION	| DACL_SECURITY_INFORMATION, nullptr, 0, &length ) &&
		ERROR_INSUFFICIENT_BUFFER == ::GetLastError() )
	{
		PSECURITY_DESCRIPTOR security = static_cast<PSECURITY_DESCRIPTOR>(::malloc( length ));
		if( security && ::GetFileSecurity( folderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, security, length, &length ) )
		{
			HANDLE hToken = nullptr;
			if( ::OpenProcessToken( ::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken ) )
			{
				HANDLE hImpersonatedToken = nullptr;
				if( ::DuplicateToken( hToken, SecurityImpersonation, &hImpersonatedToken ) )
				{
					PRIVILEGE_SET privileges;
					memset( &privileges, 0, sizeof( PRIVILEGE_SET ) );
					DWORD grantedAccess = 0;
					DWORD privilegesLength = sizeof( PRIVILEGE_SET );
					BOOL result = FALSE;

					GENERIC_MAPPING mapping;
					mapping.GenericRead = FILE_GENERIC_READ;
					mapping.GenericWrite = FILE_GENERIC_WRITE;
					mapping.GenericExecute = FILE_GENERIC_EXECUTE;
					mapping.GenericAll = FILE_ALL_ACCESS;

					::MapGenericMask( &genericAccessRights, &mapping );
					if( ::AccessCheck( security, hImpersonatedToken, genericAccessRights, &mapping, &privileges, &privilegesLength, &grantedAccess, &result ) )
					{
						bRet = result != FALSE;
					}
					::CloseHandle( hImpersonatedToken );
				}
				::CloseHandle( hToken );
			}
			::free( security );
		}
	}

	return bRet;
}

bool IsReadOnlyFolder( const CString & path )
{
	return !CanAccessFolder( path, GENERIC_READ | GENERIC_WRITE );
}

CString GetAppdataUserDirectory()
{
	wchar_t userPath[UNICODE_BUFFER_SIZE];
	if( !ExpandEnvironmentStrings( L"%APPDATA%\\VideoCat", userPath, UNICODE_BUFFER_SIZE ) )
		return CString();

	if( !PathFileExists( userPath ) )
		CreateDirectory( userPath, nullptr );

	CString result = userPath;
	result += L"\\";
	return result;
}

GlobalSettings::GlobalSettings()
	: _closeBeforePlay( 0 )
	, _markType( MarkType::MarkNotPlayed )
	, _viewType( ViewType::Mixed )
	, alwaysShowRating( FALSE )
	, hideSecondTitle( FALSE )
	, proxyType( ProxyType::UseTor )
	, proxyIP( (127 << 24) | (1) )
	, proxyPort( 9150 )
	, theme( nullptr )
	, numRandomFilms(10)
	, autoplayRandomFilm(FALSE)
	, viewScale(100)
	, proxyTimeout(0)
{
	// получаем полный путь
	{
		wchar_t path[MAX_PATH] = L"";

		HMODULE thisModule = nullptr;
		VERIFY( GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&__FUNCTION__, &thisModule ) );
		VERIFY( GetModuleFileName( thisModule, path, MAX_PATH ) );

		programFullPath = path;

		const int nameIndex = programFullPath.ReverseFind( L'\\' );
		if( nameIndex > 0 )
			programName = programFullPath.GetString() + nameIndex + 1;
		else
			programName = L"VideoCat.exe";

		VERIFY( PathRemoveFileSpec( path ) );
		_programDir = path;
		_programDir += L'\\';
	}

	if( ::IsReadOnlyFolder( _programDir ) )
		_userDirectory = ::GetAppdataUserDirectory();
	else
		_userDirectory = _programDir;
}

void GlobalSettings::Init()
{
#pragma omp parallel sections
	{
#pragma omp section
		{
			LoadFileExtensions();

			_closeBeforePlay = theApp.GetProfileInt( L"", L"closevc", 0 );
			_markType = theApp.GetProfileInt( L"", L"marktype", MarkType::MarkNotPlayed );
			_viewType = theApp.GetProfileInt( L"", L"viewtype", 0 );
			alwaysShowRating = theApp.GetProfileInt( L"", L"showstars", 0 );
			hideSecondTitle = theApp.GetProfileInt( L"", L"hidesecondtitle", 0 );

			numRandomFilms = theApp.GetProfileInt( L"", L"numRandomFilms", 10 );
			autoplayRandomFilm = theApp.GetProfileInt( L"", L"autoplayRandomFilm", 0 );

			viewScale = theApp.GetProfileInt( L"", L"viewScale", 100 );

			proxyType = (ProxyType)theApp.GetProfileInt( L"", L"proxyType", 0 );
			proxyIP = theApp.GetProfileInt( L"", L"proxyIP", (127 << 24) | (1) );
			proxyPort = (short)theApp.GetProfileInt( L"", L"proxyPort", 9150 );
			proxyTimeout = theApp.GetProfileInt( L"", L"proxyTimeout", 30 );
			
			{
				UINT numBytes = 0;
				LPBYTE dataPtr = nullptr;
				if( theApp.GetProfileBinary( L"", L"toolbarData", &dataPtr, &numBytes ) )
				{
					if( numBytes > 0 )
					{
						toolbarCommands.resize( numBytes / sizeof(CommandID) );
						memcpy_s( toolbarCommands.data(), numBytes, dataPtr, numBytes );
					}
					delete[] dataPtr;
				}
			}
		}

#pragma omp section
		{
			CString themeName = theApp.GetProfileString( L"", L"usedTheme", DEFAULT_THEME_NAME );
			CString themePath = GetGlobal().GetThemeDirectory();
			themePath += themeName;
			themePath += L"\\videocat.theme";

			theme = new ViewStyle;

			if( ::PathFileExists( themePath ) )
			{
				std::ifstream fin( themePath, std::ios_base::binary );
				FileStore::Reader reader( fin );

				reader & *theme;
				theme->bgImageFile = themeName;
			}
			else
			{
				DefaultTheme( *theme );
			}

			theme->Initialize(false);
		}

#pragma omp section
		{
			ReadProxyList();
		}
	}
}

void GlobalSettings::Destroy()
{
	delete theme;
	theme = nullptr;
}

CString GlobalSettings::GetFilmsDatabase() const
{
	return _programDir + L"videocat\\video.dat";
}

void GlobalSettings::LoadFileExtensions()
{
	const int numExtensions = theApp.GetProfileInt( L"extensions", L"count", 0 );
	if( numExtensions == 0 )
	{
		aviabledFiles.BuildDefaultExtensions();
	}
	else
	{
		for( int i = 0; i < numExtensions; ++i )
		{
			const CString ext = ResFormat( L"ext_%d", i );
			const CString player = ResFormat( L"player_%d", i );
			const CString cmdln = ResFormat( L"cmdln_%d", i );
			const CString skip3d = ResFormat( L"skip3d_%d", i );

			ExtData data;
			data.extension = theApp.GetProfileString( L"extensions", ext );
			if( data.extension.IsEmpty() )
				continue;

			data.player = theApp.GetProfileString( L"extensions", player );
			data.commandLine = theApp.GetProfileString( L"extensions", cmdln );
			data.ignoreIn3d = theApp.GetProfileInt( L"extensions", skip3d, 0 );

			aviabledFiles.extensions.push_back( data );
		}
	}
}

void GlobalSettings::SaveFileExtensions()
{
	theApp.WriteProfileInt( L"extensions", L"count", (int)aviabledFiles.extensions.size() );
	for( int i = 0; i < (int)aviabledFiles.extensions.size(); ++i )
	{
		const CString ext = ResFormat( L"ext_%d", i );
		const CString player = ResFormat( L"player_%d", i );
		const CString cmdln = ResFormat( L"cmdln_%d", i );
		const CString skip3d = ResFormat( L"skip3d_%d", i );

		const ExtData & data = aviabledFiles.extensions[i];
		theApp.WriteProfileString( L"extensions", ext, data.extension );
		theApp.WriteProfileString( L"extensions", player, data.player );
		theApp.WriteProfileString( L"extensions", cmdln, data.commandLine );
		theApp.WriteProfileInt( L"extensions", skip3d, (int)data.ignoreIn3d );
	}
}


CString GlobalSettings::GetThumbsGenerator() const
{
#ifdef NDEBUG
	const CString thumbExe = _programDir + L"videocat\\thumbs.exe";
#else
	const CString thumbExe = _programDir + L"videocat\\thumbs_d.exe";
#endif

	return thumbExe;
}

CString GlobalSettings::GetMediaInfoDll() const
{
	return _programDir + L"videocat\\MediaInfo.dll";
}

void GlobalSettings::SetCloseBeforePlay( bool onoff )
{
	if( _closeBeforePlay != (int)onoff )
	{
		_closeBeforePlay = onoff;
		theApp.WriteProfileInt( L"", L"closevc", _closeBeforePlay );
	}
}

void GlobalSettings::SetMarkType( int markType )
{
	if( _markType != markType )
	{
		_markType = markType;
		theApp.WriteProfileInt( L"", L"marktype", _markType );
	}
}

void GlobalSettings::SetViewType( int viewType )
{
	if( _viewType != viewType )
	{
		_viewType = viewType;
		theApp.WriteProfileInt( L"", L"viewtype", _viewType );
	}
}

void GlobalSettings::SetShowStars( BOOL alwaysShow )
{
	if( alwaysShowRating != alwaysShow )
	{
		alwaysShowRating = alwaysShow;
		theApp.WriteProfileInt( L"", L"showstars", alwaysShowRating );
	}
}

void GlobalSettings::SetHideSecondTitle( BOOL onoff )
{
	if( hideSecondTitle != onoff )
	{
		hideSecondTitle = onoff;
		theApp.WriteProfileInt( L"", L"hidesecondtitle", onoff );
	}
}

void GlobalSettings::SetNumRandomFilms( int num, BOOL autoplay )
{
	if( numRandomFilms != num )
	{
		numRandomFilms = num;
		theApp.WriteProfileInt( L"", L"numRandomFilms", numRandomFilms );
	}

	if( autoplayRandomFilm != autoplay )
	{
		autoplayRandomFilm = autoplay;
		theApp.WriteProfileInt( L"", L"autoplayRandomFilm", autoplayRandomFilm );
	}
}

void GlobalSettings::SetViewScale( int scale )
{
	viewScale = scale;
	theApp.WriteProfileInt( L"", L"viewScale", viewScale);
}

CStringA GlobalSettings::GetProxy() const
{
	if( proxyType == ProxyType::UseTor )
	{
		CStringA torIP;
		const DWORD ip = proxyIP;
		const DWORD port = proxyPort;
		torIP.Format( "%i.%i.%i.%i:%i", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff, port );
		return torIP;
	}
	else
	{
		if( _proxyList.empty() )
			return CStringA();

		const unsigned index = (unsigned)(((double)rand() / (double)RAND_MAX) * _proxyList.size());

		return _proxyList.at( index );
	}
}

void GlobalSettings::ReadProxyList()
{
	VC_TRY;

	_proxyList.clear();

	const CString proxyFile = _programDir + L"proxy.txt";
	if( !PathFileExists( proxyFile ) )
		return;

	std::ifstream in( proxyFile );
	while( in )
	{
		std::string proxy;
		in >> proxy;
		if( proxy.size() > 8 )
			_proxyList.emplace_back( proxy.c_str() );
	}

	VC_CATCH_ALL;
}

BOOL GlobalSettings::ShowTipOfDay() const
{
	return (BOOL)theApp.GetProfileInt( L"", L"showtips", 1 );
}

void GlobalSettings::SetShowTipOfDay( BOOL onoff )
{
	if( ShowTipOfDay() != onoff )
		theApp.WriteProfileInt( L"", L"showtips", onoff );
}

CString GlobalSettings::GetHintDir() const
{
	return _programDir + L"videocat\\hints\\";
}

void GlobalSettings::SetProxyType( int type )
{
	if( (ProxyType)type != proxyType )
	{
		proxyType = (ProxyType)type;
		theApp.WriteProfileInt( L"", L"proxyType", type );
	}
}

void GlobalSettings::SetProxyIP( DWORD ip )
{
	if( ip != proxyIP )
	{
		proxyIP = ip;
		theApp.WriteProfileInt( L"", L"proxyIP", proxyIP );
	}
}

void GlobalSettings::SetProxyPort( short port )
{
	if( port != proxyPort )
	{
		proxyPort = port;
		theApp.WriteProfileInt( L"", L"proxyPort", proxyPort );
	}
}

void GlobalSettings::SetTimeout( unsigned timeout )
{
	if( proxyTimeout != timeout )
	{
		proxyTimeout = timeout;
		theApp.WriteProfileInt( L"", L"proxyTimeout", proxyTimeout );
	}
}

void GlobalSettings::SetTollbarCommands( const std::vector<CommandID> & tc )
{
	toolbarCommands = tc;
	BOOL result = FALSE;
	if( toolbarCommands.empty() )
	{
		result = theApp.WriteProfileBinary( L"", L"toolbarData", nullptr, 0 );
	}
	else
	{
		const UINT numBytes = toolbarCommands.size() * sizeof( CommandID );
		result = theApp.WriteProfileBinary( L"", L"toolbarData", (LPBYTE)toolbarCommands.data(), numBytes );
	}
}
