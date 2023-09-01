// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "FilmDataBase.h"

#include <fstream>
#include <algorithm>
#include "hash/MurmurHash3.h"
#include <omp.h>
#include "../FileStore/femstore.h"
#include "../FileStore/std_vector.h"
#include "../GlobalSettings.h"

bool g_delimTable[255];

struct InitDelimiterTable
{
	InitDelimiterTable()
	{
		ZeroMemory( g_delimTable, sizeof(g_delimTable) );

		//const wchar_t tokenDelim[] = L" ,-_()[].:;~&#!'`+\"«»%/@?$=<>";

		// Перед добавлением граничного символа убедиться, что код символа меьнше 256

		g_delimTable[0] = true;
		g_delimTable[L' '] = true;
		g_delimTable[L','] = true;
		g_delimTable[L'-'] = true;
		g_delimTable[L'_'] = true;
		g_delimTable[L'('] = true;
		g_delimTable[L')'] = true;
		g_delimTable[L'['] = true;
		g_delimTable[L']'] = true;
		g_delimTable[L'.'] = true;
		g_delimTable[L':'] = true;
		g_delimTable[L';'] = true;
		g_delimTable[L'~'] = true;
		g_delimTable[L'&'] = true;
		g_delimTable[L'#'] = true;
		g_delimTable[L'!'] = true;
		g_delimTable[L'\''] = true;
		g_delimTable[L'`'] = true;
		g_delimTable[L'+'] = true;
		g_delimTable[L'"'] = true;
		g_delimTable[L'«'] = true;
		g_delimTable[L'»'] = true;
		g_delimTable[L'%'] = true;
		g_delimTable[L'/'] = true;
		g_delimTable[L'@'] = true;
		g_delimTable[L'?'] = true;
		g_delimTable[L'$'] = true;
		g_delimTable[L'='] = true;
		g_delimTable[L'<'] = true;
		g_delimTable[L'>'] = true;
	}
};

InitDelimiterTable idt;

bool IsDelimiter( wchar_t c )
{
	if( c >= sizeof( g_delimTable ) )
		return false;

	return g_delimTable[c];
}

bool IgnoredToken( const wchar_t * token )
{
	// все однобуквенные русские и двухбуквенные не русские уже отфильтрованы
	static const wchar_t ignoredTokens[][5] = { L"на", L"из", L"от", L"по", L"the", L"and", L"are", L"iii"/*I-II-III*/ };

	for( const wchar_t * i : ignoredTokens )
	{
		if( wcscmp( token, i ) == 0 )
			return true;
	}

	return false;
}

void FilmDb::LoadFromFile()
{
#ifndef KP_UPDATER

	std::ifstream rf( GetGlobal().GetFilmsDatabase(), std::ios_base::in | std::ios_base::binary );
	if( !rf.is_open() )
		return;

	FileStore::Reader r( rf, false );
	r & data;

#endif
}

FilmDb::HashType FilmDb::HashWord( const wchar_t * oneWord, unsigned size )
{
	const FilmDb::HashType h = MurmurHash3_x86_32( oneWord, size, 0xc2b2ae35 );
	return (h ^ (h >> 16)) & 0xFFFF;
}

const FilmDb::Indices & FilmDb::GetFilms( HashType hash ) const
{
	return _hash[hash];
}

unsigned Tokenize( const std::wstring & str, unsigned & pos, wchar_t * result )
{
	// пропускаем не буквы и не цифры
	while( str[pos] != L'\0' && IsDelimiter( str[pos] ) )
		++pos;

	unsigned tokenSize = 0;

	// размер слова ограничен 63 символами
	while( (tokenSize < 63) && str[pos] != L'\0' && !IsDelimiter( str[pos] ) )
	{
		*result = str[pos];
		if( *result == L'ё' || *result == L'Ё' )
			*result = L'е';

		++result;
		++pos;
		++tokenSize;
	}

	*result = 0;

	return tokenSize;
}


void FilmDb::BuildHash()
{
	// Не размещаю в базе данных информацию о номерах фильмов, чтобы было меньше коллизий
	// Не размещаю одно-двух буквенные слова на английском (+ the and) и однобуквенные на русском (+ на из от), они не несут смысла обычно и лишь создают путаницу

	if( data.empty() )
		return;
	
	//DWORD st = GetTickCount();

	const int size = data.size();
	
	_hash.resize( 65536 );

#pragma omp parallel for 
	for( int i = 0; i < size; ++i )
	{
		wchar_t token[64];//не обнуляю, чтобы не тратить время зря
		token[63] = L'\0';

		const KinopoiskInfo & ki = data[i];

		std::wstring title = ki.nameRU;
		::CharLowerBuffW( title.data(), title.size() );

		// разбиваем на токены русское название и хэшируем
		unsigned start = 0;
		while( true )
		{
			unsigned tokenSize = Tokenize( title, start, token );
			if( !token[0] )
				break;

			if( tokenSize == 1 || IgnoredToken(token) )
				continue;

			const HashType hash = HashWord( token, tokenSize );
			_hash[hash].push_back( i );
		};

		title = ki.nameEN;
		::CharLowerBuffW( title.data(), title.size() );

		// разбиваем на токены английское название и хэшируем
		start = 0;
		while( true )
		{
			unsigned tokenSize = Tokenize( title, start, token );
			if( !token[0] )
				break;

			// артикли выкидываем
			if( (tokenSize <= 2 ) || IgnoredToken(token) )
				continue;

			const HashType hash = HashWord( token, tokenSize );
			_hash[hash].push_back( i );
		};

	}

	//DWORD et = GetTickCount();
	//CString str;
	//str.Format( L"Time: %g sec", (et - st) / 1000.0 );
	//AfxMessageBox( str );
}
