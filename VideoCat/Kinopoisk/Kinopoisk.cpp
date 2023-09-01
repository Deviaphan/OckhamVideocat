// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "Kinopoisk.h"

#include <algorithm>
#include <set>
#include "../Cyrillic.h"
#include "../Translite.h"
#include "../WaitDlg.h"
#include "Net/CurlBuffer.h"
#include "../ErrorDlg.h"
#include "../GlobalSettings.h"
#include "../CopyWebPageDlg.h"
#include "../ResString.h"
#include "json/json.h"

Kinopoisk & GetKinopoisk()
{
	static Kinopoisk kinopoisk;

	return kinopoisk;
}


Kinopoisk::Kinopoisk()
	: _database( nullptr )
{

}

Kinopoisk::~Kinopoisk()
{
	delete _database;
}


void Kinopoisk::OpenSite( KinopoiskId id )
{
	if( id <= 0 )
		return;

	CString filmUrl;
	filmUrl.Format( L"https://www.kinopoisk.ru/film/%u/", id );

	::ShellExecute( nullptr, L"open", filmUrl, 0, 0, SW_SHOWNORMAL );
}

void Kinopoisk::OpenSimilarSite( KinopoiskId id )
{
	if( id <= 0 )
		return;

	CString filmUrl;
	filmUrl.Format( L"https://kinopoisk.ru/film/%u/like/", id );

	::ShellExecute( nullptr, L"open", filmUrl, 0, 0, SW_SHOWNORMAL );
}

void Kinopoisk::LoadDb()
{
	if( _database != nullptr )
		return;

	NoMadalWait waitDlg;

	_database = new FilmDb;
	//DWORD t1 = GetTickCount();
	_database->LoadFromFile();
	//DWORD t2 = GetTickCount();
	_database->BuildHash();
	//DWORD t3 = GetTickCount();

	//CString s;
	//s.Format( L"load = %g\nhash = %g", (t2 - t1) / 1000.0, (t3-t2) / 1000.0 );
	//AfxMessageBox( s );
}

// Частенько бывают ошибки завышения-занижения года на единицу (дата выпуска и проката, например. Поэтому сравниваю не точно
bool FuzzyEqual( int y1, int y2, bool fuzzy )
{
	if( fuzzy )
		return std::abs( y1 - y2 ) < 2;
	else
		return y1 == y2;
}

int Kinopoisk::SearchInfo( const CString & searchTitle, std::set<int> & resultIdx, bool fuzzy, bool firstPass, bool draftSearch )
{
	VC_TRY;

	resultIdx.clear();

	const unsigned minYear = 1900;
	const unsigned maxYear = 2030;

	LoadDb();

	if( _database->data.empty() )
		return 0;

	unsigned year = 0;
	std::wstring filmVersion;
	int filmVer = 0;

	std::set<int> filmIndices;

	std::wstring name = searchTitle.GetString();
	::CharLowerBuffW( name.data(), name.size() );
	// заменяем букву ё
	std::replace( name.begin(), name.end(), L'ё', L'е' );

	bool hasRu = false;
	bool hasEn = false;
	bool onlyYear = false;

	wchar_t token[64];
	// разбиваем на токены русское название и хэшируем
	unsigned start = 0;
	while( true )
	{
		unsigned tokenSize = Tokenize( name, start, token );
		if( !token[0] )
			break;

		if( iswdigit( token[0] ) )
		{
			unsigned testYear = _wtoi( token );
			if( !year && testYear > minYear && testYear < maxYear )
			{
				year = testYear;
				continue;
			}

			filmVersion = token; // возможно, это номер версии
			filmVer = testYear;
			continue;
		}
		else if( HasRus( token ) )
		{
			if( tokenSize == 1 || IgnoredToken( token ) )
				continue;

			hasRu = true;
		}
		else
		{
			if( (tokenSize <= 2) || IgnoredToken( token ) )
				continue;

			hasEn = true;
		}

		const FilmDb::HashType hash = FilmDb::HashWord( token, tokenSize );

		//собираем информацию обо всех фильмах, содержащих это слово
		const auto & films = _database->GetFilms( hash );
		for( const int & film : films )
		{
			if( (hasRu || hasEn) && year )
			{
				// Если год уже определён, можно сразу отфильтровать ложные
				const KinopoiskInfo & ki = _database->data[film];
				if( !FuzzyEqual( ki.year, year, true ) )
					continue;
			}

			filmIndices.insert( film );
		}
	}

	if( !hasRu && !hasEn )
	{
		hasRu = true;
		onlyYear = true;
	}

	CString y;
	if( year )
	{
		y.Format( L"%u", year );
		name += L" ";
		name += y.GetString();
	}

	// название может состоять из одного года
	if( filmIndices.empty() && year )
	{
		const FilmDb::HashType hash = FilmDb::HashWord( y.GetString(), y.GetLength() );

		const auto & films = _database->GetFilms( hash );
		for( const int & film : films )
		{
			filmIndices.insert( film );
		}
	}

	int bestScores = 0;
	int bestPenalty = 1000;

	for( const int & filmIdx : filmIndices )
	{
		const KinopoiskInfo & ki = _database->data[filmIdx];

		if( !onlyYear && year )
		{
			if( !FuzzyEqual( ki.year, year, fuzzy ) )
				continue;
		}

		std::wstring filmName;
		if( hasRu )
			filmName = ki.nameRU + L" ";

		size_t enOffset = filmName.size();

		if( hasEn )
			filmName += ki.nameEN;

		::CharLowerBuffW( filmName.data(), filmName.size() );

		int penalty = 0;
		int scores = 0;

		if( !filmVersion.empty() && !filmName.empty() )
		{
			// добавляем очков, если "номер версии" совпал
			size_t pos = filmName.find( filmVersion, 0 );
			if( pos != std::wstring::npos )
			{
				// проверяем, что это отдельно стоящая цифра
				bool realVersion = true;
				if( pos > 0 )
				{
					if( iswdigit( filmName[pos - 1] ) )
						realVersion = false;
				}
				if( _wtoi( &filmName[pos] ) != filmVer )
					realVersion = false;

				if( realVersion )
					++scores;
			}
		}

		bool hasWords = false;

		size_t pos = 0;
		start = 0;
		while( true )
		{
			unsigned tokenSize = Tokenize( filmName, start, token );
			if( !token[0] )
				break;

			//if( HasRus( token ) )
			//{
			//	if( /*tokenSize == 1 ||*/ IgnoredToken(token) )
			//		continue;
			//}
			//else
			//{
			//	if( /*(tokenSize <= 2) ||*/ IgnoredToken(token) )
			//		continue;
			//}

			if( start >= enOffset )
			{
				pos = 0;
				enOffset = 1000000;
			}

			size_t curPos = name.find( token, pos );
			if( curPos != std::wstring::npos )
			{
				if( (curPos + tokenSize) == name.size() || IsDelimiter( name[curPos + tokenSize] ) )
				{
					++scores;
					hasWords = true;
					pos = curPos + tokenSize; // сдвигаем, чтобы не находить каждый раз один и тот же токен

				}
				else
					++penalty;
			}
			else
			{
				//--scores;
				++penalty;
			}
		}

		if( !hasWords )
			continue;

		if( draftSearch )
		{
			//if( scores > 0 )
			//{
			//	resultIdx.insert( filmIdx );
			//}
			if( onlyYear )
			{
				if( scores > 0 )
					resultIdx.insert( filmIdx );
			}
			else if( scores >= bestScores )
			{
				if( scores > bestScores )
					resultIdx.clear();

				bestScores = scores;
				resultIdx.insert( filmIdx );
			}
		}
		else
		{
			if( scores > bestScores )
			{
				resultIdx.clear();
				resultIdx.insert( filmIdx );

				bestScores = scores;
				bestPenalty = penalty;
			}
			else if( scores == bestScores )
			{
				if( penalty < bestPenalty )
				{
					resultIdx.clear();
					resultIdx.insert( filmIdx );

					bestScores = scores;
					bestPenalty = penalty;
				}
			}
		}
	}

	if( resultIdx.empty() )
	{
		if( firstPass && PossibleTranslite( searchTitle ) )
		{
			CString transTitle = searchTitle;
			ToLower( transTitle );
			transTitle = FromTranslite( transTitle, true );

			year = SearchInfo( transTitle, resultIdx, fuzzy, draftSearch, false );
		}
	}

	return year;

	VC_CATCH( ... )
	{
		return 0;
	}
}

const KinopoiskInfo & Kinopoisk::SearchInfo( const CString & searchTitle, bool fuzzy )
{
	static KinopoiskInfo emptyInfo;

	VC_TRY;

	std::set<int> resultIdx;
	const int year = SearchInfo( searchTitle, resultIdx, fuzzy, true, false );

	if( resultIdx.empty() )
		return emptyInfo;

	if( resultIdx.size() == 1 || !year )
	{
		return _database->data[*resultIdx.begin()];
	}
	else
	{
		// т.к. сюда попадают фильмы с  +-1 годом, то можно ещё раз отфильтровать по году, но уже с точным совпадением.

		if( year != 0 )
		{
			for( const int & filmIdx : resultIdx )
			{
				const KinopoiskInfo & ki = _database->data[filmIdx];
				if( ki.year != year )
					continue;

				return ki;
			}
		}

		//  Если год не указан, то возвращаем первый попавшийся
		return _database->data[*resultIdx.begin()];
	}

	VC_CATCH( ... )
	{
		return emptyInfo;
	}
}

void Kinopoisk::SearchInfo( const CString & searchTitle, std::vector<KinopoiskInfo> & result )
{
	VC_TRY;

	std::set<int> resultIdx;
	SearchInfo( searchTitle, resultIdx, true, true, true );

	result.clear();

	if( resultIdx.empty() )
		return;

	result.reserve( resultIdx.size() );
	for( const int & filmIdx : resultIdx )
	{
		result.push_back( _database->data[filmIdx] );
	}

	VC_CATCH( ... )
	{	}
}


const KinopoiskInfo & Kinopoisk::SearchInfo( KinopoiskId id )
{
	static KinopoiskInfo emptyInfo;

	LoadDb();

	if( _database->data.empty() )
		return emptyInfo;

	KinopoiskInfo searchFor;
	searchFor.filmID = id;

	auto result = std::lower_bound( _database->data.begin(), _database->data.end(), searchFor, KinopoiskInfoByID() );
	if( result->filmID == id )
		return *result;

	return emptyInfo;
}

void SkipBlock( const std::wstring & unicode, size_t & pos )
{
	while( unicode[pos] != L'>' )
		++pos;

	++pos;
}

void ReadBlock( const std::wstring & unicode, size_t & pos, std::wstring & result )
{
	SkipBlock( unicode, pos );

	while( unicode[pos] != L'<' || unicode[pos + 1] == L'b' || unicode[pos + 1] == L'B' )
	{
		if( unicode[pos] == L'<' )
		{
			SkipBlock( unicode, pos );
			result += L"\n\r";
			continue;
		}

		result += unicode[pos];
		++pos;
	}

	CleanString( result );
}


void CleanString( std::wstring & line )
{
	static std::map< std::wstring, std::wstring> htmlSymbols;
	if( htmlSymbols.empty() )
	{
		htmlSymbols.emplace( std::wstring( L"&amp;" ), std::wstring( L"&" ) );
		htmlSymbols.emplace( std::wstring( L"&nbsp;" ), std::wstring( L" " ) );
		htmlSymbols.emplace( std::wstring( L"&#133;" ), std::wstring( L"..." ) );
		htmlSymbols.emplace( std::wstring( L"&#151;" ), std::wstring( L"-" ) );
		htmlSymbols.emplace( std::wstring( L"&Prime;" ), std::wstring( L"''" ) );
		htmlSymbols.emplace( std::wstring( L"&bdquo;" ), std::wstring( L"\"" ) );
		htmlSymbols.emplace( std::wstring( L"&gt;" ), std::wstring( L">" ) );
		htmlSymbols.emplace( std::wstring( L"&laquo;" ), std::wstring( L"\"" ) );
		htmlSymbols.emplace( std::wstring( L"&ldquo;" ), std::wstring( L"\"" ) );
		htmlSymbols.emplace( std::wstring( L"&lsquo;" ), std::wstring( L"`" ) );
		htmlSymbols.emplace( std::wstring( L"&lt;" ), std::wstring( L"<" ) );
		htmlSymbols.emplace( std::wstring( L"&prime;" ), std::wstring( L"'" ) );
		htmlSymbols.emplace( std::wstring( L"&quot;" ), std::wstring( L"\"" ) );
		htmlSymbols.emplace( std::wstring( L"&raquo;" ), std::wstring( L"\"" ) );
		htmlSymbols.emplace( std::wstring( L"&rdquo;" ), std::wstring( L"\"" ) );
	}

	for( const auto & item : htmlSymbols )
	{
		do
		{
			size_t pos = line.rfind( item.first );
			if( pos != line.npos )
				line.replace( pos, item.first.length(), item.second );
			else
				break;
		}
		while( true );
	}
}


CString ReadName( const std::wstring & unicode, size_t pos )
{
	CString result;
	while( pos < unicode.size() && unicode[pos] != L'<' )
	{
		result += unicode[pos];
		++pos;
	}

	return result.Trim();
}

void ParsePersonData( const std::string & htmlFile, std::vector<FilmId> & allFilms, Entry & entry )
{
	allFilms.clear();

	const auto errorText = L"Не удалось получить информацию о персоне";
	 
	{		
		//<script type="application/ld+json">
		//{
		//	"@context": "https://schema.org",
		//	"@type": "Person",
		//	"name": "Джет Ли",
		//	"alternateName": "Jet Li",
		//	"gender": "http://schema.org/Male",
		//	"jobTitle": ["Актер", "Продюсер", "Режиссер", "Сценарист"],
		//	"birthDate": "1963-04-26",
		//	"url": "/name/1511/",
		//	"image": "//avatars.mds.yandex.net/get-kinopoisk-image/1600647/c7d4ab53-357b-44cf-928b-e4eb6b99f0a2/360"
		//}
		//</script>		

		size_t pos = htmlFile.find( "application/ld+json" );
		if( pos == htmlFile.npos )
		{
			ShowError( errorText, false );
			return;
		}

		while( pos < htmlFile.size() && htmlFile[pos] != '>' )
		{
			++pos;
		}

		if( htmlFile[pos] != '>' )
		{
			ShowError( errorText, false );
			return;
		}

		++pos;

		Json::Value root;
		{
			Json::CharReaderBuilder rb;
			std::unique_ptr<Json::CharReader> reader( rb.newCharReader() );
			Json::String err;
			reader->parse( htmlFile.c_str() + pos, htmlFile.c_str() + htmlFile.size(), &root, &err );
		}

		if( root["name"].isString() )
		{
			entry.title = ToUnicode( root["name"].asString() );
		}

		if( root["alternateName"].isString() )
		{
			entry.titleAlt = ToUnicode( root["alternateName"].asString() );
		}
	}

	{
		size_t pos = htmlFile.find( "props" );
		if( pos == htmlFile.npos )
		{
			return;
		}
		while( pos > 0 && htmlFile[pos] != '{' )
		{
			--pos;
		}

		Json::Value root;
		{
			Json::CharReaderBuilder rb;
			std::unique_ptr<Json::CharReader> reader( rb.newCharReader() );
			Json::String err;
			reader->parse( htmlFile.c_str() + pos, htmlFile.c_str() + htmlFile.size(), &root, &err );
		}

		Json::Value propsObject = root["props"];
		if( !propsObject.isObject() )
			return;

		Json::Value apolloStateObject = propsObject["apolloState"];
		if( !apolloStateObject.isObject() )
			return;

		Json::Value dataObject = apolloStateObject["data"];
		if( !dataObject.isObject() )
			return;

		const Json::Value::Members childs = dataObject.getMemberNames();
		for( const auto & childName : childs )
		{
			if( childName.find( "Film:" ) == std::string::npos )
				continue;

			Json::Value object = dataObject[childName];
			if( !object.isObject() )
				continue;

			if( object["id"].isInt() )
			{
				allFilms.push_back( (FilmId)object["id"].asInt() );
			}
		}
	}
}

void Kinopoisk::DownloadPersonInfo( KinopoiskId id, std::vector<FilmId> & allFilms, Entry & entry )
{
#ifndef KP_UPDATER

	VC_TRY;

	Curl::Global curlGuard;

	const CString personUrl = ResFormat( L"http://webcache.googleusercontent.com/search?q=cache:https://www.kinopoisk.ru/name/%u/", id );
	
	Curl curl( GetGlobal().proxyTimeout );
	CurlBuffer chunk;

	CStringA curlQuery( personUrl );

	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

	curl.InitHeaders();
	curl.InitProxy( GetGlobal().proxyType, GetGlobal().GetProxy() );

	const CURLcode resCode = curl_easy_perform( curl );
	if( CURLE_OK != resCode )
		return;

	chunk.TerminateBuffer();

	//std::wstring unicode;

	//{
	//	char *ct = nullptr;
	//	curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &ct );
	//	::CharLowerBuffA( ct, strlen( ct ) );

	//	const bool isWindows1251 = ::strstr( ct, "windows-1251" ) != nullptr;

	//	const char * html = (const char *)chunk.GetData();
	//	if( !html )
	//		return;

	//	const size_t size = chunk.GetSize();
	//	unicode.resize( size * 4 );

	//	const int count = MultiByteToWideChar( isWindows1251 ? 1251 : CP_UTF8, 0, html, size + 1, std::data(unicode), unicode.size() );
	//	if( count <= 0 )
	//		return;

	//	unicode[count] = L'\0';
	//}

	entry.filmId = id;
	ParsePersonData( (const char*)chunk.GetData(), allFilms, entry );

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ShowError( L"Ошибка получения информации о человеке", ignore );
	}

#endif
}

void Kinopoisk::DownloadPersonInfoDirect( KinopoiskId id, std::vector<FilmId> & allFilms, Entry & entry )
{
#ifndef KP_UPDATER

	VC_TRY;

	Curl::Global curlGuard;

	const CString personUrl = ResFormat( L"http://webcache.googleusercontent.com/search?q=cache:https://www.kinopoisk.ru/name/%u/", id );
	
	CopyWebPageDlg copyDlg;
	copyDlg.startPageUrl = personUrl;

	if( copyDlg.DoModal() != IDOK )
		return;

	if( copyDlg.pageData.IsEmpty() )
		return;

	std::wstring unicode = copyDlg.pageData.GetString();
	std::string utf8;
	utf8.resize( unicode.size() * 4 );

	const int count = WideCharToMultiByte( CP_UTF8, 0, unicode.c_str(), unicode.length(), utf8.data(), utf8.size(), nullptr, nullptr );
	utf8[count]= '\0';

		//{
	//	const bool isWindows1251 = copyDlg.pageCharset.CompareNoCase( "windows-1251" ) == 0;

	//	const int length = copyDlg.pageData.GetLength();

	//	unicode.resize( length * 4 );

	//	const int count = MultiByteToWideChar( isWindows1251 ? 1251 : CP_UTF8, 0, copyDlg.pageData, length, &unicode[0], unicode.size() );
	//	if( count <= 0 )
	//		return;

	//	unicode[count] = L'\0';
	//}

	entry.filmId = id;
	ParsePersonData( utf8, allFilms, entry );

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ShowError( L"Ошибка получения информации о человеке", ignore );	
	}

#endif
}
