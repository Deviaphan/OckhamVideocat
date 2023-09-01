#pragma once

#include <vector>
#include <set>
#include <list>
#include "Entry.h"
#include "PersonInfo.h"
#include "Traverse/TraverseCollection.h"
#include "FileStore/access.h"
#include "FileStore/femstore.h"
#include "FileStore/std_vector.h"
#include "FileStore/std_set.h"
#include "CStringStore.h"
#include "TagManager.h"
#include "poster.h"


class CollectionDB
{
public:
	using DBID = CString;
	using DBType = unsigned int;
	enum
	{
		COLLECTION_VERSION = 4, // Номер версии класса коллекции
		PROGRESS_VERSION = 1, // Номер версии файла прогресса

		VideoCatDirectory = 0, // Корневая директория задана относительно папки с каталогизатором
		Local = 1,	// Обычная локальная база данных
		Extern = 2,	// Внешняя запароленная к изменениям база данных. Может находиться в сетевой папке и использоваться несколькими компьютерами
	};

	using ExcludedPath = std::set<CString>;

	class RootInfo
	{
	public:
		RootInfo()
			: innerFilesMode( FALSE )
			, searchHidden( FALSE )
		{}

	public:
		ExcludedPath excludedPath;	// Папки, исключённые из синхронизации. Для каждой корневой директории индивидуально.

		BOOL innerFilesMode;	// Каждый фильм обязан лежать в отдельной папке
		BOOL searchHidden;	// искать в скрытых и системных папках

	private:
		friend class FileStore::Access;
		template<class Store>
		void Storage( Store & ar, unsigned /*version*/ )
		{
			ar & excludedPath;

			ar & innerFilesMode;
			ar & searchHidden;
		}
	};


public:
	CollectionDB();
	~CollectionDB();

public:
	void Create( const CString & name, DBType type );
	const DBID & GetId() const;
	const CString & GetName() const;
	void SetName( const CString & name );

	PosterManager & GetPosterManager();
	const PosterManager & GetPosterManager() const;

	void Load( const CString & collectionFullPath, const CString & progressFullPath );
	void Save( const CString & collectionFullPath, const CString & progressFullPath ) const;
	static void LoadGuid( const CString & collectionFullPath, DBID & id, CString & name );

	inline unsigned int GetNumRoots() const
	{
		return static_cast<unsigned int>(_roots.size());
	}

	EntryHandle GetRoot( unsigned int index ) const
	{
		return _roots.at( index );
	}

	RootInfo & CreateRootInfo( const EntryHandle & rootHandle )
	{
		return _rootInfo[rootHandle];
	}

	RootInfo & GetRootInfo( const EntryHandle & rootHandle )
	{
		auto ri = _rootInfo.find( rootHandle );
		if( ri != _rootInfo.end() )
			return ri->second;

		throw std::exception();
		__assume(0);
	}

	EntryHandle InsertRoot( const CString & rootName, const CString & rootDir );
	EntryHandle InsertVirtualRoot( const CString & rootName );

	// добавить корневую директорию
	EntryHandle InsertRoot( const Entry & rootEntry );
	// удалить корневую директорию и все объекты, находящиеся в этой директории
	void DeleteEntry( const EntryHandle & startHandle );
	void ResortRoots();

	PathHashType GetNextVirtualIndex()
	{
		return ++_nextVirtualHash;
	}

	Entry * FindEntry( const EntryHandle & handle);
	const Entry * FindEntry( const EntryHandle & handle ) const;
	bool InsertEntry( const Entry & entry, bool checkExist = true );
	void RemoveEntry( const EntryHandle & handle );
	void RemoveEmptyFolders( const EntryHandle & rootHandle );
	void PostprocessInnerFiles( const EntryHandle & rootHandle );

	void ResortCollection( const EntryHandle & rootHandle );

	EntryHandle RenameFile( const EntryHandle & handle, const CString & newName );

	// Есть ли путь в списке исключённых
	bool IsExcludedPath( const EntryHandle & rootHandle, const wchar_t * fullPath ) const;

	// обход в глубину всех элементов, начиная с top
	Traverse::Result TraverseAll( const EntryHandle & topHandle, Traverse::Base * func );

	// выполнить функцию для каждого элемента в коллекции
	void ForEach( Traverse::Base * func );
	void ForEach( Traverse::Base * func ) const;

	void AddPerson( const PersonInfo & actor );
	void DeletePerson( const EntryHandle & handle );
	PersonInfo * FindPerson( const EntryHandle & handle );

	BOOL SearchHiddenFiles( const EntryHandle & rootHandle ) const
	{
		auto ii = _rootInfo.find( rootHandle );
		if( ii != _rootInfo.end() )
			return ii->second.searchHidden;

		return FALSE;
	}

	bool HasRecentlyViewed() const
	{
		return !_recentlyViewed.empty();
	}

	const std::list<EntryHandle> & GetRecentlyViewed() const
	{
		return _recentlyViewed;
	}

	void AddViewedFile( const Entry & entry );
	void AddViewedFile( const EntryHandle & handle );

	TagManager & GetTagManager();
	const TagManager & GetTagManager() const;

	const CString & GetPassword() const;
	void SetPassword( const CString & password );

	const CString & GetPrivatePassword() const;
	void SetPrivatePassword( const CString & password );
	void UnlockPrivate( const CString & password );
	bool PrivateLocked() const
	{
		return _privateUnlocked == false;
	}

	void SetMultiUserMode( bool use )
	{
		_multiUserAccess = use;
	}

	bool GetMultiUserMode() const
	{
		return _multiUserAccess;
	}

	void UpdatePersons();

	inline unsigned int GetNumEntries() const
	{
		return static_cast<unsigned int>(_entries.size());
	}

	inline unsigned int GetNumPersones() const
	{
		return static_cast<unsigned int>(_persons.size());
	}

	unsigned int GetNumFilms() const;

	// Выполнить обслуживание коллекции
	bool Maintenance( const CString & thumbDir );

private:
	int CleanBadEntry( std::set<EntryHandle> & allEntries );

	int CleanBadPoster();

	int CleanBadFrames( const std::set<EntryHandle> & allEntries, const CString & thumbDir );

	void UpdateDbFormat( std::map<EntryHandle, EntryHandle> & oldToNew );

public:
	bool operator < ( const CollectionDB & rhs ) const;

private:
	DBID _guid;	// Уникальный идентификатор базы данных (GUID)
	CString _name;	// Название базы данных
	DBType _type;	// Тип базы данных

	CString _syncPassword;	// пароль на изменение БД
	CString _privatePassword; // пароль на доступ к приватным папкам в этой коллекции
	bool _multiUserAccess; // блокирование базы данных при внесении изменений, для безопасной работы с несколькими пользователями

	using RootInfoList = std::map<EntryHandle, RootInfo>;
	using RootList = std::vector<EntryHandle>;
	using EntryList = std::set<Entry>;

	RootInfoList _rootInfo;
	RootList _roots;	// список корневых директорий этой базы данных
	EntryList _entries;	// все записи этой базы данных
	PathHashType _nextVirtualHash;

	std::vector<PersonInfo> _persons; // Описание сохранённых персон

	std::list<EntryHandle> _recentlyViewed; ///< 100 последних просмотренных фильмов (сохраняется в отдельный файл)

	TagManager _tagManager;

	PosterManager _pm;

	bool _privateUnlocked;

public:
	CString collectionDirectory; // НЕ СОХРАНЯЕТСЯ
	CString thumbsDirectory;	// НЕ СОХРАНЯЕТСЯ

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & _type;
		ar & _syncPassword;
		ar & _privatePassword;
		ar & _multiUserAccess;

		ar & _rootInfo;
		ar & _roots;
		ar & _entries;
		ar & _nextVirtualHash;

		ar & _persons;

		ar & _tagManager;

	}
};

FILESTORE_VERSION( CollectionDB, CollectionDB::COLLECTION_VERSION );
