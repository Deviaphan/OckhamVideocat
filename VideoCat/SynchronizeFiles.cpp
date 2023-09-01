// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "SynchronizeFiles.h"
#include <set>
#include <map>
#include "WinVerTest.h"
#include "HashItem.h"
#include "Cyrillic.h"
#include "ErrorDlg.h"
#include "Traverse/TraverseCollection.h"
#include "SynchronizeDlg.h"
#include "GlobalSettings.h"

#include <filesystem>

namespace fs = std::filesystem;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const wchar_t DVD_IFO_FILE_NAME[] = L"VIDEO_TS\\VTS_01_0.IFO";

CString GetNameFromFile( const wchar_t * path )
{
	wchar_t fullPath[2048] = {};
	wcscpy_s( fullPath, 2048, path );

	wchar_t *name = PathFindFileNameW( fullPath );
	PathRemoveExtension( name );

	int length = wcslen( name );
	length -= 1;
	if( name[length] == L'\\' )
		name[length] = L'\0';

	return name;
}

bool IsSerial( const wchar_t * fileName )
{
	CString title( fileName );
	ToLower( title );
	if( title.Find( L"сериал", 0 ) >= 0 )
	{
		// может быть папка СЕРИАЛЫ, внутри которой лежат уже папки с сериалами. Предусматриваем это
		if( title.Find( L"сериалы" ) >= 0 )
			return false;

		return true;
	}

	return false;
}

SynchronizeFiles::SynchronizeFiles( CollectionDB *db )
	: _rootEntry(nullptr)
	, _startEntry(nullptr)
	, _db(db)
	, autoFill(TRUE)
	, fillTechInfo(TRUE)
	, generateThumbs(TRUE)
	, fillPlugin( PluginKinopoisk )
{

}

struct EntriesInBase : public Traverse::Base
{
	struct UsedData
	{
		Entry * entry = nullptr;
		bool inUse = false;
	};

	std::map< EntryHandle, UsedData > allEntries;

	virtual Traverse::Result Run( Entry & entry ) override
	{
		UsedData ud;
		ud.entry = &entry;
		allEntries[entry.thisEntry] = ud;
		return Traverse::CONTINUE;
	}
};

bool SynchronizeFiles::Process( const Entry & startFolder )
{
	VC_TRY;

	std::error_code errCode;

	_startEntry = &startFolder;
	EntryHandle rootHandle = _startEntry->GetRootHandle();
	_rootEntry = _db->FindEntry( rootHandle );
	
	// Сперва проверяем наличие корневой директории, если она не найдена, то дальше искать нет смысла
	if( !fs::exists( _rootEntry->filePath, errCode ) )
	{
		AfxMessageBox( L"Корневая директория не обнаружена.\r\nПроверьте подключение диска или переместите корневую директорию", MB_OK | MB_ICONEXCLAMATION );
		return false;
	}

	// собираем информацию обо всех файлах и папках, которые уже добавлены в коллекцию
	EntriesInBase eib;
	_db->TraverseAll( startFolder.thisEntry, &eib );
	
	// сперва надо проверить, существует ли синхронизируемая директория
	bool isDirectoryExist = true;
	{
		std::wstring path;
		if( rootHandle == startFolder.thisEntry )
		{
			path = _rootEntry->filePath;
		}
		else
		{
			path = _rootEntry->filePath + startFolder.filePath;
		}

		isDirectoryExist = fs::exists( path, errCode );
	}

	if( isDirectoryExist )
	{
		auto ii = eib.allEntries.find( startFolder.thisEntry );
		if( ii != eib.allEntries.end() )
		{
			// уже имеющийся файл. Помечаем, что он никуда не делся
			ii->second.inUse = true;
		}

		ProcessFolder( startFolder, _rootEntry->filePath.length() );
	}

	// для всех найденых фильмов и папок проверяем, есть ли они уже в каталоге. Отмечаем те, которые есть
	std::vector<unsigned> newFileIndices;
	for( unsigned index = 0; index < (unsigned)newFiles.size(); ++index )
	{
		auto ii = eib.allEntries.find( newFiles[index].thisEntry );
		if( ii != eib.allEntries.end() )
		{
			// уже имеющийся файл. Помечаем, что он никуда не делся
			ii->second.inUse = true;
		}
		else
		{
			// фильмы и папки, которые не были найдены в каталоге - новые
			newFileIndices.push_back( index );
		}
	}

	// фильмы, которые не искались в списке 'eib' - удалены с диска. Показать их в списке на удаление
	std::vector<EntryHandle> lostFiles;
	for( auto & ii : eib.allEntries )
	{
		if( !(ii.second.inUse) )
		{
			lostFiles.push_back( ii.second.entry->thisEntry );
		}
	}

	CSynchronizeDlg syncDlg;
	syncDlg.autofillInfo = autoFill;
	syncDlg.fillTechinfo = fillTechInfo;
	syncDlg.generateThumbs = generateThumbs;

	// формируем список новых файлов
	for( unsigned i : newFileIndices )
	{
		Entry & entry = newFiles[i];

		syncDlg.newFilms.emplace_back( entry.filePath.c_str() );
	}

	// формируем список файлов, которые понадобится удалить
	for( const EntryHandle & h : lostFiles )
	{
		Entry * entry = _db->FindEntry( h );
		if( entry )
			syncDlg.lostedFilms.emplace_back(  entry->filePath.c_str()  );
	}

	if( syncDlg.newFilms.empty() && syncDlg.lostedFilms.empty() )
	{
		AfxMessageBox( L"Изменений в составе коллекции не обнаружено.", MB_OK );
		return false;
	}

	if( syncDlg.DoModal() != IDOK )
		return false;

	autoFill = syncDlg.autofillInfo;
	fillTechInfo = syncDlg.fillTechinfo;
	generateThumbs = syncDlg.generateThumbs;
	fillPlugin = syncDlg.GetAutofillPlugin();

	// Удаляем те потерянные файлы, которые разрешили удалять
	// индексы элементов совпадают с их индексами в исходном массиве
	for( unsigned i = 0; i < (unsigned)syncDlg.lostedFilms.size(); ++i )
	{
		if( syncDlg.lostedFilms[i].allow )
		{
			_db->DeleteEntry( lostFiles[i] );
		}
	}

	for( unsigned i = 0; i < (unsigned)syncDlg.newFilms.size(); ++i )
	{
		if( !syncDlg.newFilms[i].allow )
			continue;

		Entry & entry = newFiles[newFileIndices[i]];
		Entry * parent = _db->FindEntry( entry.parentEntry );
		if( parent )
		{
			// проверяем, что объект ещё не добавлен (папки могут снова попасть в список новых)
			if( _db->InsertEntry( entry, true ) )
			{
				parent->AddChild( entry.thisEntry );
			}
		}
	}

	_db->RemoveEmptyFolders( rootHandle );

	// сортировка дочерних элементов каждого узла
	_db->ResortCollection( rootHandle );

	// Если включен режим "фильм в отдельной папке", то проверяем все папки после добавления
	_db->PostprocessInnerFiles( rootHandle );

	return true;

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка поиска новых фильмов", ignore );

		return false;
	}
}

bool SynchronizeFiles::ProcessFolder( const Entry & startFolder, unsigned rootPathLength )
{
	fs::path searchPath( _rootEntry->filePath );
	if( startFolder.filePath != _rootEntry->filePath )
	{
		searchPath.append( startFolder.filePath );
	}

	const bool isEpisode = startFolder.IsTV();

	std::vector<Entry> folderNewFiles;

	for( const fs::directory_entry & item : fs::directory_iterator( searchPath, std::filesystem::directory_options::skip_permission_denied ) )
	{		
		const std::wstring filename( item.path().filename().wstring() );

		// пропускаем файлы и папки, начинаеющиеся с точки
		if( filename[0] == L'.' )
			continue;		

		Entry newEntry;
		newEntry.parentEntry = startFolder.thisEntry;
		newEntry.filePath = item.path().c_str() + rootPathLength; // сохраняем путь от корневой директории, поэтому начало пути нужно отрезать
		newEntry.rootEntry = _startEntry->GetRootHandle();
		newEntry.title = item.path().stem().wstring(); // Имя по умолчанию формируется из имени файла, без расширения

		const bool isFolder = item.is_directory();

		if( isFolder )
		{
			// папки всегда заканчиваются на слэш (нужно это гарантировать самостоятельно)
			if( newEntry.filePath.back() != L'\\' )
			{
				newEntry.filePath += L"\\";
			}

			// пропускаем папки и файлы, исключённые пользователем из поиска
			if( _db->IsExcludedPath( startFolder.GetRootHandle(), newEntry.filePath.c_str() ) )
				continue;

			// проверяем, не DVD ли это архив (копипаста с DVD диска)
			const std::wstring ifoName = _rootEntry->filePath + newEntry.filePath + DVD_IFO_FILE_NAME;
			std::error_code errCode;
			if( !fs::exists(ifoName, errCode) )
			{
				Set( newEntry.flags, EntryTypes::IsFolder );
				// Если папка содержит слово "сериал", то это папка с сериалом
				if( IsSerial( newEntry.title.c_str() ) )
				{
					Set( newEntry.flags, EntryTypes::TVseries );
				}
				// Если эта папка внутри папки сериала, то это папка "сезон сериала" с эпизодами
				else if( isEpisode )
				{
					Set( newEntry.flags, EntryTypes::TVseason );
				}
			}
			else
			{
				Set( newEntry.flags, EntryTypes::DvdArchive );
			}
		}
		else
		{
			if( !GetGlobal().aviabledFiles.TestFile( filename.c_str() ) )
				continue;

			// Если файл внутри папки сериала или сезона, то это эпизод сериала
			if( isEpisode )
			{
				Set( newEntry.flags, EntryTypes::TVepisode );
			}
		}

		newEntry.thisEntry.type = EntryHandle::IS_FILE;
		newEntry.thisEntry.hash = newEntry.BuildHash();// HashingPath( newEntry.filePath.c_str(), newEntry.filePath.length() );

		newFiles.push_back( newEntry );
		if( isFolder )
		{
			folderNewFiles.push_back( newEntry );
		}
	}

	for( Entry & nextRoot : folderNewFiles )
	{
		ProcessFolder( nextRoot, rootPathLength );
	}

	return true;
}

