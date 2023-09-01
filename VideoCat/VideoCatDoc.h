#pragma once

#include <set>
#include <vector>
#include <list>

#include "CollectionDB.h"
#include "DisplayItem.h"
#include "EntryHandle.h"
#include "SortPredicates.h"

class CVideoCatView;
class CVideoTreeView;

typedef std::vector< DisplayItem > FilteredFileItems;


class CVideoCatDoc : public CDocument
{
protected:
	static const UINT DOCUMENT_VERSION = 4;
	static const UINT USER_INFO_VERSION = 4;

	CVideoCatDoc();
	DECLARE_DYNCREATE( CVideoCatDoc )
public:
	~CVideoCatDoc() override;

public:
	struct CollectionInfo
	{
		CollectionDB::DBID id; // guid коллекции
		CString name; // текстовое название коллекции для списка
		CString fullPath; // путь к файлу коллекции

		bool operator < ( const CollectionInfo & rhs ) const
		{
			return name < rhs.name;
		}
	};

public:

	const CollectionDB::DBID & GetTutorialID() const;
	
	void CreateCollection( const CString & title, const CString & path );
	void EditCollection( const CollectionInfo & newInfo );
	void AttachCollection( const CString & title, const CString & path );
	void DetachCollection( const CollectionDB::DBID & id );

	// загрузить коллекцию
	void ReloadCollection( const CollectionDB::DBID & id );
	// Выгрузить из памяти коллекцию и все связанные с ней данные
	void UnloadCurrentCollection();
	// пересохранить текущую коллекцию
	void SaveCurrentCollection();

	void SaveCollectionList() const;
	void LoadCollectionList();
	void LoadCollectionList_Old();

	inline CollectionDB * GetCurrentCollection()
	{
		return _currentCollection.get();
	}

	CollectionInfo * FindCollectionInfo( const CollectionDB::DBID & id );

	inline unsigned GetNumCollections() const
	{
		return static_cast<unsigned>(_collections.size());
	}

	inline const CollectionInfo * GetCollectionInfo( unsigned index ) const
	{
		if( !_collections.empty() )
			return &_collections.at( index );
		else
			return nullptr;
	}

	const CString & GetThumbDir() const
	{
		return _currentCollection->thumbsDirectory;
	}

	BOOL Ready() const
	{
		return static_cast<BOOL>(_currentCollection != nullptr);
	}

	FilteredFileItems & GetFilteredFiles()
	{
		return _filteredFileItems;
	}

	const CString & GetCurrentCatFolder() const
	{
		return _currentCatFolder;
	}

	std::wstring GetMoviePath( const Entry & entry ) const;


	void BuildRandomMovies();
	void BuildRecentlyMovies();
	void BuildNewMovies();

	inline bool HasRecentlyMovies() const
	{
		if( _currentCollection.get() )
			return _currentCollection->HasRecentlyViewed();

		return false;
	}

	void RebuildFilteredFiles( Entry * info, Entry * oldFi = nullptr );
	void RebuildFilteredFiles( const CString & namePart, BOOL extraFiltering );
	void RebuildFilteredFiles( Genres genres, BOOL strictCheck );
	void RebuildFilteredFiles( std::set<TagId> & tags, BOOL strictCheck );

	BOOL IsFilterDirectory() const;
	BOOL IsFilterName() const;
	BOOL IsFilterGenre() const;

	CVideoCatView * GetVideoCatView();
	CVideoTreeView * GetVideoTreeView();

	void InvalidateDatabase()
	{
		_needToSaveDatabase = TRUE;
	}

	void InvalidateProgressInfo()
	{
		_needToSaveProgressInfo = TRUE;
	}

	void SetSortPredicate( sort_predicates::SortType type );
	DisplayItemComparator GetCurrentPredicate() const;

	void AddViewedFile( Entry & entry );

	void AddPerson( const Entry & person, const PersonInfo & personInfo );
	PersonInfo * FindPerson( const EntryHandle & handle );
	void DeletePerson( const EntryHandle & handle );

protected:
	bool FindFilesByName( const CString & searchString, BOOL extraFiltering );
	void FindFilesByYear( const CString & searchString, BOOL extraFiltering );

public:
	virtual BOOL OnNewDocument();
	virtual void Serialize( CArchive& );
	virtual void OnCloseDocument();

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump( CDumpContext& dc ) const override;
#endif

protected:
	DECLARE_MESSAGE_MAP()

private:

	std::vector<CollectionInfo> _collections; // список подключенных баз данных с фильмами
	std::unique_ptr<CollectionDB> _currentCollection; // активная коллекция фильмов

	CollectionDB::DBID _lastUsedCollection; // идентификатор последней использованной коллекции

	FilteredFileItems _filteredFileItems; ///< Список фильмов/папок, отображаемых для выбранной директории или в соответствии с фильторм или поиском
	CString _currentCatFolder;
	CString _searchNamePart;

	enum FilterType
	{
		FileDirectory, // фильмы в папке
		FileName,	// фильмы по имени
		FileGenre,	// фильмы по жанру
		FileRandom,	// случайные фильмы
		FileTags, // фильмы по тэгу
		FileRecent, // последние просмотренные фильмы
		FilterType_Force32 = 0x7FFFFFFF
	};

	FilterType _filterType;

	mutable BOOL _needToSaveDatabase;
	mutable BOOL _needToSaveProgressInfo;

	DisplayItemComparator _predicate;
};
