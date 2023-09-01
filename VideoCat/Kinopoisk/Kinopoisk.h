#pragma once

#include <vector>
#include <set>
#include "KinopoiskInfo.h"
#include "FilmDataBase.h"
#include "../Entry.h"

class Kinopoisk
{
private:
	Kinopoisk();
	friend Kinopoisk & GetKinopoisk();

public:
	~Kinopoisk();

public:
	// Найти описание фильма по названию файла с фильмом
	const KinopoiskInfo & SearchInfo( const CString & searchTitle, bool fuzzy );

	void SearchInfo( const CString & searchTitle, std::vector<KinopoiskInfo> & result );

	// Найти описание фильма по идентификатору
	const KinopoiskInfo & SearchInfo( KinopoiskId id );

	// получить информацию о фильмах для актёра
	static void DownloadPersonInfo( KinopoiskId id, std::vector<FilmId> & allFilms, Entry & entry );
	static void DownloadPersonInfoDirect( KinopoiskId id, std::vector<FilmId> & allFilms, Entry & entry );

	// Открыть описание фильм на сайте cinemmate.cc
	static void OpenSite( KinopoiskId id );
	static void OpenSimilarSite( KinopoiskId id );

private:
	int SearchInfo( const CString & searchTitle, std::set<int> & resultIdx, bool fuzzy, bool firstPass, bool draftSearch );

	void LoadDb();

private:
	FilmDb * _database;
};

Kinopoisk & GetKinopoisk();

bool IsSerial( const KinopoiskInfo & info );
bool IsSerial( const wchar_t * fileName );


void SkipBlock( const std::wstring & unicode, size_t & pos );
void ReadBlock( const std::wstring & unicode, size_t & pos, std::wstring & result );
void CleanString( std::wstring & line );
