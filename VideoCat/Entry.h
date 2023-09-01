#pragma once
#include "EntryHandle.h"

#include "SharedBase/EntryAdditionalData.h"
#include "SharedBase/EntryBase.h"

#include <vector>
#include "Kinopoisk/KinopoiskInfo.h"
#include "poster.h"
#include "TagManager.h"
#include "FileStore/access.h"
#include "FileStore/std_vector.h"
#include "CStringStore.h"
#include "PluginManager.h"
#include "SortPredicates.h"

class CollectionDB;

using EntryType = unsigned int;

enum class EntryTypes: unsigned int
{
	Empty = 0,
	IsFolder = 1u,	// объект является папкой, а не файлом
	DvdArchive = 1u << 1u,	// Объект является папкой с копией DVD диска, в котором содержатся файлы *.IFO, *.VOB и прочие. Флаг устанавливается на папку, чтобы рассматривать её как фильм
	IsFilm3D = 1u << 2u,	// фильм сохранён в 3D формате (между анаглифом и стереопарой разницы не делаю)
	InnerFile = 1u << 3u, // Вложеннный в папку файл. Флаг установлен, если файлы лежат по одному в своих папках
	Private = 1u << 4u,	// Папка, защищённая паролем
	HomeVideo = 1u << 5u,	// Папка с личными видеозаписями, для которых не нужно искать описания и постеры. Папка исключается из автозаполнения

	TVseries = 1u << 10u, // сериал
	TVseason = 1u << 11u, // папка с сезоном сериала
	TVepisode = 1u << 12u,	// Эпизод сериала
};

inline EntryType GetValue( const EntryTypes entryType ) noexcept
{
	return static_cast<EntryType>(entryType);
}

inline void Set( EntryType & flags, const EntryTypes entryType ) noexcept
{
	flags |= GetValue( entryType );
}

inline void Clear( EntryType & flags, const EntryTypes entryType ) noexcept
{
	flags &= ~GetValue( entryType );
}

inline bool Test( const EntryType & flags, const EntryTypes entryType ) noexcept
{
	if( entryType != EntryTypes::Empty )
		return (flags & GetValue( entryType )) != 0;

	return false;
}



namespace FileStore
{
	template<class Archive>
	void Storage( Archive & store, EntryAdditionalData & ead, unsigned /*version*/ )
	{
		store & ead.id;
		store & ead.data;
	}
}

extern bool g_forceInvalidateDatabase;

class Entry : public EntryBase
{
public:
	std::vector<EntryHandle> childs;	// идентификаторы дочерних объектов, если объект является папкой
	EntryHandle thisEntry;	// идентификатор этого объекта
	EntryHandle parentEntry;	// идентификатор родительского объекта
	EntryHandle rootEntry;	// Идентификатор корневой директории

	EntryType flags;	// Флаги, описывающие тип записи
	unsigned int numViews;	// количество просмотров фильма (хранится отдельно от основной базы, локально для каждого пользователя)

	PosterID posterId; // дескриптор постера

	std::vector< std::pair<std::wstring, std::wstring> > techInfo;	// техническая информация о фильме, если была получена
	std::vector<TagId> tags;	// назначенные пользователем метки

	FILETIME date; // дата добавления фильма
	
	PluginID pluginID; // Идентификатор плагина, при помощи которого заполнена карточка

	sort_predicates::SortType sortType; // переназначенный режим сортировки для дочерних узлов папки

public:
	explicit Entry( EntryHandle entryHash = EntryHandle() )
		: thisEntry( entryHash )
		, flags( (EntryType)EntryTypes::Empty )
		, numViews( 0 )
		, posterId( DefaultPosterID )
		, pluginID( PluginNone )
		, sortType( sort_predicates::SortDefault )
		, date{ 0, 0 }
	{
	}

public:
	bool operator < ( const Entry & rhs ) const
	{
		return thisEntry < rhs.thisEntry;
	}

public:
	inline bool IsFolder() const
	{
		return Test( flags, EntryTypes::IsFolder );
	}
	inline bool IsFile() const
	{
		return !Test( flags, EntryTypes::IsFolder ) && thisEntry.IsReal();
	}

	inline bool IsPerson() const
	{
		return thisEntry.IsPerson();
	}

	inline bool IsTV() const
	{
		return Test( flags, EntryTypes::TVseries ) || Test( flags, EntryTypes::TVseason ) || Test( flags, EntryTypes::TVepisode );
	}

	inline bool IsPrivate() const
	{
		return Test( flags, EntryTypes::Private );
	}

	inline bool IsFilm3D() const
	{
		return Test( flags, EntryTypes::IsFilm3D );
	}

	// Задать сезон и эпизод
	bool SetEpisode( unsigned int season, unsigned int episode )
	{
		season &= 0xFF;
		episode &= 0xFFFF;

		const unsigned int value = seriesData;
		seriesData = (season << 16) | episode;

		return seriesData != value;
	}

	unsigned int GetSeason() const
	{
		return seriesData >> 16;
	}

	unsigned int GetEpisode() const
	{
		return seriesData & 0xFFFF;
	}

	// Скопировать поля, которые мог перезадать пользователь, чтобы не потерять их после синхронизации
	void CopyUserData( const Entry & rhs );

	EntryHandle GetRootHandle() const
	{
		if( rootEntry != EntryHandle::EMPTY )
			return rootEntry;
		return thisEntry;
	}

	std::wstring GetMoviePath( const std::wstring & rootDir, CollectionDB & db ) const;


	void AddChild( const EntryHandle & handle );
	void RemoveChild( const EntryHandle & h );

	void SetCurrentDate();

	// созать хэш из хэша корня и пути к файлу, от корневой директории
	PathHashType BuildHash() const;

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned version )
	{
		ar & thisEntry;
		ar & parentEntry;
		ar & rootEntry;
		ar & childs;

		ar & title;
		ar & titleAlt;
		ar & tagline;
		ar & description;
		ar & comment;
		ar & urlLink;

		ar & flags;
		ar & genres;
		ar & filmId;
		ar & year;
		ar & rating;

		ar & filePath;
		ar & posterId;
		ar & seriesData;

		ar & techInfo;
		ar & tags;

		ar & additional;

		if( version > 1 )
		{
			ar & date.dwLowDateTime;
			ar & date.dwHighDateTime;

			ar & pluginID;
		}
		else
		{
			// для старых версий ставим время обновления коллекции
			if( Store::isReader )
			{
				g_forceInvalidateDatabase = true;
				SetCurrentDate();

				if( filmId != NO_FILM_ID )
				{
					pluginID = PluginKinopoisk;
				}
			}
		}

		if( version >= 3 )
			ar & sortType;
	}
};

FILESTORE_VERSION( Entry, 3 );

inline bool LessDate( const FILETIME & t1, const FILETIME & t2 )
{
	if( t1.dwHighDateTime < t2.dwHighDateTime )
		return true;
	if( t1.dwHighDateTime > t2.dwHighDateTime )
		return false;

	return t1.dwLowDateTime < t2.dwLowDateTime;
}

struct EntryLess
{
	bool operator()( const Entry & lhs, const Entry & rhs ) const
	{
		return lhs.thisEntry < rhs.thisEntry;
	}
};


bool AfterEditEqual( const Entry & lhs, const Entry & rhs );

DisplayItemComparator GetEntryPredicate( DisplayItemComparator current, CollectionDB * db, const Entry & entry );
