// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CollectionDB.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include "ErrorDlg.h"
#include "GlobalSettings.h"
#include "GuidTool.h"
#include "HashItem.h"
#include "ResString.h"
#include "poster.h"
#include "FileStore/std_list.h"
#include "SortPredicates.h"
#include "CollectionMaintenanceDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const CStringA COLLECTION_FILE_IDENTIFIER = "Ockham:VideoCat\n";
const CStringA PROGRESS_FILE_IDENTIFIER = "Ockham:Progress\n";

CollectionDB::CollectionDB()
	: _type( Local )
	, _multiUserAccess(false)
	, _nextVirtualHash(0)
	, _privateUnlocked(false)
{}

CollectionDB::~CollectionDB()
{}

PosterManager & CollectionDB::GetPosterManager()
{
	return _pm;
}

const PosterManager & CollectionDB::GetPosterManager() const
{
	return _pm;
}

bool CollectionDB::operator < ( const CollectionDB & rhs ) const
{
	return _name < rhs._name;
}

void CollectionDB::Create( const CString & name, DBType type )
{
	_guid = GetGuidString();
	_name = name;
	_type = type;
}

const CollectionDB::DBID & CollectionDB::GetId() const
{
	return _guid;
}

const CString & CollectionDB::GetName() const
{
	return _name;
}

void CollectionDB::SetName( const CString & name )
{
	_name = name;
}

bool CollectionDB::IsExcludedPath( const EntryHandle & rootHandle, const wchar_t * fullPath ) const
{
	auto ri = _rootInfo.find( rootHandle );
	if( ri == _rootInfo.end() )
		return false;

	CString lower = fullPath;
	ToLower( lower );

	auto ii = ri->second.excludedPath.find( lower );
	if( ii == ri->second.excludedPath.end() )
		return false;

	return true;
}

Entry * CollectionDB::FindEntry( const EntryHandle & handle )
{
	Entry entry;
	entry.thisEntry = handle;

	auto ii = _entries.find( entry );
	if( ii != _entries.end() )
		return const_cast<Entry*>( &(*ii) );

	return nullptr;
}

const Entry * CollectionDB::FindEntry( const EntryHandle & handle ) const
{
	Entry entry;
	entry.thisEntry = handle;

	auto ii = _entries.find( entry );
	if( ii != _entries.end() )
		return &(*ii);

	return nullptr;
}

EntryHandle CollectionDB::InsertRoot( const CString & rootName, const CString & rootDir )
{
	Entry newRootEntry;
	newRootEntry.title = rootName;
	newRootEntry.filePath = rootDir;
	Set( newRootEntry.flags, EntryTypes::IsFolder );
	newRootEntry.thisEntry.type = EntryHandle::IS_ROOT | EntryHandle::IS_FILE;
	newRootEntry.thisEntry.hash = GetNextVirtualIndex(); // отвязываем хэш корневой директории от пути, потмоу что на разных внешних дисках эти пути могут быть идентичны
	// HashingPath( rootDir.GetString(), rootDir.GetLength() );

	return InsertRoot( newRootEntry );
}

EntryHandle CollectionDB::InsertVirtualRoot( const CString & rootName )
{
	Entry newRootEntry;
	newRootEntry.title = rootName;
	Set( newRootEntry.flags, EntryTypes::IsFolder );
	newRootEntry.thisEntry.type = EntryHandle::IS_ROOT | EntryHandle::IS_VIRTUAL;
	newRootEntry.thisEntry.hash = GetNextVirtualIndex();

	return InsertRoot( newRootEntry );
}


EntryHandle CollectionDB::InsertRoot( const Entry & rootEntry )
{
	// запрещаем добавлять один корень дважды - не имеет смысла
	for( const EntryHandle & root : _roots )
	{
		if( root == rootEntry.thisEntry )
			return EntryHandle::EMPTY;
	}
		
	_entries.insert( rootEntry );
	_roots.push_back( rootEntry.thisEntry );

	ResortRoots();

	return rootEntry.thisEntry;
}

void CollectionDB::ResortRoots()
{
	typedef std::pair<CString, EntryHandle> RootName;

	std::vector<RootName> roots;
	for( const auto & r : _roots )
	{
		Entry * entry = FindEntry( r );
		if( entry )
		{
			roots.emplace_back( CString( entry->title.c_str() ), entry->thisEntry );
		}
	}

	std::sort( roots.begin(), roots.end() );

	_roots.clear();
	_roots.reserve( roots.size() );
	for( const RootName & r : roots )
	{
		_roots.push_back( r.second );
	}
}

Traverse::Result CollectionDB::TraverseAll( const EntryHandle & topHandle, Traverse::Base * func )
{
	Entry * top = FindEntry( topHandle );
	if( !top )
		return Traverse::CONTINUE;

	if( top->IsPrivate() && !_privateUnlocked )
		return Traverse::CONTINUE;

	if( !topHandle.IsRoot() )
	{
		Traverse::Result result = func->Run( *top );
		if( Traverse::CONTINUE != result )
			return result;

		if( !top->IsFolder() )
			return Traverse::CONTINUE;
	}

	for( const EntryHandle & eh : top->childs )
	{
		Traverse::Result result = TraverseAll( eh, func );
		if( result == Traverse::STOP )
			return result;
	}

	return Traverse::CONTINUE;
}

void CollectionDB::ForEach( Traverse::Base * func )
{
	for( const Entry & entry : _entries )
	{
		if( entry.IsPrivate() && !_privateUnlocked )
			continue;

		func->Run( (Entry &)entry );
	}
}

void CollectionDB::ForEach( Traverse::Base * func ) const
{
	for( const Entry & entry : _entries )
	{
		if( entry.IsPrivate() && !_privateUnlocked )
			continue;

		func->Run( (Entry &)entry );
	}
}

void CollectionDB::DeleteEntry( const EntryHandle & startHandle )
{
	Traverse::GetAll get;
	TraverseAll( startHandle, &get );

	PosterManager & pm = GetPosterManager();

	Entry * startEntry = FindEntry( startHandle );
	if( startEntry )
	{
		if( startEntry->posterId != DefaultPosterID )
		{
			pm.RemovePoster( startEntry->posterId );
		}

		Entry * parent = FindEntry( startEntry->parentEntry );
		if( parent )
			parent->RemoveChild( startHandle );

		_entries.erase( *startEntry );
		startEntry = nullptr; // сразу делаю инвалидным, во избежание проблем
	}

	Entry currentEntry;
	// удаляем корневой узел и все его дочерние узлы, включая их постеры
	for( const EntryHandle & h : get.allHandles )
	{
		currentEntry.thisEntry = h;

		Entry * entry = FindEntry( h );
		if( entry )
		{
			if( entry->posterId != DefaultPosterID )
			{
				pm.RemovePoster( entry->posterId );
			}
		}

		_entries.erase( currentEntry );
	}

	if( startHandle.IsRoot() )
	{
		// удаляем из списка корневых директорий
		auto ii = std::find( _roots.begin(), _roots.end(), startHandle );
		if( ii != _roots.end() )
			_roots.erase( ii );
	}
}

bool CollectionDB::InsertEntry( const Entry & entry, bool checkExist )
{
	if( checkExist )
	{
		Entry * ptr = FindEntry( entry.thisEntry );
		if( ptr )
			return false;
	}

	auto newEntry = _entries.insert( entry );
	const_cast<Entry&>(*newEntry.first).SetCurrentDate(); // сортировка только по дескриптору, поэтому безопасно

	return true;
}

void CollectionDB::RemoveEntry( const EntryHandle & handle )
{
	Entry * entry = FindEntry( handle );
	if( entry )
	{
		if( entry->posterId != DefaultPosterID )
		{
			PosterManager & pm = GetPosterManager();
			pm.RemovePoster( entry->posterId );
		}

		Entry * parent = FindEntry( entry->parentEntry );
		if( parent )
		{
			parent->RemoveChild( handle );
		}

		_entries.erase( *entry );
	}
}

// заменяем старый дескриптор на новый во всех объектах, использовавших старый дескриптор
struct ChangeHandle: public Traverse::Base
{
	EntryHandle oldHandle;
	EntryHandle newHandle;

	virtual Traverse::Result Run( Entry & entry ) override
	{
		for( EntryHandle & h : entry.childs )
		{
			if( h == oldHandle )
				h = newHandle;
		}

		if( entry.parentEntry == oldHandle )
			entry.parentEntry = newHandle;

		return Traverse::CONTINUE;
	}
};

EntryHandle CollectionDB::RenameFile( const EntryHandle & handle, const CString & newName )
{
	Entry * entry = FindEntry( handle );
	if( !entry )
		return handle;

	std::wstring rootPath;
	{
		Entry * rootEntry = FindEntry( entry->GetRootHandle() );
		if( !rootEntry )
			return handle;

		rootPath = rootEntry->filePath;
	}

	std::wstring oldPath = rootPath + entry->filePath;

	if( Test( entry->flags, EntryTypes::InnerFile ) )
	{
		// В случае вложенного файла - это папка, а папки заканчиваются на \. Удаляем последний символ, чтобы потом правильно имя вырезать
		oldPath.pop_back();
	}

	std::error_code errorCode;
	if( !std::filesystem::exists( oldPath, errorCode ) )
		return handle;

	std::wstring newPath;
	size_t idx = oldPath.rfind( L'\\' );
	if( idx != oldPath.npos )
	{
		newPath = oldPath.substr( 0, idx + 1 );
	}
	newPath += newName.GetString();

	
	if( Test( entry->flags, EntryTypes::InnerFile ) )
	{
		// В случае вложенного файла - это папка, а папки заканчиваются на \.
		newPath += L'\\';
	}
	else
	{
		std::filesystem::path path( oldPath );
		newPath += path.extension().c_str();
	}

	if( oldPath == newPath )
		return handle;
	
	// если файл с таким именем уже существует, то ничего делать не нужно - старые файлы при этом переименовывались и терялись
	if( std::filesystem::exists( newPath, errorCode ) )
		return handle;

	const EntryHandle oldHandle = handle;

	// добавляю префикс \\?\, чтобы можно было исопльзовать длиннющие пути
	std::filesystem::rename( oldPath, (L"\\\\?\\" + newPath) );

	// Т.к. записи отсортированы по дескриптору, то невозможно просто перезаписать дескриптор,
	// вместо этого придётся удалить старую запись и добавить новую, с новым дескриптором
	// Ссылки на этот дескриптор можно будет протсо перезаписать во всех других объектах
	Entry replaceEntry( *entry );
	replaceEntry.filePath = &newPath[rootPath.length()];
	replaceEntry.thisEntry.hash = replaceEntry.BuildHash();// HashingPath( replaceEntry.filePath.c_str(), replaceEntry.filePath.length() );
	EntryHandle newHandle = replaceEntry.thisEntry;

	{
		auto ii = _entries.find( *entry );
		if( ii != _entries.end() )
			_entries.erase( ii );

		entry = nullptr; // чтобы при случайной попытке использоваться было очень больно
	}
	_entries.insert( replaceEntry );

	// переименовываем файл с кадрами в соответствии с изменившемся дескриптором
	{
		const CString oldThumbName = thumbsDirectory + oldHandle.ToBase64() + L".jpg";
		const CString newThumbName = thumbsDirectory + newHandle.ToBase64() + L".jpg";

		std::error_code errorCode;
		if( std::filesystem::exists( oldThumbName.GetString(), errorCode ) )
			std::filesystem::rename( oldThumbName.GetString(), newThumbName.GetString() );
	}
	
	ChangeHandle change;
	change.oldHandle = oldHandle;
	change.newHandle = newHandle;

	ForEach( &change );

	// в списке последних просмотренных тоже заменяем
	for( EntryHandle & e : _recentlyViewed )
	{
		if( e == oldHandle )
		{
			e = newHandle;
			break;
		}
	}

	return newHandle; // возвращаем новый дескриптор
}

void CollectionDB::Load( const CString & fullPath, const CString & progressFullPath )
{	
	std::map<EntryHandle, EntryHandle> oldToNew;

	// загружаем коллекцию
	std::error_code errorCode;
	if( std::filesystem::exists( fullPath.GetString(), errorCode ) )
	{
		VC_TRY;

		std::ifstream fin( fullPath, std::ios_base::binary );
		FileStore::Reader read( fin );

		CStringA fileIdentifier;
		read & fileIdentifier;
		if( fileIdentifier != COLLECTION_FILE_IDENTIFIER )
			throw (int)-1;

		int classVersion( 0 );
		read & classVersion;

		read & _guid;
		read & _name;

		read & *this;

		wchar_t buffer[MAX_PATH + 1];
		wcscpy_s( buffer, fullPath );
		PathRemoveFileSpec( buffer );
		collectionDirectory.Format( L"%s\\", buffer );
		thumbsDirectory.Format( L"%s\\%s\\", buffer, _guid.GetString() );


#pragma omp parallel sections
		{
#pragma omp section
			{
				read & GetPosterManager();
			}
#pragma omp section
			{
				//// в первых версиях обнаружилась проблема, что добавляемые папки не были отсортированы и дописывались в конец списка, поэтому старые файлы надо пересортировать
				//// это делается параллельным потоком, чтобы не было замедления в работе
				//if( classVersion < 3 )
				//{
				//	for( EntryHandle & rh : _roots )
				//	{
				//		ResortCollection( rh );
				//	}
				//}

				// изменился алгоритм хэширования пути
				// теперь в начало каждого пути (кроме корня) добавляется id корневой директории, чтобы можно было в разных корневых папках делать одинаковую структуру папок.
				if( classVersion < 4 /*COLLECTION_VERSION*/ )
				{
					UpdateDbFormat( oldToNew );
				}
			}
		}
		
		VC_CATCH( ... )
		{
			ShowError( ResFormat( IDS_ERROR_COLLECTION_FORMAT_ERROR, fullPath.GetString() ), false );
		}
	}

	// загружаем прогресс просмотра, если он уже существует
	if( std::filesystem::exists( progressFullPath.GetString(), errorCode ) )
	{
		VC_TRY;

		std::ifstream fin( progressFullPath, std::ios_base::binary );
		FileStore::Reader read( fin );

		CStringA fileIdentifier;

		read & fileIdentifier;
		if( fileIdentifier != PROGRESS_FILE_IDENTIFIER )
			throw (int)-2;

		int classVersion( 0 );
		read & classVersion;

		DBID dbGuid;
		read & dbGuid; // guid коллекции, для которой сохранён прогресс
		if( dbGuid != _guid )
			throw (int)-3;

		// список последних просмотренных фильмов
		read & _recentlyViewed;

		// количество просмотров каждого фильма, если есть
		std::map<EntryHandle, unsigned int> numViews;
		read & numViews;

		if( !oldToNew.empty() )
		{
			// если было изменение идентификаторов, то нужн ообновить список просмотренного
			std::list<EntryHandle> newRecentlyViewed;

			for( EntryHandle & eh : _recentlyViewed )
			{
				auto ii = oldToNew.find( eh );
				if( ii != oldToNew.end() )
				{
					newRecentlyViewed.emplace_back( ii->second );
				}
			}
			_recentlyViewed.swap( newRecentlyViewed );

			std::map<EntryHandle, unsigned int> newNumViews;
			for( const auto & nv : numViews )
			{
				auto ii = oldToNew.find( nv.first );
				if( ii != oldToNew.end() )
				{
					newNumViews.emplace( ii->second, nv.second );
				}
			}
			numViews.swap( newNumViews );
		}
		
		for( auto & progress : numViews )
		{
			Entry * entry = FindEntry( progress.first );
			if( entry )
				entry->numViews = progress.second;
		}


		VC_CATCH( ... )
		{
			ShowError( ResFormat( IDS_ERROR_COLLECTION_FORMAT_ERROR, progressFullPath.GetString() ), false );
		}
	}
}

void CollectionDB::LoadGuid( const CString & fullPath, CollectionDB::DBID & id, CString & name )
{
	VC_TRY;

	id.Empty();
	name.Empty();

	std::error_code errorCode;
	if( !std::filesystem::exists( fullPath.GetString(), errorCode ) )
	{
		throw -1;
	}

	std::ifstream fin( fullPath, std::ios_base::binary );
	FileStore::Reader read( fin );

	CStringA fileIdentifier;
	read & fileIdentifier;
	if( fileIdentifier != COLLECTION_FILE_IDENTIFIER )
		throw (int)1;

	int classVersion( 0 );
	read & classVersion;

	read & id;
	read & name;

	VC_CATCH( ... )
	{
		ShowError( ResFormat(IDS_ERROR_COLLECTION_FORMAT_ERROR, fullPath.GetString()), false );
	}
}

void CollectionDB::Save( const CString & fullPath, const CString & progressFullPath ) const
{
	//сохраняем коллекцию
	if( !fullPath.IsEmpty() )
	{
		VC_TRY;

		const CString fullPathTemp( fullPath + L".tmp" );
		
		{
			std::ofstream fout( fullPathTemp, std::ios_base::binary );
			FileStore::Writer write( fout );

			// дублирую версию класса и GUID из объекта, чтбоы можно было делать быструю проверку файла, не загружая всю коллекцию
			write & COLLECTION_FILE_IDENTIFIER;
			write & (unsigned int)COLLECTION_VERSION;
			write & _guid;
			write & _name;

			write & *this;

			write & GetPosterManager();
		}// чтобы файл закрылся перед переименованием

		// удаляю старый файл, если он был
		std::error_code errorCode;
		if( std::filesystem::exists( fullPath.GetString(), errorCode ) )
			std::filesystem::remove( fullPath.GetString() );

		// переименовываю временный файл в файл коллекции
		std::filesystem::rename( fullPathTemp.GetString(), fullPath.GetString() );

		VC_CATCH( ... )
		{
			ShowError( ResFormat( IDS_ERROR_COLLECTION_SAVE_ERROR, _name.GetString(), fullPath.GetString() ), false );
		}
	}
		
	// Сохраняем прогресс просмотра и список последних просмотренных фильмов
	if( !progressFullPath.IsEmpty() )
	{
		VC_TRY;

		const CString progressFullPathTemp( progressFullPath + L".tmp" );

		{
			std::ofstream fout( progressFullPathTemp, std::ios_base::binary );
			FileStore::Writer write( fout );

			write & PROGRESS_FILE_IDENTIFIER;
			write & (unsigned int)PROGRESS_VERSION;
			write & _guid; // guid коллекции, для которой сохранён прогресс

			// список последних просмотренных фильмов
			write & _recentlyViewed;

			// количество просмотров каждого фильма, если есть
			struct CollectViewed : public Traverse::Base
			{
				std::map<EntryHandle, unsigned int> numViews;

				virtual Traverse::Result Run( Entry & entry ) override
				{
					if( entry.numViews > 0 )
						numViews.emplace( entry.thisEntry, entry.numViews );

					return Traverse::CONTINUE;
				}
			};

			CollectViewed collect;
			ForEach( &collect );

			write & collect.numViews;
		}// чтобы файл закрылся перед переименованием

		// удаляю старый файл, если он был
		std::error_code errorCode;
		if( std::filesystem::exists( progressFullPath.GetString(), errorCode ) )
			std::filesystem::remove( progressFullPath.GetString() );

		// переименовываю временный файл в файл коллекции
		std::filesystem::rename( progressFullPathTemp.GetString(), progressFullPath.GetString() );

		VC_CATCH( ... )
		{
			ShowError( ResFormat( IDS_ERROR_COLLECTION_SAVE_ERROR, _name.GetString(), progressFullPath.GetString() ), false );
		}
	}
}

void CollectionDB::AddPerson( const PersonInfo & personInfo )
{
	for( PersonInfo & a : _persons )
	{
		if( a.thisEntry == personInfo.thisEntry )
		{
			// Обновим данные, если персоны уже была добавлена
			a = personInfo;
			return;
		}
	}

	_persons.push_back( personInfo );
}

void CollectionDB::DeletePerson( const EntryHandle & handle )
{
	for( auto actor = _persons.begin(); actor != _persons.end(); ++actor )
	{
		if( actor->thisEntry == handle )
		{
			_persons.erase( actor );
			break;
		}
	}

	Entry * entry = FindEntry( handle );

	Entry * parent = FindEntry( entry->parentEntry );
	if( parent )
		parent->RemoveChild( handle );

	_entries.erase( *entry );
}

PersonInfo * CollectionDB::FindPerson( const EntryHandle & handle )
{
	for( PersonInfo & actor : _persons )
	{
		if( actor.thisEntry == handle )
			return &actor;
	}

	return nullptr;
}

void CollectionDB::AddViewedFile( const Entry & entry )
{
	EntryHandle handle = entry.thisEntry;

	if( Test( entry.flags, EntryTypes::TVepisode ) )
	{
		// для эпизода находим дескриптор папки сериала, даже если эпизод лежит в папке сезона
		Entry * parent = FindEntry( entry.parentEntry );
		if( parent )
		{
			if( Test( parent->flags, EntryTypes::TVseason ) )
			{
				handle = parent->parentEntry;
			}
			else
			{
				handle = parent->thisEntry;
			}
		}
	}

	// удаляем фильм из списка, если он в нём уже был
	auto it = std::find( _recentlyViewed.begin(), _recentlyViewed.end(), handle );
	if( it != _recentlyViewed.end() )
		_recentlyViewed.erase( it );

	// добавляем фильм в начало списка
	_recentlyViewed.push_front( handle );

	// если фильмов набралось больше 100, то удаляем последние
	while( _recentlyViewed.size() > 100 )
	{
		_recentlyViewed.pop_back();
	}
}

void CollectionDB::AddViewedFile( const EntryHandle & handle )
{
	_recentlyViewed.push_front( handle );
}

TagManager & CollectionDB::GetTagManager()
{
	return _tagManager;
}

const TagManager & CollectionDB::GetTagManager() const
{
	return _tagManager;
}

const CString & CollectionDB::GetPassword() const
{
	return _syncPassword;
}

void CollectionDB::SetPassword( const CString & password )
{
	_syncPassword = password;
}

const CString & CollectionDB::GetPrivatePassword() const
{
	return _privatePassword;
}

void CollectionDB::SetPrivatePassword( const CString & password )
{
	_privatePassword = password;
}

void CollectionDB::UnlockPrivate( const CString & password )
{
	if( _privatePassword == password )
		_privateUnlocked = true;
}

void CollectionDB::UpdatePersons()
{
	// собираем информацию обо всех фильмах id->Handle
	struct GetAll : public Traverse::Base
	{
		std::map<FilmId, std::set<EntryHandle> > allFilms;

		virtual Traverse::Result Run( Entry & entry ) override
		{
			// добавляем все и реальные и виртуальные фильмы
			if( !entry.thisEntry.IsPerson() && entry.filmId != NO_FILM_ID )
			{
				allFilms[entry.filmId].insert( entry.thisEntry );
			}

			return Traverse::CONTINUE;
		}
	};

	GetAll getAll;

	ForEach( &getAll );

	std::set<EntryHandle> usedRoots;

	// Для каждой персоны обновляем список имеющихся для неё фильмов
	for( const PersonInfo & info : _persons )
	{
		Entry * person = FindEntry( info.thisEntry );
		if( !person )
			continue;

		usedRoots.emplace( person->GetRootHandle() );

		// очищаем старый список фильмов
		person->childs.clear();

		for( FilmId id : info.allFilms )
		{
			auto ii = getAll.allFilms.find( id );
			if( ii == getAll.allFilms.end() )
				continue;

			for( const EntryHandle & h : ii->second )
			{
				person->AddChild( h );
			}
		}
	}

	for( const EntryHandle & h : usedRoots )
	{
		ResortCollection( h );
	}
}

unsigned int CollectionDB::GetNumFilms() const
{
	// персоны храняться отдельно от фильмов, поэтому тут их нет и проверять не требуется

	unsigned int count = 0;

	for( const Entry & entry : _entries )
	{
		if( !entry.IsFolder() )
		{
			if( !Test( entry.flags, EntryTypes::TVepisode ) )
				++count; // считаем фильмы
		}
		else
		{
			if( Test( entry.flags, EntryTypes::TVseries ) )
				++count; // Считаем сериал целиком за один фильм. Сезоны и эпизоды пропускаются
		}
	}

	return count;
}

void CollectionDB::RemoveEmptyFolders( const EntryHandle & rootHandle )
{
	bool hasRemovedFolders = false;

	do
	{
		hasRemovedFolders = false;

		for( EntryList::iterator ii = _entries.begin(); ii != _entries.end(); )
		{
			if( ii->rootEntry != rootHandle )
			{
				++ii;
				continue;
			}

			if( !ii->thisEntry.IsRoot() && ii->IsFolder() && ii->childs.empty() )
			{
				if( ii->posterId != DefaultPosterID )
				{
					GetPosterManager().RemovePoster( ii->posterId );
				}

				Entry * parent = FindEntry( ii->parentEntry );
				if( parent )
					parent->RemoveChild( ii->thisEntry );

				ii = _entries.erase( ii );
				hasRemovedFolders = true;
			}
			else
			{
				++ii;
			}
		}
	}
	while( hasRemovedFolders );
}

void CollectionDB::PostprocessInnerFiles( const EntryHandle & rootHandle )
{
	const CollectionDB::RootInfo & rootInfo = GetRootInfo( rootHandle );

	for(const auto & e : _entries)
	{
		if( e.childs.size() == 1 )
		{
			// сортировка происходит только по дескриптору, поэтому менять флаги безопасно
			Entry & entry = const_cast<Entry &>(e);
			if( rootInfo.innerFilesMode )
			{
				Clear( entry.flags, EntryTypes::IsFolder );
				Set( entry.flags, EntryTypes::InnerFile );

				Entry * child = FindEntry( e.childs[0] );
				if( child )
				{
					Set( child->flags, EntryTypes::InnerFile );
				}
			}
			else
			{
				Clear( entry.flags, EntryTypes::InnerFile );
				Set( entry.flags, EntryTypes::IsFolder );
				Entry * child = FindEntry( e.childs[0] );
				if( child )
				{
					Clear( child->flags, EntryTypes::InnerFile );
				}
			}
		}
		else if( Test( e.flags, EntryTypes::InnerFile ) && e.childs.size() > 1 )
		{
			Entry & entry = const_cast<Entry &>(e);
			Clear( entry.flags, EntryTypes::InnerFile );
			Set( entry.flags, EntryTypes::IsFolder );
		}
	}
}

class TestFolderTitle
{
public:
	TestFolderTitle( CollectionDB * d )
		: db( d )
	{
	}

public:
	CollectionDB * db;

public:
	bool operator()( const EntryHandle &lhs, const EntryHandle & rhs ) const
	{
		const Entry * eLhs = db->FindEntry( lhs );
		const Entry * eRhs = db->FindEntry( rhs );
		if( !eLhs || !eRhs )
			return false;

		// т.к. отсортируются и файлы, то сперва проверяем, чтобы папки были вначале. Это ускорит и сортировку при отображении в списке фильмов
		const bool lhsFolder = eLhs->IsFolder();
		const bool rhsFolder = eRhs->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		int r = CompareNoCase( eLhs->title, eRhs->title );
		if( r < 0 )
			return true;
		if( r > 0 )
			return false;

		r = CompareNoCase( eLhs->titleAlt, eRhs->titleAlt );

		return r < 0;
	}
};

void FolderSort( TestFolderTitle & tft, std::vector<EntryHandle> & childs )
{
	if( childs.size() > 1 )
		std::sort( childs.begin(), childs.end(), tft );

	for( EntryHandle & e : childs )
	{
		Entry * entry = tft.db->FindEntry( e );
		if( entry )
		{
			FolderSort( tft, entry->childs );
		}
	}
}

void CollectionDB::ResortCollection( const EntryHandle & rootHandle )
{
	TestFolderTitle tft( this );

	Entry * root = FindEntry( rootHandle );
	if( !root )
		return;

	FolderSort( tft, root->childs );
}

bool CollectionDB::Maintenance( const CString & thumbDir )
{
	CWaitCursor waiting;

	CollectionMaintenanceDlg dlg;
	dlg.Create( IDD_MAINTENANCE, AfxGetApp()->GetMainWnd() );
	dlg.ShowWindow( SW_SHOW );

	std::set<EntryHandle> allEntries;

	dlg.AddString( L"Поиск ошибочных записей..." );
	const int numBadEntries = CleanBadEntry( allEntries );
	if( numBadEntries > 0 )
	{
		dlg.AddString( ResFormat( L"  Удалено ошибочных записей: %i", numBadEntries ) );
	}
	else
	{
		dlg.AddString( L"  Ошибок не найдено" );
	}

	dlg.AddString( L"Поиск устаревших постеров..." );
	const int numBadPosters = CleanBadPoster();
	if( numBadPosters > 0 )
	{
		dlg.AddString( ResFormat( L"  Удалено постеров: %i", numBadPosters ) );
	}
	else
	{
		dlg.AddString( L"  Ошибок не найдено" );
	}

	dlg.AddString( L"Поиск устаревших кадров..." );
	const int numBadFrames = CleanBadFrames( allEntries, thumbDir );
	if( numBadFrames > 0 )
	{
		dlg.AddString( ResFormat( L"  Удалено кадров: %i", numBadFrames ) );
	}
	else
	{
		dlg.AddString( L"  Ошибок не найдено" );
	}

	dlg.AddString( L" " );

	dlg.AddString( L"Обслуживание коллекции завершено" );

	AfxMessageBox( L"Обслуживание коллекции завершено", MB_OK );

	dlg.DestroyWindow();

	return (numBadEntries + numBadPosters) > 0;
}

void CollectionDB::UpdateDbFormat( std::map<EntryHandle, EntryHandle> & oldToNew )
{
	CWaitCursor waiting;

	;

	EntryList newEntryList;

	// сперва нужно конвертировать идентификаторы корневых директорий
	for( EntryHandle & root : _roots )
	{
		if( !root.IsReal() )
		{
			oldToNew.emplace( root, root ); // тоже добавляю, чтобы потом не проверять, изменился ли идентификатор
			continue;
		}

		EntryHandle newRoot( root );
		newRoot.hash = GetNextVirtualIndex();
		oldToNew.emplace( root, newRoot );

		root = newRoot;
	}

	RootInfoList newRootInfo;
	for( const auto & ri : _rootInfo )
	{
		newRootInfo.emplace( oldToNew[ri.first], ri.second );
	}
	_rootInfo.swap( newRootInfo ); // подменяем на обновлённые данные

	// делать придётся в два прохода
	// 1) обновить корневые директоии и собственный идентификатор каждой записи
	// 2) обновить идентификатор родительской папки и идентификаторы дочерних объектов

	// 1
	for( const Entry & entry : _entries )
	{
		if( !entry.thisEntry.IsReal() )
		{
			// для виртуаьлных хэш не зависит от пути
			newEntryList.insert( entry );
			continue;
		}
		else if( entry.thisEntry.IsRoot() )
		{
			// для корневых директорий убираю привязку к пути до корня

			Entry newEntry( entry );
			newEntry.thisEntry = oldToNew[entry.thisEntry];

			newEntryList.insert( newEntry );
			continue;
		}

		Entry newEntry( entry );

		// В первую очередь изменяю идентификатор корневой директории
		newEntry.rootEntry = oldToNew[entry.rootEntry];
		newEntry.thisEntry.hash = newEntry.BuildHash();

		oldToNew.emplace( entry.thisEntry, newEntry.thisEntry );

		newEntryList.insert( newEntry );

		// переименовываем файл с кадрами в соответствии с изменившемся дескриптором
		{
			const CString oldThumbName = thumbsDirectory + entry.thisEntry.ToString() + L".jpg";
			const CString newThumbName = thumbsDirectory + newEntry.thisEntry.ToBase64() + L".jpg";

			std::error_code errorCode;
			if( std::filesystem::exists( oldThumbName.GetString(), errorCode ) )
				std::filesystem::rename( oldThumbName.GetString(), newThumbName.GetString() );
		}

	}

	// 2
	for( const Entry & cEntry : newEntryList )
	{
		if( !cEntry.thisEntry.IsReal() )
		{
			continue;
		}

		// ЗАПРЕЩЕНО ИЗМЕНЯТЬ thisEntry !!!
		Entry & entry = const_cast<Entry &>(cEntry);

		if( !entry.thisEntry.IsRoot() )
		{
			entry.parentEntry = oldToNew[entry.parentEntry];
		}

		for( EntryHandle & child : entry.childs )
		{
			child = oldToNew[child];
		}

	}

	_entries.swap( newEntryList );

	g_forceInvalidateDatabase = true;
}
