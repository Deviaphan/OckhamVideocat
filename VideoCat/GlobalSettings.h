#pragma once

#include <vector>
#include "Net/CurlBuffer.h"
#include "Version.h"
#include "AviableFiles.h"
#include "ViewStyle.h"
#include "Commands/CommandID.h"

#define DEFAULT_THEME_NAME L".Светлая тема (стандартная)"

namespace MarkType
{
	enum Enum
	{
		MarkNotPlayed = 0,
		MarkPlayed,
		NoMark,
	};
}

namespace ViewType
{
	enum Enum
	{
		Tiles = 0,
		Mixed = 1,
		List = 2,
	};
}


bool IsReadOnlyFolder( const CString & path );

class GlobalSettings
{
	friend GlobalSettings & GetGlobal();


private:
	GlobalSettings();

public:
	void Init();
	void Destroy();

public:
	void ReadProxyList();

	void LoadFileExtensions();
	void SaveFileExtensions();

	// директория, в которую установлена программа
	const CString & GetProgramDirectory() const
	{
		return _programDir;
	}
	// рабочая директория. Либо папка с программой, либо папка %APPDATA%\videocat
	const CString & GetDefaultDirectory() const
	{
		return _userDirectory;
	}

	const CString& GetSettingsFileName() const
	{
		static const CString dir = GetDefaultDirectory() + L"ockham.json";
		return dir;
	}

	const CString & GetOldSettingsFileName() const
	{
		static const CString dir = GetDefaultDirectory() + L"ockham.ini";
		return dir;
	}

	const CString & GetKeyFileName() const
	{
		static const CString key = GetProgramDirectory() + L"key.png";
		return key;
	}

	const CString & GetThemeDirectory() const
	{
		static const CString key = GetProgramDirectory() + L"videocat\\theme\\";
		return key;
	}

	const CString & Get7zFileName() const
	{
		static const CString zipFile = GetProgramDirectory() + L"videocat\\7z\\7za.exe";
		return zipFile;
	}

	CString GetFilmsDatabase() const;

	// полный путь до генератора кадров
	CString GetThumbsGenerator() const;
	// полный путь до MediaInfo.dll
	CString GetMediaInfoDll() const;

	bool GetCloseBeforePlay() const
	{
		return _closeBeforePlay != 0;
	}
	void SetCloseBeforePlay( bool onoff );

	MarkType::Enum GetMarkType() const
	{
		return static_cast<MarkType::Enum>(_markType);
	}

	void SetMarkType( int markType );

	int GetViewType() const
	{
		return _viewType;
	}

	void SetViewType( int viewType );

	void SetShowStars( BOOL alwaysShow );
	BOOL GetShowStars() const
	{
		return alwaysShowRating;
	}

	CStringA GetProxy() const;

	void SetHideSecondTitle( BOOL onoff );

	BOOL ShowTipOfDay() const;
	void SetShowTipOfDay( BOOL onoff );
	CString GetHintDir() const;

	void SetNumRandomFilms( int num, BOOL autoplay );

	void SetTollbarCommands( const std::vector<CommandID> & tc );

	void SetViewScale( int scale );

public:
	Version versionInfo;

	CString programFullPath;
	CString programName;

	CRect bgFullSize;
	CRect bgTree;
	CRect bgView;

	BOOL alwaysShowRating;


	BOOL hideSecondTitle;

	AviableFiles aviabledFiles;

	int numRandomFilms;
	BOOL autoplayRandomFilm;

	ProxyType proxyType; // способ использования прокси-сервера
	void SetProxyType( int type );

	DWORD proxyIP;
	void SetProxyIP( DWORD ip );

	short proxyPort;
	void SetProxyPort( short port );

	unsigned proxyTimeout;
	void SetTimeout( unsigned timeout );

	ViewStyle * theme;
	int viewScale;

	std::vector<CommandID> toolbarCommands;

private:
	CString _programDir; // директория, в которой лежит ехе файл
	CString _userDirectory; // Локальная папка пользователя

	int _closeBeforePlay; // Закрывать каталогизатор при запуске воспроизведения фильма
	int _markType; // отмечать просмотренные или не просмотренные фильмы
	int _viewType; // способ отображения фильмов: плиткой или списком
	

	std::vector<CStringA> _proxyList; // список прокси-серверов
};


GlobalSettings & GetGlobal();

inline BOOL vcWritePrivateProfileInt( const wchar_t * lpAppName, const wchar_t * lpKeyName, int value, const wchar_t * lpFileName )
{
	wchar_t buffer[10] = {};

	wsprintf( buffer, L"%d", value );
	return ::WritePrivateProfileStringW( lpAppName, lpKeyName, buffer, lpFileName );
}

inline int vcGetPrivateProfileInt( const wchar_t * lpAppName, const wchar_t * lpKeyName, int defaultValue, const wchar_t * lpFileName )
{
	wchar_t defValue[10] = {};
	wsprintf( defValue, L"%d", defaultValue );

	wchar_t buffer[10] = {};
	::GetPrivateProfileStringW( lpAppName, lpKeyName, defValue, buffer, 10, lpFileName );
	return _wtoi( buffer );
}
