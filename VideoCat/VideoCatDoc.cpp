// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "VideoCatDoc.h"
#include "VideoCat.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <filesystem>
#include <fstream>

#include "CStringStore.h"
#include "CardTechInfo.h"
#include "ErrorDlg.h"
#include "FileStore/femstore.h"
#include "FileStore/std_list.h"
#include "FileStore/std_map.h"
#include "GlobalSettings.h"
#include "HashItem.h"
#include "Kinopoisk/FilmDataBase.h"
#include "Kinopoisk/LoadFilmInfo.h"
#include "ManageCollectionsDlg.h"
#include "ProgramSettingsDlg.h"
#include "ResString.h"
#include "SynchronizeFiles.h"
#include "TestPassword.h"
#include "ThumbGenerator.h"
#include "Traverse/FindByGenres.h"
#include "Traverse/FindByName.h"
#include "Traverse/FindByTags.h"
#include "Traverse/FindByYear.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "Commands/CommandInfo.h"
#include "Commands/CommandExecute.h"
#include "FirstRunDlg.h"
#include "json/json.h"
#include "Cyrillic.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CollectionDB::DBID TUTORIAL_GUID( L"21B3AAF2-D04B-4914-9CC9-9F14289AD4B7" );

//static const wchar_t registryName[] = L"Software\\maaGames\\VideoCat";
static const wchar_t predicateName[] = L"predicate";


IMPLEMENT_DYNCREATE(CVideoCatDoc, CDocument)

BEGIN_MESSAGE_MAP(CVideoCatDoc, CDocument)
END_MESSAGE_MAP()



CVideoCatDoc::CVideoCatDoc()
	: _needToSaveDatabase(FALSE)
	, _needToSaveProgressInfo(FALSE)
	, _filterType( FilterType::FileDirectory )
	, _predicate(nullptr)
{
	std::ios_base::sync_with_stdio( true );
}

CVideoCatDoc::~CVideoCatDoc()
{
}

const CollectionDB::DBID & CVideoCatDoc::GetTutorialID() const
{
	return TUTORIAL_GUID;
}

void CVideoCatDoc::CreateCollection( const CString & title, const CString & path )
{
	VC_TRY;

	CString newCollectionFolder = path;
	if( newCollectionFolder == L"." )
	{
		newCollectionFolder = GetGlobal().GetProgramDirectory();
	}

	if( newCollectionFolder.GetAt( newCollectionFolder.GetLength() - 1 ) != L'\\' )
		newCollectionFolder += L'\\';
	
	CollectionDB db;
	db.Create( title, CollectionDB::Local );
	//db.SetPassword( dlg.password );

	CString fullPath = newCollectionFolder + db.GetId() + L".ockham";
	CString progressPath = GetGlobal().GetDefaultDirectory() + db.GetId() + L".progress";
	db.Save( fullPath, progressPath );

	CollectionInfo collectionInfo;
	collectionInfo.id = db.GetId();
	collectionInfo.name = db.GetName();
	collectionInfo.fullPath = fullPath;

	_collections.push_back( collectionInfo );
	std::sort( _collections.begin(), _collections.end() );

	SaveCollectionList();

	ReloadCollection( collectionInfo.id );

	GetVideoTreeView()->UpdateCollectionList();

	GetVideoTreeView()->RebuildTree();

	VC_CATCH_ALL;
}

void CVideoCatDoc::EditCollection( const CollectionInfo & newInfo )
{
	VC_TRY;

	CVideoCatDoc::CollectionInfo * info = FindCollectionInfo( newInfo.id );
	if( !info )
		return;

	info->name = newInfo.name;

	SaveCollectionList();

	ReloadCollection( newInfo.id );

	GetVideoTreeView()->UpdateCollectionList();

	GetVideoTreeView()->RebuildTree();

	VC_CATCH_ALL;
}

void CVideoCatDoc::AttachCollection( const CString & title, const CString & path )
{
	VC_TRY;

	std::error_code errorCode;
	if( !std::filesystem::exists( std::filesystem::path( path.GetString() ), errorCode ) )
		return;

	CollectionInfo collectionInfo;

	CollectionDB::LoadGuid( path, collectionInfo.id, collectionInfo.name );

	if( !title.IsEmpty() )
		collectionInfo.name = title;

	collectionInfo.fullPath = path;

	_collections.push_back( collectionInfo );
	std::sort( _collections.begin(), _collections.end() );

	SaveCollectionList();

	ReloadCollection( collectionInfo.id );

	GetVideoTreeView()->UpdateCollectionList();

	GetVideoTreeView()->RebuildTree();

	VC_CATCH_ALL;
}

void CVideoCatDoc::DetachCollection( const CollectionDB::DBID & id )
{
	VC_TRY;

	if( id == GetTutorialID() )
		return;

	bool loadNextCollection = false;

	if( GetCurrentCollection()->GetId() == id )
	{
		UnloadCurrentCollection();
		loadNextCollection = true;
	}

	for( auto ii = _collections.begin(); ii != _collections.end(); ++ii )
	{
		// проверку пароля, когда верну пароль

		if( ii->id == id )
		{
			_collections.erase( ii );
			break;
		}
	}

	SaveCollectionList();

	if( loadNextCollection )
	{
		const CVideoCatDoc::CollectionInfo * info = GetCollectionInfo( 0 );
		if( info )
		{
			ReloadCollection( info->id );
		}
	}

	GetVideoTreeView()->UpdateCollectionList();
	
	GetVideoTreeView()->RebuildTree();	

	VC_CATCH_ALL;
}

void CVideoCatDoc::ReloadCollection( const CollectionDB::DBID & id )
{
	if( id.IsEmpty() )
		return;

	// Саму на себя базу перезагружать не нужно
	if( _currentCollection.get() && (_currentCollection->GetId() == id) )
		return;

	// Такой ситуации быть не должно, но проверяю на всякий случай
	CollectionInfo * info = FindCollectionInfo( id );
	if( !info )
		return;

	CString fullPath = info->fullPath;
	if( fullPath.IsEmpty() )
	{
		ShowError( ResFormat( IDS_COLLECTION_NOT_FOUND, fullPath.GetString() ), false );
		return;
	}

	if( fullPath[0] == L'.' )
	{
		CString fullCollectionPath = GetGlobal().GetDefaultDirectory();
		fullCollectionPath += fullPath.GetString() + 2; // пропускаю .\ и добавляю имя файла
		fullPath = fullCollectionPath;
	}

	// Если файл базы данных отсутствует (базу можно удалить с диска не через каталогизатор), то загружать будет нечего
	if( !::PathFileExists( fullPath ) )
	{
		ShowError( ResFormat(IDS_COLLECTION_NOT_FOUND, fullPath.GetString() ), false );
		return;
	}

	// выгружаем текущую коллекцию
	UnloadCurrentCollection();

	// загружаем новую коллекцию
	_currentCollection = std::make_unique<CollectionDB>();
	const CString progressPath = GetGlobal().GetDefaultDirectory() + info->id + L".progress";
	g_forceInvalidateDatabase = false;
	_currentCollection->Load( fullPath, progressPath );

	_needToSaveDatabase = g_forceInvalidateDatabase; // Если было обновление формата базы данных или обектов в базе, то требуется пересохранение
	_needToSaveProgressInfo = g_forceInvalidateDatabase;
	g_forceInvalidateDatabase = false;
}

void CVideoCatDoc::UnloadCurrentCollection()
{
	VC_TRY;

	if( _currentCollection )
	{
		SaveCurrentCollection();
		_currentCollection.reset( nullptr );
	}

	_filteredFileItems.clear();
	_needToSaveDatabase = FALSE;
	_needToSaveProgressInfo = FALSE;

	VC_CATCH( ... )
	{
		ShowError( L"Ошибка сохранения прогресса просмотра", false );
	}
}

void CVideoCatDoc::SaveCurrentCollection()
{
	CollectionInfo * curInfo = FindCollectionInfo( _currentCollection->GetId() );
	if( curInfo )
	{
		CString dbPath;
		CString progressPath;

		if( _needToSaveDatabase )
			dbPath = curInfo->fullPath;

		if( _needToSaveProgressInfo )
			progressPath = GetGlobal().GetDefaultDirectory() + _currentCollection->GetId() + L".progress";

		_currentCollection->Save( dbPath, progressPath );

		_needToSaveDatabase = FALSE;
		_needToSaveProgressInfo = FALSE;
	}
}

CVideoCatDoc::CollectionInfo * CVideoCatDoc::FindCollectionInfo( const CollectionDB::DBID & id )
{
	for( CollectionInfo & info : _collections )
	{
		if( info.id == id )
			return &info;
	}

	return nullptr;
}

void CVideoCatDoc::SaveCollectionList() const
{
	VC_TRY;

	const CString ockhamProperties = GetGlobal().GetSettingsFileName();
	if( ockhamProperties.IsEmpty() )
		return;
	
	Json::Value root;
	Json::StreamWriterBuilder builder;
	const std::unique_ptr<Json::StreamWriter> writer( builder.newStreamWriter() );

	if( !_currentCollection->GetId().IsEmpty() )
	{
		root["lastUsed"] = ToUTF8( _currentCollection->GetId().GetString() );
	}

	if( !_collections.empty() )
	{
		Json::Value collections;

		for( const auto& item : _collections )
		{
			Json::Value jItem;
			jItem["ID"] = ToUTF8( item.id.GetString() );
			jItem["Name"] = ToUTF8( item.name.GetString() );
			jItem["Path"] = ToUTF8( item.fullPath.GetString() );

			collections.append( jItem );
		}
		root["collections"] = collections;
	}

	std::filesystem::path propFile( ockhamProperties.GetString() );
	std::ofstream props( propFile );
	unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
	props.write( (char*)bom, sizeof( bom ) );
	writer->write( root, &props );

	VC_CATCH_ALL;
}

void CVideoCatDoc::LoadCollectionList_Old()
{
	VC_TRY;

	const CString ockhamProperties = GetGlobal().GetOldSettingsFileName();

	wchar_t buffer[MAX_PATH];

	const int numCollections = vcGetPrivateProfileInt( L"COLLECTIONS", L"Num", 0, ockhamProperties );
	for( int i = 0; i < numCollections; ++i )
	{
		CollectionInfo info;

		const CString dbId( ResFormat( L"CollectionID_%i", i + 1 ) );
		const CString dbName( ResFormat( L"CollectionName_%i", i + 1 ) );
		const CString dbPath( ResFormat( L"CollectionPath_%i", i + 1 ) );

		if( 0 == GetPrivateProfileString( L"COLLECTIONS", dbId, L"", buffer, MAX_PATH, ockhamProperties ) )
			continue;
		info.id = buffer;

		if( 0 == GetPrivateProfileString( L"COLLECTIONS", dbName, L"", buffer, MAX_PATH, ockhamProperties ) )
			continue;
		info.name = buffer;

		if( 0 == GetPrivateProfileString( L"COLLECTIONS", dbPath, L"", buffer, MAX_PATH, ockhamProperties ) )
			continue;
		info.fullPath = buffer;

		if( !::PathFileExists( info.fullPath ) )
			continue;

		_collections.push_back( info );
	}

	if( GetPrivateProfileString( L"COLLECTIONS", L"LastUsed", L"", buffer, MAX_PATH, ockhamProperties ) )
	{
		_lastUsedCollection = buffer;
	}

	VC_CATCH_ALL;
}

void CVideoCatDoc::LoadCollectionList()
{
	VC_TRY;

	UnloadCurrentCollection();
	_collections.clear();

	const CString ockhamProperties = GetGlobal().GetSettingsFileName();

	std::error_code errorCode;

	if( !std::filesystem::exists( ockhamProperties.GetString(), errorCode ) )
	{
		LoadCollectionList_Old();
	}
	else
	{
		Json::Value root;
		{
			Json::CharReaderBuilder reader;
			JSONCPP_STRING errs;
			std::ifstream props( ockhamProperties.GetString() );
			char utf8[3];
			props.read( utf8, 3 );
			parseFromStream( reader, props, &root, &errs );
		}

		if( root["collections"].isArray() )
		{
			Json::Value collections = root["collections"];

			for( Json::ArrayIndex i = 0; i < collections.size(); ++i )
			{
				Json::Value item = collections[i];
				if( item.isObject() )
				{
					const Json::String id = item["ID"].asString();
					const Json::String name = item["Name"].asString();
					const Json::String path = item["Path"].asString();

					CollectionInfo info;
					info.id = id.c_str();
					info.name = ToUnicode( name ).c_str();
					info.fullPath = ToUnicode( path ).c_str();

					if( !std::filesystem::exists( info.fullPath.GetString(), errorCode ) )
						continue;

					_collections.emplace_back( info );
				}
			}
		}

		if( root["lastUsed"].isString() )
		{
			_lastUsedCollection = ToUnicode( root["lastUsed"].asString() ).c_str();
		}
	}

	if( _collections.empty() )
	{
		// первый запуск
		FirstRunDlg frd;
		frd.DoModal();
	}

	// если коллекция туториалов не добавлена - добавляем в список
	{
		bool hasTutorial = false;
		for( const auto& info : _collections )
		{
			if( info.id == GetTutorialID() )
			{
				hasTutorial = true;
				break;
			}
		}

		if( !hasTutorial )
		{
			CString tutorialPath = GetGlobal().GetProgramDirectory();
			tutorialPath += L"videocat-tutorial.ockham";

			if( std::filesystem::exists( std::filesystem::path( tutorialPath.GetString() ), errorCode ) )
			{
				CollectionInfo info;
				info.id = GetTutorialID();
				info.name = L"Руководство пользователя";
				info.fullPath = tutorialPath;

				_collections.push_back( info );
			}
		}
	}

	if( _lastUsedCollection.IsEmpty() )
		_lastUsedCollection = GetTutorialID();

	std::sort( _collections.begin(), _collections.end() );

	VC_CATCH_ALL;
}


DisplayItemComparator ReadPredicate()
{
	DWORD predicateId = theApp.GetProfileInt( L"", predicateName, sort_predicates::SortYearDown );

	return sort_predicates::GetPredicate( (sort_predicates::SortType)predicateId );
}

void WritePredicate( DisplayItemComparator predicate )
{
	int predicateId = sort_predicates::GetPredicateId( predicate );

	theApp.WriteProfileInt( L"", predicateName, predicateId );
}

void CVideoCatDoc::SetSortPredicate( sort_predicates::SortType type )
{
	DisplayItemComparator predicate = sort_predicates::GetPredicate( type );

	if( type != sort_predicates::SortDefault && _predicate == predicate )
		return;

	if( predicate == nullptr )
	{
		_predicate = ReadPredicate();
	}
	else
	{
		WritePredicate( predicate );
		_predicate = predicate;
	}

	if( _filteredFileItems.empty() )
		return;

	CollectionDB * cdb = GetCurrentCollection();

	CVideoTreeView * tree = GetVideoTreeView();

	std::sort( _filteredFileItems.begin(), _filteredFileItems.end(), GetEntryPredicate( _predicate, cdb, *tree->GetCurrentItem()) );

	CVideoCatView * view = GetVideoCatView();
	if( view )
	{
		view->RecalcScrollLayout( TRUE );
		view->SelectFirst();
		view->UpdateView();
	}
}

DisplayItemComparator CVideoCatDoc::GetCurrentPredicate() const
{
	return _predicate;
}

BOOL CVideoCatDoc::OnNewDocument()
{
	VC_TRY;

	CWaitCursor wait;

	if (!CDocument::OnNewDocument())
		return FALSE;
	
	this->SetTitle( ResString( AFX_IDS_APP_TITLE) );

	LoadCollectionList();
	ReloadCollection( _lastUsedCollection );

	SetSortPredicate( sort_predicates::SortDefault );

	return TRUE;

	VC_CATCH( ... )
	{
		ShowError( L"Ошибка загрузки коллекции", false );
		return FALSE;
	}
}
//
//BOOL CVideoCatDoc::AddRootDirectory()
//{
//	VC_TRY;
//
//	CSelectRootDirectory dlg;
//
//	if( IDOK != dlg.DoModal() )
//		return TRUE;
//
//	CollectionDB * db = GetCurrentCollection();
//	if( !db )
//		return FALSE;
//
//	EntryHandle rootHandle;
//	
//	const bool isRealRoot = dlg.rootType == 0;
//
//	if( isRealRoot )
//	{
//		rootHandle = db->InsertRoot( dlg.rootName, dlg.rootDir );
//		if( rootHandle == EntryHandle::EMPTY )
//			return FALSE;
//	}
//	else
//	{
//		rootHandle = db->InsertVirtualRoot( dlg.rootName );
//		if( rootHandle == EntryHandle::EMPTY )
//			return FALSE;
//	}
//
//	CollectionDB::RootInfo & rootInfo = db->CreateRootInfo( rootHandle );
//	rootInfo.searchHidden = dlg.useHiddenFolder;
//	rootInfo.innerFilesMode = dlg.useFileInFolder;
//	rootInfo.excludedPath.swap( dlg.excludedPath );
//
//	if( isRealRoot )
//	{
//		SynchronizeFiles sf( db );
//		sf.Process( *db->FindEntry( rootHandle ) );
//
//		if( sf.autoFill )
//		{
//			if( sf.fillPlugin != PluginManual )
//				LoadFilmsInfo( db, rootHandle );
//			else
//				MarkAsUserDefinedInfo( db, rootHandle );
//		}
//
//		if( sf.fillTechInfo )
//		{
//			::ReadFileInfo( *db, rootHandle );
//		}
//
//		if( sf.generateThumbs )
//		{
//			int multiplier = 1;
//			BOOL autoselect = FALSE;
//			const ThumbsMode::Enum mode = GetThumbsGenerationMode( dlg.rootName, multiplier, autoselect );
//
//			GenerateThumbnails( db, rootHandle, mode, multiplier, TRUE );
//		}
//	}
//
//	InvalidateDatabase();
//
//	return TRUE;
//
//	VC_CATCH( ... )
//	{
//		ShowError( L"Ошибка добавления корневой директории", false );
//		return FALSE;
//	}
//}

void CVideoCatDoc::Serialize(CArchive& /*ar*/)
{
}

#ifdef _DEBUG
void CVideoCatDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CVideoCatDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif


void ReadPosters( FilteredFileItems & items, PosterManager & pm )
{
	const int size = (int)items.size();

#pragma omp parallel for
	for( int i = 0; i < size; ++i )
	{
		DisplayItem & item = items[i];
		Gdiplus::Bitmap* poster = nullptr;
		//const bool isSeries = item._info->IsTV();
		pm.LoadPoster( item._info->posterId, poster );

		item._poster.reset( poster );
	}
}

void CVideoCatDoc::BuildRandomMovies()
{
	VC_TRY;

	_filteredFileItems.clear();
	_filterType = FilterType::FileRandom;

	struct FilmList : public Traverse::Base
	{
		std::vector<EntryHandle> films;

		virtual Traverse::Result Run( Entry & entry ) override
		{
			// пропускаем сериалы
			if( entry.IsTV() )
				return Traverse::GOBACK;

			// пропускаем объекты фильмов, находящихся внутри папки, которая "как фильм"
			if( Test(entry.flags, EntryTypes::InnerFile) && entry.childs.empty() )
				return Traverse::CONTINUE;

			// Если это не корневая директория и не папка, то добавляем
			if( !entry.IsFolder() && !entry.thisEntry.IsRoot() )
				films.push_back( entry.thisEntry );

			return Traverse::CONTINUE;
		}
	};

	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;

	CVideoTreeView * treeView = GetVideoTreeView();
	if( treeView )
		treeView->SelectItem( nullptr );

	FilmList filmList;
	cdb->ForEach( &filmList );
	
	// перемешиваем
	if( filmList.films.size() > 1 )
	{
		const unsigned count = (unsigned)filmList.films.size() - 1;
		for( unsigned i = 0; i < count; ++i )
		{
			unsigned index = (unsigned)( ((float)rand() / (float)RAND_MAX) * count );
			std::swap( filmList.films[i], filmList.films[index] );
		}
	}
		
	const unsigned numRandom = (std::min)((unsigned)GetGlobal().numRandomFilms, (unsigned)filmList.films.size());
	if( numRandom == 0 )
		return;

	_filteredFileItems.reserve( numRandom );
	for( unsigned i = 0; i < numRandom; ++i )
	{
		Entry * entry = cdb->FindEntry( filmList.films[i] );
		if( entry )
			_filteredFileItems.emplace_back(  entry  );
	}	

	_currentCatFolder = ResString( IDS_FILTER_TITLE_RANDOM );


	CVideoCatView * view = GetVideoCatView();
	if( view )
	{
		ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

		_searchNamePart.Empty();

		view->RefreshDisplayList();

		if( !_filteredFileItems.empty() && GetGlobal().autoplayRandomFilm )
		{
			// воспроизводим первый из случайных фильмов
			DisplayItem & item = _filteredFileItems[0];

			CommandInfo info;
			info.doc = this;
			info.entry = item._info;
			info.displayItem = &item;

			CommandExecute::Instance().Process( CommandID::OpenEntry, &info );
		}
	}
		
	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Невозможно показать случайные фильмы", ignore );
	}
}

void CVideoCatDoc::BuildRecentlyMovies()
{
	VC_TRY;
	
	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;

	if( !cdb->HasRecentlyViewed() )
		return;

	CVideoTreeView * treeView = GetVideoTreeView();
	if( treeView )
		treeView->SelectItem( nullptr );

	_filteredFileItems.clear();

	const std::list<EntryHandle> & recently = cdb->GetRecentlyViewed();

	_filteredFileItems.reserve( recently.size() );

	for( const EntryHandle & h : recently )
	{
		Entry * entry = cdb->FindEntry( h );
		if( !entry )
			continue;

		_filteredFileItems.emplace_back( DisplayItem( entry ) );
	}

	CVideoCatView * view = GetVideoCatView();
	ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

	_currentCatFolder = ResString( IDS_FILTER_TITLE_RECENTLY );
	_searchNamePart.Empty();

	view->RefreshDisplayList();

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка построения списка последних просмотренных фильмов", ignore );
	}
}

void CVideoCatDoc::BuildNewMovies()
{
	VC_TRY;

	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;
	
	CVideoTreeView * treeView = GetVideoTreeView();
	if( treeView )
		treeView->SelectItem( nullptr );

	struct GetFileTimes : public Traverse::Base
	{
		std::map<EntryHandle, FILETIME> allMovies;

		Traverse::Result Run( Entry & entry ) override
		{
			if( !entry.thisEntry.IsRoot() && !entry.IsFolder() && entry.numViews == 0 )
			{
				if( !Test(entry.flags, EntryTypes::TVepisode) )
				{
					allMovies.emplace( entry.thisEntry, entry.date );
				}
				else
				{
					// для эпизодов добавляем не сам эпизод, а папку с сезоном или сериалом
					// причём, задаём самую последнюю дату из всех имеющихся эпизодов, чтобы при добавлении серий папка попадала в новинки
					auto ii = allMovies.find( entry.parentEntry );
					if( ii == allMovies.end() )
					{
						allMovies.emplace( entry.parentEntry, entry.date );
					}
					else
					{
						if( LessDate( ii->second, entry.date) )
							ii->second = entry.date;
					}
				}
			}

			return Traverse::CONTINUE;
		}
	};

	GetFileTimes gft;
	cdb->ForEach( &gft );

	std::vector< std::pair<EntryHandle, FILETIME> > allMovies( gft.allMovies.begin(), gft.allMovies.end() );

	std::sort( allMovies.begin(), allMovies.end(), []( const auto & lhs, const auto & rhs )->bool{	return LessDate( lhs.second, rhs.second);	}	);

	_filteredFileItems.clear();

	const int left = (std::max)( 0, (int)gft.allMovies.size() - 50 );
	for( int i = (int)gft.allMovies.size() - 1; i >= left; --i )
	{
		Entry * entry = cdb->FindEntry( allMovies[i].first );
		if( !entry )
			continue;

		_filteredFileItems.emplace_back( DisplayItem( entry ) );
	}

	CVideoCatView * view = GetVideoCatView();
	ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

	_currentCatFolder = ResString( IDS_FILTER_TITLE_NEW_FILMS );
	_searchNamePart.Empty();

	view->RefreshDisplayList();

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка построения списка новых фильмов", ignore );
	}
}


void CVideoCatDoc::RebuildFilteredFiles( Entry * info, Entry * oldFi )
{
	VC_TRY;

	_filterType = FilterType::FileDirectory;

	_searchNamePart.Empty();

	_filteredFileItems.clear();
	bool isListView = false;

	if( info )
	{
		_filteredFileItems.reserve( info->childs.size() );

		CollectionDB * cdb = GetCurrentCollection();
		if( !cdb )
			return;

		for( auto & item : info->childs )
		{
			Entry * entry = cdb->FindEntry( item );
			if( entry )
				_filteredFileItems.push_back( entry );
		}

		if( !_filteredFileItems.empty() )
			std::sort( _filteredFileItems.begin(), _filteredFileItems.end(), GetEntryPredicate( _predicate, cdb, *info ) );

		switch( GetGlobal().GetViewType() )
		{
			case ViewType::Mixed:
				isListView = info->IsTV();
				break;
			case ViewType::List:
				isListView = true;
				break;
			default: // ViewType::Tiles
				isListView = false;
				break;
		}

		ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

		_currentCatFolder = info->title.c_str();
		if( _currentCatFolder.IsEmpty() )
			_currentCatFolder = info->titleAlt.c_str();
	}

	CVideoCatView * view = GetVideoCatView();
	if( view )
	{
		view->SetListView( isListView );

		if( info && !_filteredFileItems.empty() && info->IsTV() )
		{
			const bool startState = _filteredFileItems.front()._info->numViews > 0;

			for( auto & fi : _filteredFileItems )
			{
				const unsigned int numViews = fi._info->numViews;
				if( (numViews > 0) != startState )
				{
					oldFi = fi._info;
					break;
				}
			}
		}

		view->RefreshDisplayList( oldFi );
	}

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка отображения списка фильмов", ignore );
	}
}

bool CVideoCatDoc::FindFilesByName( const CString & searchString, BOOL extraFiltering )
{
	VC_TRY;

	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return false;

	Traverse::FindByName findByName;
	findByName.SearchFor( searchString, false );
	
	const bool useExtraFiltering = extraFiltering && !_filteredFileItems.empty();
	const bool useFastSearch = !_searchNamePart.IsEmpty() && (0 == searchString.Find( _searchNamePart ) ) && ( -1 == searchString.Find(L" "));


	if( useExtraFiltering || useFastSearch )
	{
		// Если это не новый поиск, а уточнение предыдущего запроса, то ищем среди уже найденых, чтобы ускорить поиск
		for( auto & item : _filteredFileItems )
		{
			findByName.Run( *item._info );
		}
	}
	else
	{
		// выполняем поиск по всей базе данных
		cdb->ForEach( &findByName );
	}

	_filteredFileItems.clear();

	for( EntryHandle & eh : findByName.handles )
	{
		Entry * entry = cdb->FindEntry( eh );
		if( entry )
		{
			_filteredFileItems.emplace_back( entry );
		}
	}

	_currentCatFolder = ResString(IDS_FILTER_TITLE_NAME);
	_searchNamePart = searchString;

	return !_filteredFileItems.empty();

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( ResFormat( L"Ошибка поиска по имени.\r\nЗапрос: %s", searchString.GetString() ), ignore );

		return false;
	}
}

void CVideoCatDoc::FindFilesByYear( const CString & searchString, BOOL extraFiltering )
{
	VC_TRY;
	
	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;

	Traverse::FindByYear findByYear;
	findByYear.SearchFor( searchString );

	if( !extraFiltering )
	{
		// Нельзя использовать уточняющий поиск. Выполняем поиск по всей базе данных
		cdb->ForEach( &findByYear );
	}
	else
	{
		for( auto & item : _filteredFileItems )
		{
			findByYear.Run( *item._info );
		}
	}

	_filteredFileItems.clear();

	for( EntryHandle eh : findByYear.handles )
	{
		Entry * entry = cdb->FindEntry( eh );
		if( entry )
			_filteredFileItems.emplace_back( entry );
	}

	_currentCatFolder = ResString( IDS_FILTER_TITLE_YEAR );

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( ResFormat( L"Ошибка поиска по году.\r\nЗапрос: %s", searchString.GetString() ), ignore );
	}
}

void CVideoCatDoc::RebuildFilteredFiles( const CString & namePart, BOOL extraFiltering )
{
	VC_TRY;

	if( namePart.GetLength() < 3 )
		return;

	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;

	const CString subname = namePart;

	CVideoTreeView * treeView = GetVideoTreeView();
	if( treeView )
	{
		treeView->SelectItem( nullptr );
	}

	_filterType = FilterType::FileName;

	// символ # обозначает, что искать только по году
	bool byNameResult = false;
	switch( subname[0] )
	{
		case L'#':
		case L'<':
		case L'>':
			byNameResult = false;
			break;
		default:
			byNameResult = FindFilesByName( subname, extraFiltering );
			break;
	}

	// если по имени не нашлось, попробуем по году
	if( !byNameResult )
		FindFilesByYear( subname, extraFiltering );

	if( !_filteredFileItems.empty() )
		std::sort( _filteredFileItems.begin(), _filteredFileItems.end(), *_predicate );

	const bool isListView = GetGlobal().GetViewType() == 2;

	ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

	CVideoCatView * view = GetVideoCatView();
	if( view )
	{
		view->SetListView( isListView );
		view->RefreshDisplayList();
	}

	VC_CATCH( ... )
	{}
}

void CVideoCatDoc::RebuildFilteredFiles( Genres genres, BOOL strictCheck )
{
	VC_TRY;

	if( genres == 0 )
		return;

	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;

	CVideoTreeView * treeView = GetVideoTreeView();
	if( treeView )
	{

		treeView->SelectItem( nullptr );
	}

	_filterType = FilterType::FileGenre;

	_searchNamePart.Empty();
	_filteredFileItems.clear();

	Traverse::FindByGenres findByGenres;
	findByGenres.SearchFor( genres, strictCheck != FALSE );

	cdb->ForEach( &findByGenres );

	for( EntryHandle eh : findByGenres.handles )
	{
		Entry * entry = cdb->FindEntry( eh );
		if( entry )
			_filteredFileItems.emplace_back( entry );
	}

	_currentCatFolder = ResString(IDS_FILTER_TITLE_GENRE);

	if( !_filteredFileItems.empty() )
		std::sort( _filteredFileItems.begin(), _filteredFileItems.end(), *_predicate );

	const bool isListView = GetGlobal().GetViewType() == 2;

	ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

	CVideoCatView * view = GetVideoCatView();
	if( view )
	{
		view->SetListView( isListView );
		view->RefreshDisplayList();
	}

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка поиска по жанру", ignore );

	}
}

void CVideoCatDoc::RebuildFilteredFiles( std::set<TagId> & tags, BOOL strictCheck )
{
	VC_TRY;

	if( tags.empty() )
		return;

	_filterType = FilterType::FileTags;

	_searchNamePart.Empty();
	_filteredFileItems.clear();

	CollectionDB * cdb = GetCurrentCollection();
	if( !cdb )
		return;

	CVideoTreeView * treeView = GetVideoTreeView();
	if( treeView )
	{
		treeView->SelectItem( nullptr );
	}

	Traverse::FindByTags findByTags;
	findByTags.SearchFor( tags, strictCheck != FALSE );

	cdb->ForEach( &findByTags );

	for( const EntryHandle & h : findByTags.handles )
	{
		Entry * entry = cdb->FindEntry( h );
		if( entry )
			_filteredFileItems.emplace_back( entry );
	}

	if( !_filteredFileItems.empty() )
		std::sort( _filteredFileItems.begin(), _filteredFileItems.end(), *_predicate );
	
	_currentCatFolder = ResString( IDS_FILTER_TITLE_TAG );

	const bool isListView = GetGlobal().GetViewType() == ViewType::List;
	ReadPosters( _filteredFileItems, cdb->GetPosterManager() );

	CVideoCatView * view = GetVideoCatView();
	if( view )
	{
		view->SetListView( isListView );
		view->RefreshDisplayList();
	}

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка поиска по метке", ignore );
	}
}

CVideoCatView * CVideoCatDoc::GetVideoCatView()
{
	POSITION viewPos = GetFirstViewPosition();
	CView * view = GetNextView( viewPos );
	while( view )
	{
		if( view->IsKindOf( RUNTIME_CLASS( CVideoCatView ) ) )
			return dynamic_cast<CVideoCatView*>(view);
		else
			view = GetNextView( viewPos );
	}

	return nullptr;
}

CVideoTreeView * CVideoCatDoc::GetVideoTreeView()
{
	POSITION viewPos = GetFirstViewPosition();
	CView * view = GetNextView( viewPos );
	while( view )
	{
		if( view->IsKindOf( RUNTIME_CLASS( CVideoTreeView ) ) )
			return dynamic_cast<CVideoTreeView*>(view);
		else
			view = GetNextView( viewPos );
	}

	return nullptr;
}


void CVideoCatDoc::OnCloseDocument()
{
	SaveCollectionList();

	UnloadCurrentCollection();

	CDocument::OnCloseDocument();
}

BOOL CVideoCatDoc::IsFilterDirectory() const
{
	return _filterType == FilterType::FileDirectory;
}

BOOL CVideoCatDoc::IsFilterName() const
{
	return _filterType == FilterType::FileName;
}

BOOL CVideoCatDoc::IsFilterGenre() const
{
	return _filterType == FilterType::FileGenre;
}

void CVideoCatDoc::AddViewedFile( Entry & entry )
{
	GetCurrentCollection()->AddViewedFile( entry );

	InvalidateProgressInfo();
}

void CVideoCatDoc::AddPerson( const Entry & person, const PersonInfo & personInfo )
{
	if( !_currentCollection.get() )
		return;

	_currentCollection->InsertEntry( person );
	_currentCollection->AddPerson( personInfo );
	
	InvalidateDatabase();
}

PersonInfo * CVideoCatDoc::FindPerson( const EntryHandle & handle )
{
	if( !_currentCollection.get() )
		return nullptr;

	return _currentCollection->FindPerson( handle );
}

void CVideoCatDoc::DeletePerson(const  EntryHandle & handle )
{
	if( !_currentCollection.get() )
		return;

	_currentCollection->DeletePerson( handle );

	InvalidateDatabase();
}



std::wstring CVideoCatDoc::GetMoviePath( const Entry & entry ) const
{
	Entry * rootEntry = _currentCollection->FindEntry( entry.GetRootHandle() );
	if( !rootEntry )
		return std::wstring();

	if( Test(entry.flags, EntryTypes::InnerFile) )
	{
		if( !entry.childs.empty() )
		{
			Entry * child = _currentCollection->FindEntry( entry.childs[0] );
			if( !child )
				return std::wstring();

			return rootEntry->filePath + child->filePath;
		}
		else
		{
			return rootEntry->filePath + entry.filePath;
		}
	}
	else
	{
		if( Test(entry.flags, EntryTypes::DvdArchive) )
		{
			return rootEntry->filePath + entry.filePath + DVD_IFO_FILE_NAME;
		}
		else
		{
			return rootEntry->filePath + entry.filePath;
		}
	}	
}

