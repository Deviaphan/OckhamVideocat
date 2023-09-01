#include "pch.h"
#include "KinopoiskUpdateDlg.h"
#include "Net/curl/curl.h"
#include "json/json.h"
#include "FileStore/femstore.h"
#include "FileStore/std_vector.h"
#include "FileStore/std_unordered_map.h"
#include "Net/CurlBuffer.h"
#include "Cyrillic.h"
#include <string>
#include <string.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "HexToUnicode.h"
#include "resource.h"
#include "genres.h"
#include "Kinopoisk/LoadKinopoiskPage.h"
#include <thread>
#include <omp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum
{
	PROCESS_TIMER = 4321
};


void LoadFilmInfoUnofficial( int id, KinopoiskInfo & info )
{
	Curl curl( 30, false, "Ockham:VideoCat" );

	// получил перманентный бан на кинопоиске. Теперь никакой быстрой проверки
	// сперва быстро проверяю, есть ли оценка
	//{
	//	CurlBuffer chunk;

	//	const char urlTemplate[] = "https://rating.kinopoisk.ru/%i.xml";

	//	CStringA curlQuery;
	//	curlQuery.Format( urlTemplate, id );

	//	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
	//	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	//	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

	//	if( CURLE_OK != curl_easy_perform( curl ) )
	//		return;

	//	const DWORD et = GetTickCount();
	//	chunk.TerminateBuffer();

	//	const char * ptr = (char *)chunk.GetData();
	//	if( !ptr )
	//		return;

	//	const char * pos = strstr( ptr, "kp_rating" );
	//	if( pos == nullptr )
	//		return;

	//	const char * lastPtr = ptr + chunk.GetSize();
	//	while( *pos != '>' )
	//	{
	//		++pos;
	//		if( pos >= lastPtr )
	//			break;
	//	}

	//	++pos;

	//	if( pos < lastPtr )
	//	{
	//		double v = atof( pos );
	//		info.rating = (unsigned char)(v * 10.0 + 0.5);
	//	}
	//}


	// http://kinopoiskapiunofficial.tech/user
	const std::string urlFormat = "kinopoiskapiunofficial.tech/api/v2.2/films/%i";

	CStringA curlQuery;
	curlQuery.Format( urlFormat.c_str(), id );

	CurlBuffer chunk;
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

	//curl.InitHeaders();
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_HEADER, 0 ) );
	{
#error kinopoiskapiunofficial X-API-KEY
		curl.GetCurlList().header = curl_slist_append( curl.GetCurlList().header, "X-API-KEY:*********" );

		curl.GetCurlList().header = curl_slist_append( curl.GetCurlList().header, "Content-Type: application/json; charset=utf-8" );
		curl.GetCurlList().header = curl_slist_append( curl.GetCurlList().header, "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3" );
		curl.GetCurlList().header = curl_slist_append( curl.GetCurlList().header, "DNT: 1" );
		curl.GetCurlList().header = curl_slist_append( curl.GetCurlList().header, "Cache-Control: max-age=0'" );
		curl.GetCurlList().header = curl_slist_append( curl.GetCurlList().header, "Connection: keep-alive" );

		VerifyCurl( curl_easy_setopt( curl, CURLOPT_HTTPHEADER, curl.GetCurlList().header ) );
	}

	if( CURLE_OK != curl_easy_perform( curl ) )
		return;

	chunk.TerminateBuffer();

	const char * json = (const char *)chunk.GetData();

	Json::Value root;
	{
		Json::CharReaderBuilder rb;
		std::unique_ptr<Json::CharReader> reader( rb.newCharReader() );
		Json::String err;
		reader->parse( (char *)chunk.GetData(), (char *)chunk.GetData() + chunk.GetSize(), &root, &err );
	}

	if( !root["kinopoiskId"].isInt() )
		return;
	info.filmID = root["kinopoiskId"].asInt();

	if( root["nameRu"].isString() )
		info.nameRU = ToUnicode( root["nameRu"].asString() );

	if( root["nameEn"].isString() )
		info.nameEN = ToUnicode( root["nameEn"].asString() );

	if( info.nameRU.empty() && info.nameEN.empty() )
	{
		if( root["nameOriginal"].isString() )
		{
			info.nameEN = ToUnicode( root["nameOriginal"].asString() );
			if( info.nameRU.empty() )
				info.nameRU = info.nameRU;
		}

		if( info.nameRU.empty() && info.nameEN.empty() )
		{
			return;
		}
	}

	if( root["slogan"].isString() )
		info.tagline = ToUnicode( root["slogan"].asString() );

	if( root["description"].isString() )
		info.description = ToUnicode( root["description"].asString() );

	if( root["year"].isString() )
	{
		const std::string y = root["year"].asString();
		if( !y.empty() )
		{
			try
			{
				info.year = std::stoi( y );
			}
			catch( ... )
			{
				;
			}
		}
	}

	if( root["genres"].isArray() )
	{
		Genres ImportKinopoiskGenres( const std::wstring & line ); //fwd

		Genres genres = 0;

		Json::Value genreArray = root["genres"];
		const int numItems = genreArray.size();
		for( int i = 0; i < numItems; ++i )
		{
			if( genreArray[i]["genre"].isString() )
			{
				const std::string s = genreArray[i]["genre"].asString();
				if( s.empty() )
					continue;

				std::wstring ss = ToUnicode( s );

				genres |= ImportKinopoiskGenres( ss );
			}
		}

		info.genres = genres;
	}

	if( root["ratingKinopoisk"].isDouble() )
	{
		info.rating = (unsigned char)(root["ratingKinopoisk"].asDouble() * 10.0 + 0.5);
	}	
}

void ThreadLoadInfo( CKinopoiskUpdateDlg* dlg )
{
	CWaitCursor waiting;

	curl_global_init( CURL_GLOBAL_ALL );

	DWORD st = GetTickCount();

	const int oldSize = dlg->_idPath.size();

	const int total = dlg->_ids.size();
	int count = 0;
	int loaded = 0;

	const int size = dlg->_ids.size();

	/*
	// сперва бинарным поиском проверю, когда закончатся фильмы, чтобы потом многопоточно грузить из оставшихся
	int right = size-1;

	KinopoiskInfo info;

	info.filmID = NO_KINOPOISK_ID;
	LoadFilmInfoUnofficial( dlg->_ids[right], info );

	if( info.filmID == NO_KINOPOISK_ID )
	{
		int offset = size / 2;
		do
		{
			info.filmID = NO_KINOPOISK_ID;
			LoadFilmInfoUnofficial( dlg->_ids[right], info );

			if( info.filmID == NO_KINOPOISK_ID )
				right -= offset;
			else
				right += offset;

			offset /= 2;
		}
		while( offset > 1 );

		size = right;
	}
	*/
	enum
	{
		RANGE_SIZE = 160
	};

	const int numRanges = size / RANGE_SIZE + 1;
	for( int lRange = 0; lRange < numRanges; ++lRange )
	{		
		const int last = (std::min)((lRange + 1) * RANGE_SIZE, size);
				
#pragma omp parallel for num_threads(16)
		for( int cur = lRange * RANGE_SIZE; cur < last; ++cur )
		{
			if( dlg->_stop )
				break;

			const int id = dlg->_ids[cur];

			KinopoiskInfo info;

			LoadFilmInfoUnofficial( id, info );

			if( info.filmID != NO_KINOPOISK_ID )
			{
#pragma omp critical
				{
					++loaded;
					dlg->AddItem( info );
				}
			}

#pragma omp atomic
			++count;

			CString text;
			text.Format( L"Загружено %i (%i) из %i", count, loaded, total );
#pragma omp critical
			{
				dlg->_editProgress.SetWindowText( text );
			}
		}

	}

	DWORD et = GetTickCount();

	curl_global_cleanup();

	const int newSize = dlg->_idPath.size();

	dlg->DrawDataBase( 0 );

	CString msg;
	msg.Format( L"Было-стало: %i   -  %i фильмов\n\rВремя загрузки: %.3f сек", oldSize, newSize, (et - st) / 1000.0 );

	AfxMessageBox( msg, MB_OK );
}


void ThreadUpdateRatings( CKinopoiskUpdateDlg * dlg )
{

}


CKinopoiskUpdateDlg::CKinopoiskUpdateDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_KINOPOISKUPDATE_DIALOG, pParent)
	, _startID( 298 )
	, _numImportID( 1 )
	, _progressTitle( _T( "" ) )
	, _filmID( 0 )
	, _year( 0 )
	, _genre( _T( "" ) )
	, _nameRU( _T( "" ) )
	, _nameEN( _T( "" ) )
	, _tagline( _T( "" ) )
	, _description( _T( "" ) )
	, _curProxy( -1 )
	, _proxyTitle( _T( "" ) )
	, _stop( false )
	, _rating( 0 )
	, _stopOnError( 1 )
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CKinopoiskUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_START_ID, _startID );
	DDX_Text( pDX, IDC_EDIT_NUM_ID, _numImportID );
	DDX_Control( pDX, IDC_BTN_IMPORT_KINOPOISK, _btnImport );
	//DDX_Text( pDX, IDC_EDIT_PROGRESS, _progressTitle );
	DDX_Text( pDX, IDC_KINOPOISK_ID, _filmID );
	DDX_Text( pDX, IDC_EDIT_YEAR, _year );
	DDX_Text( pDX, IDC_EDIT_GENRE, _genre );
	DDX_Text( pDX, IDC_EDIT_RU, _nameRU );
	DDX_Text( pDX, IDC_EDIT_EN, _nameEN );
	DDX_Text( pDX, IDC_EDIT_TAG, _tagline );
	DDX_Text( pDX, IDC_EDIT_DESC, _description );
	DDX_Control( pDX, IDC_EDIT_PROGRESS, _editProgress );
	DDX_Text( pDX, IDC_EDIT_RATING, _rating );
	DDX_Control( pDX, IDC_REFILL_LIST, _refillList );
}

BEGIN_MESSAGE_MAP(CKinopoiskUpdateDlg, CDialog)
	ON_BN_CLICKED( IDC_BTN_EXIT, OnBtnExit )
	ON_BN_CLICKED( IDC_BTN_EXPORT_DB, OnBtnExportDB )
	ON_BN_CLICKED( IDC_BTN_FIND_EMPTY, OnBtnFindEmpty )
	ON_BN_CLICKED( IDC_BTN_IMPORT_KINOPOISK, OnBtnImportKinopoisk )
	ON_BN_CLICKED( IDC_BTN_IMPORT_LOST, OnBtnImportLost )
	ON_BN_CLICKED( IDC_BTN_REFILL, OnBtnRefill )
	ON_BN_CLICKED( IDC_BTN_SHOW_FILM, OnBtnShowFilm )
	ON_BN_CLICKED( IDC_BTN_STOP, OnBtnStop )
	ON_BN_CLICKED( IDC_UPDATE_RATINGS, OnBnClickedUpdateRatings )
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CKinopoiskUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	BuildPath();

	int last = 298;
	for( auto& ii : _idPath )
	{
		if( ii.first > last )
			last = ii.first;
	}

	_startID = last + 1;

	_progressTitle.Format( L"Всего фильмов: %i.  Последний ID: %i", (int)_idPath.size(), last );
	_editProgress.SetWindowTextW( _progressTitle );

	if( !_idPath.empty() )
	{
		const int div = 100000;

		const int totalGroups = last / div + 5;

		std::vector<int> refill( totalGroups );

		for( const auto & ii : _idPath )
		{
			const int idx = ii.first / div;
			refill[idx]++;
		}

		for( int g = 0; g < refill.size(); ++g )
		{
			int start = g * div;
			int end = (g + 1) * div;
			CString line;
			line.Format( L"%i - %i  (left %i)", start, end, div - refill[g] );

			_refillList.AddString( line );
		}
	}

	UpdateData( FALSE );
	return TRUE;
}

void CKinopoiskUpdateDlg::BuildPath()
{
	try
	{
		CWaitCursor wait;

		std::ifstream in( "kinopoisk.bin", std::ios_base::binary );

		if( in.peek() == std::ifstream::traits_type::eof() )
			return;

		FileStore::Reader read( in, true );
		read & _idPath;
	}
	catch( ... )
	{
	}
}

void CKinopoiskUpdateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CKinopoiskUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CKinopoiskUpdateDlg::AddItem( const KinopoiskInfo & item )
{
	_idPath[item.filmID] = item;
	return true;
}


void CKinopoiskUpdateDlg::ImportFinished()
{
	if( !_idPath.empty() )
	{
		_startID = 0;
		for( auto& ii : _idPath )
		{
			if( ii.first > _startID )
				_startID = ii.first;
		}
	}

	_progressTitle.Format( L"Всего фильмов: %i", (int)_idPath.size() );
	_editProgress.SetWindowTextW( _progressTitle );

	UpdateData( FALSE );
}

bool CKinopoiskUpdateDlg::IsStoped()
{
	return _stop;
}


void CKinopoiskUpdateDlg::DrawDataBase( int currentIndex )
{
	_progressTitle.Format( L"Всего: %i\t\t%i / %i", (int)_idPath.size(), currentIndex, (int)_ids.size() );
	_editProgress.SetWindowTextW( _progressTitle );
}


void CKinopoiskUpdateDlg::ProcessID()
{
	/*
	if( _ids.empty() )
	{
		return;
	}

	CWaitCursor wait;

	const int id = _ids.front();
	_ids.pop_front();

	{
		Curl curl( 30, false, "Ockham:VideoCat" );

		// http://kinopoiskapiunofficial.tech/user
		std::string unoff = "kinopoiskapiunofficial.tech/api/v2.2/films/%i";

		CStringA curlQuery;
		curlQuery.Format( unoff.c_str(), id );

		CurlBuffer chunk;
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

		curl.InitHeaders();

		if( CURLE_OK != curl_easy_perform( curl ) )
			return;

		chunk.TerminateBuffer();

		const char * json = (const char *)chunk.GetData();

		Json::Value root;
		Json::Reader reader;
		reader.parse( json, json + chunk.GetSize(), root );

		Json::Value data = root["data"];

		int filmId = data["filmId"].asInt();
		int year = std::stoi( data["year"].asString() );

		std::wstring nameRu = ToUnicode( data["nameRu"].asString() );
		std::wstring nameEn = ToUnicode( data["nameEn"].asString() );
		std::wstring slogan = ToUnicode( data["slogan"].asString() );
		std::wstring description = ToUnicode( data["description"].asString() );

		Genres genres = 0;//977743

		Json::Value genreArray = data["genres"];
		const int numItems = genreArray.size();
		for( int i = 0; i < numItems; ++i )
		{
			std::wstring ss = ToUnicode( genreArray[i]["genre"].asString() );
			
			Genres ImportKinopoiskGenres( const std::wstring & line );

			genres |= ImportKinopoiskGenres( ss );

		}

		if( filmId != 0 && !nameRu.empty() )
		{
			KinopoiskInfo info;

			info.filmID = filmId;
			info.year = year;
			info.nameRU = nameRu;
			info.nameEN = nameEn;
			info.tagline = slogan;
			info.description = description;
			info.genres = genres;

			AddItem( info );
		}
	}

	
	//CStringA proxy;

	//KinopoiskInfo info;

	//if( ::LoadKinopoiskInfo( id, info, proxy ) )
	//	AddItem( info );
		
	DrawDataBase( 0 );

	*/
}

void CKinopoiskUpdateDlg::OnBtnImportKinopoisk()
{
	CWaitCursor waiting;

	UpdateData();
	_ids.clear();

	// загружаем N новых фильмов, начиная с заданного идентификатора
	if( _numImportID == 1 )
	{
		_ids.push_back( _startID );
	}
	else
	{
		int offset = 0;
		for( int i = 0; i < _numImportID; )
		{
			if( _idPath.find( _startID + offset ) == _idPath.end() )
			{
				_ids.push_back( _startID + offset );
				++i;
			}

			++offset;
		}
	}
	_stopOnError = 1;
	_stop = false;

	std::thread thread( ThreadLoadInfo, this );
	thread.detach();
}

bool CKinopoiskUpdateDlg::ProcessFilm( KinopoiskId id, const char * ptr, size_t size, bool isWindows1251 )
{
	
	return true;
}


void CKinopoiskUpdateDlg::OnBtnExit()
{
	if( IDYES == AfxMessageBox(L"Сохранить изменения в БД?", MB_YESNO) )
	{
		CWaitCursor wait;

		std::ofstream kpFile( "kinopoisk.bin", std::ios_base::binary | std::ios_base::trunc );
		FileStore::Writer write( kpFile, true );
		write & _idPath;
	}

	__super::OnOK();
}


void CKinopoiskUpdateDlg::OnBtnShowFilm()
{
	UpdateData();

	auto iiPath = _idPath.find( _filmID );
	if( iiPath == _idPath.end() )
	{
		_year = 0;
		_genre.Empty();
		_nameRU.Empty();
		_nameEN.Empty();
		_tagline.Empty();
		_description.Empty();
		_rating = 0.0;
	}
	else
	{
		_filmID = iiPath->second.filmID;
		_year = iiPath->second.year;

		_genre.Empty();
		for( int i = (int)Genre::FirstGenre; i < (int)Genre::NumOfGenres; ++i )
		{
			if( Test( iiPath->second.genres, (Genre)i ) )
				_genre += Descript( (Genre)i ) + L", ";
		}

		_nameRU = iiPath->second.nameRU.c_str();
		_nameEN = iiPath->second.nameEN.c_str();
		_tagline = iiPath->second.tagline.c_str();
		_description = iiPath->second.description.c_str();

		_rating = double( iiPath->second.rating ) / 10.0;
	}
	UpdateData( FALSE );
}


void CKinopoiskUpdateDlg::OnBtnImportLost()
{
	CWaitCursor waiting;

	UpdateData();
	
	const int idx = _refillList.GetCurSel();
	if( idx < 0 )
		return;

	const int minID = idx * 100000;
	const int maxID = (idx + 1) * 100000;

	_ids.clear();

	int cur = minID;
	const int last = maxID;

	for( ; cur < last; ++cur )
	{
		if( _idPath.find( cur ) == _idPath.end() )
			_ids.push_back( cur );
	}

	//std::sort( _ids.begin(), _ids.end(), [] ( const int a, const int b )->bool {return a < b; } );

	_numImportID = _ids.size();

	_stopOnError = 0;
	_stop = false;

	std::thread thread( ThreadLoadInfo, this );
	thread.detach();
}


void CKinopoiskUpdateDlg::OnBtnStop()
{
	_stop = TRUE;
}


void CKinopoiskUpdateDlg::OnBtnExportDB()
{
	std::vector<KinopoiskInfo> ki;
	ki.reserve( _idPath.size() );
	for( const auto & item : _idPath )
	{
		// У некоторых фильмов в названиях есть лишние постфиксы. Удаляем.
		KinopoiskInfo i = item.second;
		size_t pos = i.nameRU.find( L" (видео)" );
		if( pos == std::wstring::npos )
			pos = i.nameRU.find( L" (ТВ)" );

		if( pos != std::wstring::npos )
			i.nameRU.erase( pos, std::wstring::npos );

		ki.push_back( i );
	}
	
	std::ofstream out( "T:\\video.dat", std::ios_base::binary );
	FileStore::Writer w( out, false );
	w & ki;
}


void CKinopoiskUpdateDlg::OnBtnRefill()
{
	CWaitCursor waiting;
	UpdateData();

	const int idx = _refillList.GetCurSel();
	if( idx < 0 )
		return;

	const int minID = idx * 100000;
	const int maxID = (idx + 1) * 100000;

	_ids.clear();

	for( const auto & item : _idPath )
	{
		if( item.first < minID || item.first > maxID )
			continue;

		bool fill = false;

		fill |= item.second.nameRU.empty();
		fill |= item.second.nameEN.empty();
		//fill |= item.second.tagline.empty();	
		fill |= item.second.description.empty();
		fill |= item.second.genres == 0;
		fill |= item.second.year == 0;

		if( fill )
		{
			_ids.push_back( item.first );
		}
	}

	_numImportID = _ids.size();

	_stopOnError = 1;
	_stop = false;

	std::thread thread( ThreadLoadInfo, this );
	thread.detach();
}


void CKinopoiskUpdateDlg::OnBtnFindEmpty()
{
	CWaitCursor waiting;

	UpdateData();

	if( _idPath.empty() )
	{
		_startID = 298;
		UpdateData( FALSE );
		return;
	}

	int lastId = 0;
	for( auto& ii : _idPath )
	{
		if( ii.first > lastId )
			lastId = ii.first;
	}

	_startID += 1;

	while( _startID <= lastId )
	{
		if( _idPath.find( _startID ) == _idPath.end() )
			break;

		++_startID;
	};

	UpdateData( FALSE );
}


void CKinopoiskUpdateDlg::OnTimer( UINT_PTR nIDEvent )
{
	CDialog::OnTimer( nIDEvent );
}


__int64 GetSystemTimeAsUnixTime( CStringA & curTime )
{
	//Get the number of seconds since January 1, 1970 12:00am UTC
	//Code released into public domain; no attribution required.

	_SYSTEMTIME st;
	GetSystemTime( &st );
	curTime.Format( "%02i:%02i %i.%i.%i", st.wHour, st.wMinute, st.wDay, st.wMonth, st.wYear );

	const __int64 UNIX_TIME_START = 0x019DB1DED53E8000; //January 1, 1970 (start of Unix epoch) in "ticks"
	const __int64 TICKS_PER_SECOND = 10000000; //a tick is 100ns

	FILETIME ft;
	GetSystemTimeAsFileTime( &ft ); //returns ticks in UTC

	//Copy the low and high parts of FILETIME into a LARGE_INTEGER
	//This is so we can access the full 64-bits as an Int64 without causing an alignment fault
	LARGE_INTEGER li;
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	//Convert ticks since 1/1/1970 into seconds
	return (li.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND;
}


void CKinopoiskUpdateDlg::OnBnClickedUpdateRatings()
{
	return;

	//CWaitCursor waiting;

	//const char urlTemplate[] = "https://rating.kinopoisk.ru/%i.xml";
	///* result =
	//<rating><kp_rating num_vote="80229">6.548</kp_rating><imdb_rating num_vote="353000">6.8</imdb_rating></rating>
	//*/

	//Curl curl( 30, false );
	//CurlBuffer chunk;

	//for( auto & info : _idPath )
	//{
	//	//if( info.second.rating != 0 )
	//	//	continue;

	//	CStringA curlQuery;
	//	curlQuery.Format( urlTemplate, info.second.filmID );

	//	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
	//	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	//	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

	//	curl.InitHeaders();

	//	if( CURLE_OK != curl_easy_perform( curl ) )
	//		continue;

	//	const DWORD et = GetTickCount();
	//	chunk.TerminateBuffer();

	//	const char * ptr = (char *)chunk.GetData();
	//	if( !ptr )
	//		continue;

	//	const char * pos = strstr( ptr, "kp_rating" );
	//	if( pos == nullptr )
	//		break;

	//	const char * lastPtr = ptr + chunk.GetSize();
	//	while( *pos != '>' )
	//	{
	//		++pos;
	//		if( pos >= lastPtr )
	//			break;
	//	}

	//	++pos;

	//	if( pos < lastPtr )
	//	{
	//		double v = atof( pos );
	//		info.second.rating = (unsigned char)(v * 10.0 + 0.5);
	//	}

	//	chunk.Reset();
	//}

	//AfxMessageBox( L"OK" );
}
