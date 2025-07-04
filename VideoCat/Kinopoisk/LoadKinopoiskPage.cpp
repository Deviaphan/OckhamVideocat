﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "LoadKinopoiskPage.h"
#include <map>
#include <string>
#include "Net/CurlBuffer.h"
#ifndef KP_UPDATER
#include "../GlobalSettings.h"
#include "../CopyWebPageDlg.h"
#endif
#include "Kinopoisk.h"
#include "Cyrillic.h"
#include <string_view>
#include "json/json.h"

void LoadPageData( const CString& startUrl, std::vector<char>& utf8 )
{
	CopyWebPageDlg copyDlg;
	copyDlg.startPageUrl = startUrl;

	if( copyDlg.DoModal() != IDOK )
		return;

	if( copyDlg.pageData.IsEmpty() )
		return;

	CWaitCursor wait;

	utf8.resize( copyDlg.pageData.GetLength() * 3 );
	int utf8Size = WideCharToMultiByte( CP_UTF8, 0, copyDlg.pageData.GetString(), copyDlg.pageData.GetLength(), std::data( utf8 ), (int)utf8.size(), nullptr, nullptr );
	utf8[utf8Size] = '\0';
}

bool LoadKinopoiskInfoDirect( KinopoiskId id, KinopoiskInfo & info )
{
	VC_TRY;
	
	CString filmUrl;

	if( id != 0 )
		filmUrl.Format( L"https://www.kinopoisk.ru/film/%u/", id );
	else
		filmUrl = L"https://www.kinopoisk.ru/";

	::std::vector<char> utf8;
	LoadPageData( filmUrl, utf8 );
	if( utf8.empty() )
		return false;

	return ProcessFilmJson( (KinopoiskId)id, utf8.data(), utf8.data() + utf8.size(), info );

	VC_CATCH( ... )
	{
		return false;
	}
}

Genres ImportKinopoiskGenres( const std::wstring & line )
{
	Genres genres = EmptyGenres;

	for( int i = (int)Genre::FirstGenre; i < (int)Genre::NumOfGenres; ++i )
	{
		const std::wstring g = (Descript( (Genre)i ).GetString());
		//if( line.find( Genre::Descript( (Genre::Enum)i ).GetString() ) != std::wstring::npos )
		if( CompareNoCase( line, g ) == 0 )
		{
			Set( genres, (Genre)i );
			return genres;
		}
	}

	return genres;
}

unsigned char ImportKinopoiskRating( const std::wstring & line )
{
	if( !iswdigit( line[0] ) )
		return 0;

	double value = _wtof( line.c_str() );
	value *= 10.0;
	return (unsigned char)((unsigned int)value);
}

bool ProcessFilmJson( KinopoiskId id, const char * startPtr, const char * endPtr, KinopoiskInfo & info )
{
	const std::string_view jsonStart( startPtr );
	std::size_t pos = jsonStart.find( "application/ld+json" );
	if( pos == std::string_view::npos )
		return false;

	std::size_t taglinePos = jsonStart.find( "tagline" );

	const char * ptr = startPtr + pos;
	while( (ptr < endPtr) && (*ptr != '>') )
		++ptr;

	if( ptr == endPtr )
		return false;

	++ptr;

	Json::Value data;
	{
		Json::CharReaderBuilder rb;
		std::unique_ptr<Json::CharReader> reader( rb.newCharReader() );
		Json::String err;
		reader->parse( ptr, endPtr, &data, &err );
	}

	if( data["url"].isString() )
	{
		const std::string url = data["url"].asString();
		// в конце url находится идентификатор фильма
		auto idPos = url.length() - 1;
		if( !isdigit( url[idPos] ) )
			--idPos;
		while( isdigit( url[idPos] ) )
		{
			--idPos;
		}
		++idPos;

		info.filmID = std::atoi( url.c_str() + idPos );
	}
	else
	{
		info.filmID = id;
	}

	if( data["name"].isString() )
		info.nameRU = ToUnicode( data["name"].asString() );

	if( data["alternateName"].isString() )
		info.nameEN = ToUnicode( data["alternateName"].asString() );

	if( info.nameRU.empty() && info.nameEN.empty() )
		return false;

	//if( data["slogan"].isString() )
	//	info.tagline = ToUnicode( data["slogan"].asString() );

	if( data["description"].isString() )
		info.description = ToUnicode( data["description"].asString() );

	if( data["datePublished"].isString() )
	{
		const std::string y = data["datePublished"].asString();
		if( !y.empty() )
		{
			try
			{
				info.year = (unsigned short)std::stoi( y );
			}
			catch( ... )
			{
				;
			}
		}
	}

	if( data["genre"].isArray() )
	{
		Genres ImportKinopoiskGenres( const std::wstring & line ); //fwd

		Genres genres = 0;

		Json::Value genreArray = data["genre"];
		const int numItems = genreArray.size();
		for( int i = 0; i < numItems; ++i )
		{
			if( genreArray[i].isString() )
			{
				const std::string s = genreArray[i].asString();
				if( s.empty() )
					continue;

				std::wstring ss = ToUnicode( s );

				genres |= ImportKinopoiskGenres( ss );
			}
		}

		info.genres = genres;
	}

	if( data["aggregateRating"].isObject() )
	{
		Json::Value ratingObject = data["aggregateRating"];
		if( ratingObject["ratingValue"].isDouble() )
		{
			const double d = ratingObject["ratingValue"].asDouble();
			info.rating = (unsigned char)floor(d * 10.0 + 0.5);
		}
	}

	if( taglinePos != std::string_view::npos )
	{
		//"tagline":"The brawl to end it all",

		const char* ptr = startPtr + taglinePos;
		while( *ptr != ':' )
			++ptr;

		// надо проверить на "tagline":null"
		if( ptr[0] == ':' && ptr[1] == 'n' && ptr[2] == 'u' && ptr[3] == 'l' )
		{
			;
		}
		else
		{
			ptr += 2; // пропуск двоеточия и кавычек

			std::string tagline;

			while( *ptr != '"' )
			{
				switch( *ptr )
				{
					case '\\':
						tagline += '\\';
						++ptr;
						tagline += *ptr;
						break;

					default:
						tagline += *ptr;
						break;
				}

				++ptr;
			}

			if( !tagline.empty() )
				info.tagline = ToUnicode( tagline );
		}
	}

	return true;
}

bool ProcessFilm( KinopoiskId id, const std::wstring & unicode, KinopoiskInfo & info )
{
	// КАК ОКАЗАЛОСЬ
	// IExplore возвращает код переформатированных, т.е. не исходный код страницы, а сам из своего дерева код воссоздаёт
	// поэтому порядок тэгов в строке может отличаться от тогО, что видно в исходном коде в Фаерфоксе

	// RU EN YEAR
	// <meta property="og:title" content="«Стражи Галактики. Часть&amp;nbsp;2» (Guardians of the Galaxy Vol. 2, 2017)">

	// Кинопоиск сильно поменяли разметку. Теперь год придётся получать отсюда
	// Ру-Ен названия тоже со временем придётся брать сдесь. Пока ещё правильно работает поиск по странице

	{
		// год

		// <meta property="og:title" content="«Стражи Галактики. Часть&amp;nbsp;2» (Guardians of the Galaxy Vol. 2, 2017)">
		std::wstring yearBase( L"og:title" );
		size_t pos = unicode.find( yearBase, 0 );
		if( pos == unicode.npos )
		{
			info.year = 0;
		}
		else
		{
			// og:title может быть в конце строки мета-тега, а не перед названием, поэтому искать придётся не до вхождения этого тэга, но и левее
			// Возможно появление ошибок...
			size_t startPos = 1000;

			SkipBlock( unicode, pos ); // перематываем в конец блока
			
			// от конца блока перематываем до первой цифры
			while( pos > startPos )
			{
				--pos;
				
				if( ::iswdigit( unicode[pos] ) )
					break;
			}

			if( ::iswdigit( unicode[pos] ) )
			{
				pos -= 3; // сдвигаем на начало 4х значного года

				info.year = (unsigned short)_wtoi( &unicode[pos] );
			}
		}
	}

	{
		// русское название
		//  <h1 class="moviename-big" itemprop="name"><span class="moviename-title-wrapper">Матрица</span></h1>
		std::wstring rusTitleBase1( L"\"moviename-title-wrapper\"" );
		std::wstring rusTitleBase2( L"\"moviename-big\"" );
		size_t pos = unicode.find( rusTitleBase1 );
		if( pos != unicode.npos )
		{
			pos += rusTitleBase1.length();
			ReadBlock( unicode, pos, info.nameRU );

			std::wstring isSeries;
			ReadBlock( unicode, pos, isSeries );
			if( isSeries.find( L"сериал" ) != isSeries.npos )
				info.nameRU += L" (сериал)";
		}
		else
		{
			size_t pos = unicode.find( rusTitleBase2 );
			if( pos != unicode.npos )
			{
				pos += rusTitleBase2.length();
				ReadBlock( unicode, pos, info.nameRU );

				std::wstring isSeries;
				ReadBlock( unicode, pos, isSeries );
				if( isSeries.find( L"сериал" ) != isSeries.npos )
					info.nameRU += L" (сериал)";
			}
		}
	}
	{
		// оригинальное название
		// <span itemprop="alternativeHeadline">Singin' in the Rain</span>
		std::wstring altTitleBase( L"\"alternativeHeadline\"" );
		size_t pos = unicode.find( altTitleBase );
		if( pos != unicode.npos )
		{
			pos += altTitleBase.length();
			ReadBlock( unicode, pos, info.nameEN );
		}

	}

	if( info.nameRU.empty() && info.nameEN.empty() )
	{
		AfxMessageBox( L"Не удалось получить данные о фильме.\r\nВозможно, IP адрес компьютера временно забанен.\r\nПопробуйте обновить данные позже.", MB_OK );
		return false;
	}

	{
		// слоган
		// <tr><td class="type">слоган</td><td style="color: #555">«This summer, the galaxy won't save itself»</td></tr>
		std::wstring taglineBase( L"слоган" );
		size_t pos = unicode.find( taglineBase );
		if( pos != unicode.npos )
		{
			SkipBlock( unicode, pos ); // skip  слоган</td>
			ReadBlock( unicode, pos, info.tagline );

			// в тэгах бывал мусор, поэтому нужно отфильтровать
			size_t npos = info.tagline.find( L"original" );
			if( npos != std::wstring::npos )
			{
				while( npos > 1 )
				{
					if( info.tagline[npos] == L'(' )
						break;
					--npos;
				}
				if( npos > 1 )
				{
					info.tagline.erase( npos, std::wstring::npos );
				}
			}

			if( info.tagline.size() < 10 )
				info.tagline.clear(); // скорее всего, это ошибка и мусор попал
		}
	}
	{
		// описание
		//  <div class="brand_words film-synopsys" itemprop="description">Все в &nbsp;сборе: землянин Питер Квилл( Звездный Лорд ), молчаливый громила

		std::wstring descriptionBase( L"film-synopsys" );
		size_t pos = unicode.find( descriptionBase );
		if( pos != unicode.npos )
		{
			ReadBlock( unicode, pos, info.description ); // skip block

			if( info.description.size() < 100 )
				info.description.clear();  // скорее всего, это ошибка и мусор попал
		}
	}

	{
		// жанры
		/*
		<span itemprop="genre">
		<a href="https://www.kinopoisk.ru/lists/navigator/sci-fi/?quick_filters=films" data-popup-info="disabled">фантастика</a>,
		<a href="https://www.kinopoisk.ru/lists/navigator/action/?quick_filters=films">боевик</a>,
		<a href="https://www.kinopoisk.ru/lists/navigator/adventure/?quick_filters=films">приключения</a>,
		<a href="https://www.kinopoisk.ru/lists/navigator/comedy/?quick_filters=films">комедия</a></span>,
		<a href="https://www.kinopoisk.ru/film/841263/keywords/">...</a>
		<a class="wordLinks" href="https://www.kinopoisk.ru/film/841263/keywords/">слова</a>
		*/

		std::wstring genreBase( L"genre" );

		size_t pos = unicode.find( genreBase, 0 );
		if( pos != unicode.npos )
		{
			SkipBlock( unicode, pos ); // <span itemprop="genre">
			
			do 
			{
				std::wstring genreName;
				ReadBlock( unicode, pos, genreName ); // SkipBlock( unicode, pos ); // Skip есть в начале ReadBlock // <a href="https://www.kinopoisk.ru/lists/navigator

				SkipBlock( unicode, pos ); // </a>,

				if( genreName.empty() || !::iswalpha(genreName[0]) )
					break;

				Genres oldGenre = info.genres;
				info.genres |= ImportKinopoiskGenres( genreName );

				if( oldGenre == info.genres )
					break; // на случай, если что-то пойдёт совсем не так, чтобы не получилось вечного цикла
			}
			while( pos != unicode.npos );
		}

	}

	{
		// рейтинг
		// <span class="rating_ball">7.517</span>
		std::wstring ratingBase( L"\"rating_ball\"" );
		size_t pos = unicode.find( ratingBase );
		if( pos != unicode.npos )
		{
			SkipBlock( unicode, pos );			
			const wchar_t * ptrValue = &unicode[pos];
			double value = _wtof( ptrValue );
			info.rating = (unsigned char)(value * 10.0);
		}
	}

	info.filmID = id;

	return true;
}
