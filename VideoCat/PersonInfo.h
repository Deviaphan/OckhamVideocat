#pragma once

#include <vector>
#include "EntryHandle.h"
#include "Entry.h"

#include "FileStore/access.h"
#include "FileStore/std_vector.h"

class CollectionDB;
class CVideoCatDoc;

class PersonInfo
{
public:
	PersonInfo()
	{}

public:
	bool operator < ( const PersonInfo & rhs ) const
	{
		return thisEntry < rhs.thisEntry;
	}

public:
	EntryHandle thisEntry;	
	std::vector<FilmId> allFilms; // идентификаторы фильмов, в которых снялся актёр, даже если фильмов сейчас нет в БД

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & thisEntry;
		ar & allFilms;	
	}
};

struct PersonSearchInfo
{
	FilmId id = NO_FILM_ID;
	BOOL directLoad = FALSE;
};

PersonSearchInfo GetPersonId( FilmId personId, std::vector<FilmId> * allFilms );

EntryHandle AddPersonEntry( CVideoCatDoc & doc, CollectionDB & cdb, const EntryHandle & parentHandle );

bool UpdatePersonEntry( CollectionDB & cdb, const EntryHandle & personHandle );
