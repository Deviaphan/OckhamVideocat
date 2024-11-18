// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "LoadSerialEpisodes.h"
#include "Net/CurlBuffer.h"
#include "../GlobalSettings.h"
#include "../CopyWebPageDlg.h"
#include "Kinopoisk.h"


void LoadSerialEpisodes( KinopoiskId serialID, SeriesDescription & description )
{
	//curl_global_init( CURL_GLOBAL_ALL );
	//Curl curl( GetGlobal().proxyTimeout );
	//CurlBuffer chunk;
	//CStringA curlQuery;

	//curlQuery.Format( "http://webcache.googleusercontent.com/search?q=cache:https://www.kinopoisk.ru/film/%d/episodes/", serialID );
	////curlQuery.Format( "https://www.kinopoisk.ru/film/%u/episodes/", serialID );

	//VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
	//VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	//VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

	//curl.InitHeaders();
	//curl.InitProxy(GetGlobal().proxyType, GetGlobal().GetProxy() );

	//if( CURLE_OK != curl_easy_perform( curl ) )
	//	return;

	//chunk.TerminateBuffer();

	//char *ct = nullptr;
	//curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &ct );
	//bool isWindows1251 = ::strstr( ct, "WINDOWS-1251" ) != nullptr;

	//const char * ptr = (char*)chunk.GetData();
	//if( !ptr )
	//	return;

	//std::wstring unicode;
	//unicode.resize( chunk.GetSize() * 2 );

	//int count = MultiByteToWideChar( isWindows1251 ? 1251 : CP_UTF8, 0, ptr, chunk.GetSize() + 1, &unicode[0], unicode.size() );
	//unicode[count] = L'\0';

	CopyWebPageDlg copyDlg;
	copyDlg.startPageUrl.Format( L"https://www.kinopoisk.ru/film/%u/episodes/", serialID );
	if( copyDlg.DoModal() != IDOK )
		return;

	CWaitCursor wait;

	std::wstring unicode = copyDlg.pageData.GetString();
	//{
	//	const bool isWindows1251 = copyDlg.pageCharset.CompareNoCase( "windows-1251" ) == 0;

	//	const int length = copyDlg.pageData.GetLength();

	//	unicode.resize( length * 4 );

	//	const int count = MultiByteToWideChar( isWindows1251 ? 1251 : CP_UTF8, 0, copyDlg.pageData, length, &unicode[0], unicode.size() );
	//	if( count <= 0 )
	//		return;

	//	unicode[count] = L'\0';
	//}

	std::vector<size_t> seasonOffsets;

	size_t startPos = 0;

	CString seasonName;
	int seasonNumber = 1;

	do
	{
		seasonName.Format( L"\"s%d\"", seasonNumber );
		size_t pos = unicode.find( seasonName, startPos );
		if( pos == unicode.npos )
			break;

		startPos = pos;
		++seasonNumber;
		seasonOffsets.push_back( pos );
	}
	while( true );

	seasonOffsets.push_back( unicode.npos );

	for( unsigned int seasonIndex = 0; seasonIndex < (unsigned int)(seasonOffsets.size() - 1); ++seasonIndex )
	{
		size_t pos = seasonOffsets[seasonIndex];
		const size_t endPos = seasonOffsets[seasonIndex + 1];

		do
		{
			std::wstring episodeNumberBase( L">Эпизод " );
			pos = unicode.find( episodeNumberBase, pos );
			if( pos >= endPos )
				break;

			if( pos == unicode.npos )
				break;

			pos += episodeNumberBase.length();

			int episode = _wtoi( &unicode[pos] );

			std::wstring nameRU;
			std::wstring nameEN;

			SkipBlock( unicode, pos );
			SkipBlock( unicode, pos );
			SkipBlock( unicode, pos );
			ReadBlock( unicode, pos, nameRU );
			if( !nameRU.empty() && nameRU[0] == L'\r' || nameRU[0] == L'\n' )
				nameRU.clear();

			SkipBlock( unicode, pos );
			SkipBlock( unicode, pos );
			ReadBlock( unicode, pos, nameEN );

			if( pos >= endPos )
				continue;

			if( !nameEN.empty() && nameEN[0] == L'\r' || nameEN[0] == L'\n' )
				nameEN.clear();

			if( nameRU.empty() && nameEN.empty() )
				break;

			Episode & desc = description[seasonIndex+1][episode];
			desc.nameRU = nameRU.c_str();
			desc.nameEN = nameEN.c_str();
			desc.episode = episode;
		}
		while( true );
	}
}


