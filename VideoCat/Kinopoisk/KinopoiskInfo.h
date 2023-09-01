#pragma once
#include <string>
#include <utility>
#include "../FileStore/femstore.h"
#include "../FileStore/version.h"
#include "../FileStore/std_string.h"
#include "../genres.h"

using KinopoiskId = unsigned int;

enum: unsigned int { NO_KINOPOISK_ID = 0 };

class KinopoiskInfo
{
public:
	KinopoiskInfo()
		: genres( EmptyGenres )
		, filmID( NO_KINOPOISK_ID )
		, year( 0 )
		, rating( 0 )
	{}

public:
	std::wstring nameEN;
	std::wstring nameRU;
	std::wstring tagline;
	std::wstring description;

	Genres genres;

	KinopoiskId filmID;

	unsigned short year;
	unsigned char rating;

private:
	friend class FileStore::Access;
	template< class Store>
	void Storage( Store & ar, unsigned version )
	{
		ar & filmID;
		ar & genres;
		ar & year;
		ar & rating;
		ar & nameEN;
		ar & nameRU;
		ar & tagline;
		ar & description;
	}
};

struct KinopoiskInfoByID
{
	bool operator()( const KinopoiskInfo & lhs, const KinopoiskInfo & rhs ) const
	{
		return lhs.filmID < rhs.filmID;
	}
};

FILESTORE_VERSION( KinopoiskInfo, 1 );
