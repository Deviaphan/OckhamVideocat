// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AndroidExport.h"
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <algorithm>
#include "AndroidExportDlg.h"
#include "../CollectionDB.h"
#include "../Traverse/TraverseCollection.h"
#include "../poster.h"
#include "../Cyrillic.h"
#include "../ProgressDlg.h"
#include "../ResString.h"
#include "../GlobalSettings.h"
#include "../SynchronizeFiles.h"
#include "../LoadPng.h"
#include "../BitmapTools.h"
#include "../resource.h"

struct GroupFilms : public Traverse::Base
{
	typedef std::map< std::wstring, std::vector< std::pair<EntryHandle, std::wstring> > > Groups;

	bool byRu = true;
	Groups groupRu;

	bool byEn = true;
	Groups groupEn;

	bool byYear = true;
	typedef std::map< unsigned, std::vector<std::pair<EntryHandle, std::wstring> > > GroupYear;
	GroupYear groupYear;

	bool byGenre = true;
	Groups groupGenre;

	bool byTags = true;
	Groups groupTags;

	bool byTree = false;

	std::vector<EntryHandle> allEntries;

	int exportMode = CAndroidExportDlg::ExportAllFilm;
	std::set<TagId> tags;

	TagManager * tm = nullptr;

	Traverse::Result Run( Entry & entry ) override
	{
		bool needProcess = false;

		if( !entry.IsFolder() )
		{
			// добавляются только фильмы, без эпизодов
			if( !entry.IsTV() && !entry.thisEntry.IsRoot() )
				needProcess = true;
		}
		else if( Test(entry.flags, EntryTypes::TVseries) )
		{
			// для сериалов добавляется только название самого сериала, без сезонов и эпизодов
			needProcess = true;
		}

		if( !needProcess )
		{
			if( byTree )
				allEntries.push_back( entry.thisEntry );

			return Traverse::CONTINUE;
		}

		switch( exportMode )
		{
			case CAndroidExportDlg::ExportAllFilm:
			{
				break;
			}
			case CAndroidExportDlg::ExportSelectedTag:
			{
				if( !HasTag(entry) )
					return Traverse::CONTINUE;
				break;
			}
			case CAndroidExportDlg::ExportNotSelectedTag:
			{
				if( HasTag( entry ) )
					return Traverse::CONTINUE;
				break;
			}
		}

		if( byRu )
			AddByRu( entry );

		if( byEn )
			AddByEn( entry );

		if( byYear )
			AddByYear( entry );

		if( byGenre )
			AddByGenre( entry );

		if( byTags )
			AddByTag( entry );

		allEntries.push_back( entry.thisEntry );

		return Traverse::CONTINUE;
	}

	bool HasTag( const Entry & entry ) const
	{
		for( const TagId & tag : entry.tags )
		{
			if( tags.find( tag ) != tags.end() )
				return true;
		}

		return false;
	}

	void AddByTag( const Entry & entry )
	{
		for( const TagId & tag : entry.tags )
		{
			groupTags[tm->GetTag( tag )].emplace_back( entry.thisEntry, entry.title );
		}
	}

	void AddByGenre( const Entry & entry )
	{
		if( entry.genres == EmptyGenres )
			return;

		for( int g = (int)Genre::FirstGenre; g < (int)Genre::NumOfGenres; ++g )
		{
			if( Test( entry.genres, (Genre)g ) )
			{
				groupGenre[Descript( (Genre)g ).GetString()].emplace_back( entry.thisEntry, entry.title );
			}
		}
	}

	void AddByYear( const Entry & entry )
	{
		groupYear[entry.year].emplace_back( entry.thisEntry, entry.title );
	}

	void AddByRu( const Entry & entry )
	{
		if( !entry.title.empty() )
		{
			std::wstring gName;
			gName = std::toupper( entry.title[0] );

			if( !iswalpha( gName[0] ) )
				gName = L"...";

			groupRu[gName].emplace_back( entry.thisEntry, entry.title );
		}
	}

	void AddByEn( const Entry & entry )
	{
		if( entry.titleAlt.empty() )
			return;

		std::wstring gName;

		CString en = entry.titleAlt.c_str();
		ToUpper( en );
		int pos = en.Find( L"THE ", 0 );
		if( pos == 0 )
		{
			gName = (en.GetLength() > 4) ? std::toupper( en[4] ) : std::toupper( en[0] );
		}
		else if( (pos = en.Find( L"A " )) == 0 )
		{
			gName = (en.GetLength() > 3) ? std::toupper( en[3] ) : std::toupper( en[0] );
		}
		else
		{
			gName = std::toupper( en[0] );
		}

		if( !iswalpha( gName[0] ) )
			gName = L"...";

		groupEn[gName].emplace_back( entry.thisEntry, entry.titleAlt );
	}
};

void SaveToFile( const std::wstring & data, const CString & fileName )
{
	std::string utf8;
	utf8.resize( data.size() * 4 );
	int utf8Size = WideCharToMultiByte( CP_UTF8, 0, data.c_str(), data.size(), std::data(utf8), utf8.size(), 0, 0 );
	utf8[utf8Size] = 0;

	std::ofstream out( fileName );
	out << utf8.c_str();
}

class CreateExport
{
public:
	CreateExport( CollectionDB & db, int markType, bool descr )
		: _db( db )
		, _markType( markType )
		, _descr( descr )
	{
	}

public:
	void IndexFile( const CString & fileName, const GroupFilms & groups );

	void ExportChilds( const Entry * root, const CString & parentFile );
	void EntryFiles( const CString & fileParent, const CString & fileName, std::vector< std::pair<EntryHandle, std::wstring> > & films );
	
	void Lang( const CString & fileName, GroupFilms::Groups & data, const wchar_t * prefix);
	void Years( const CString & fileName, GroupFilms::GroupYear & data );
	void List(  const CString & fileName, GroupFilms::Groups & data );
	void FileTree( GroupFilms & data );
	void Posters( const CString  & posterFolder, const GroupFilms & groups );

private:
	void AddFileHeader( std::wstring & file, const wchar_t * cssPath );

public:
	const CollectionDB & _db;
	int _markType;
	bool _descr;

	CString _folder;
};

void CreateExport::AddFileHeader( std::wstring & file, const wchar_t * cssPath )
{
	file += L"<!DOCTYPE HTML>\n";
	file += L"<html lang='ru-RU'>\n" \
		L"<head>\n" \
		L"<meta charset='utf-8'/>\n" \
		L"<title>Ockham:VideoCat Menu</title>\n" \
		L"<link rel='stylesheet' type='text/css' href='";

	file += cssPath;

	file += L"'/>\n</head>\n";
}

void CreateExport::IndexFile( const CString & fileName, const GroupFilms & groups )
{
	std::wstring index;
	AddFileHeader( index, L"./data/ockham.css" );

	index += L"<body>\n<table class='Index'>\n"	\
		L"<th class='Title'>Режим фильтрации</th>\n";

	if( groups.byTree )
	{
		index += L"<tr><td><a href='./data/file-tree.html'><div class='Alpha'>Дерево папок</div></a></td></tr>\n";
	}
	if( groups.byRu )
	{
		index += L"<tr><td><a href='./data/name-ru.html'><div class='Alpha'>Русское название</div></a></td></tr>\n";
	}
	if( groups.byEn )
	{
		index += L"<tr><td><a href='./data/name-en.html'><div class='Alpha'>Оригинальное название</div></a></td></tr>\n";
	}
	if( groups.byYear )
	{
		index += L"<tr><td><a href='./data/year.html'><div class='Alpha'>Год выпуска</div></a></td></tr>\n";
	}
	if( groups.byGenre )
	{
		index += L"<tr><td><a href='./data/genre.html'><div class='Alpha'>Жанр</div></a></td></tr>\n";
	}
	if( groups.byTags )
	{
		index += L"<tr><td><a href='./data/tags.html'><div class='Alpha'>Личная Метка</div></a></td></tr>\n";
	}

	index += L"</table>\n</body>\n</html>\n";

	SaveToFile( index, fileName );
}

void CreateExport::ExportChilds( const Entry * root, const CString & parentFile )
{
	std::vector< std::pair<EntryHandle, std::wstring> > folderFilms;

	for( const EntryHandle & h : root->childs )
	{
		const Entry * item = _db.FindEntry( h );
		if( !item )
			continue;

		if( item->IsFolder() )
		{
			folderFilms.emplace_back( h, L"..." + item->title );
		}
		else
		{
			folderFilms.emplace_back( h, item->title );
		}
	}

	EntryFiles( parentFile, root->thisEntry.ToString() + L".html", folderFilms );
}

void CreateExport::EntryFiles( const CString & fileParent, const CString & fileName, std::vector< std::pair<EntryHandle, std::wstring> > & films )
{
	std::sort( films.begin(), films.end(), []( auto&l, auto&r )->bool{ return CompareNoCase( l.second, r.second ) < 0; } );

	std::wstring file;

	// добавляется JavaScript функция, Чтобы разворачивать описание фильма
	file += L"<!DOCTYPE HTML>\n"	\
		L"<html lang='ru-RU'>\n"	\
		L"<head>\n"	\
		L"<meta charset='utf-8'/>"	\
		L"<title>Ockham:VideoCat</title>"	\
		L"<link rel='stylesheet' type='text/css' href='./ockham.css'/>\n"	\
		L"<script type = \"text/javascript\"> function openbox( id ){ element = document.getElementById( id ).style; if( element != null ){element.display='block';}} </script>"	\
		L"</head>\n"	\
		L"<body>\n";

	file += L"<a href='./";
	file += fileParent.GetString();
	file += L"'><div class='Title'><img src='back.png'/>НАЗАД</div></a>\n";

	const TagManager & tm = _db.GetTagManager();

	int id = 0;
	wchar_t buffer[10] = {};

	for( const auto & h : films )
	{
		const Entry * entry = _db.FindEntry( h.first );
		if( !entry )
			continue;

		++id;
		
		const bool isFolder = entry->IsFolder() || entry->thisEntry.IsRoot();

		if( isFolder )
		{
			file += L"<a href='";
			file += entry->thisEntry.ToString();
			file += L".html'>";
			file += L"<div class='ENTRY'>";
		}
		else
		{
			if( _descr && !entry->description.empty() )
			{
				_itow_s( id, buffer, 10 );
				file += L"<div class='ENTRY' onclick=\"openbox('";
				file += buffer;
				file += L"'); return false\">";
			}
			else
			{
				buffer[0] = 0;
				file += L"<div class='ENTRY'>";
			}
		}

		file += L"<table class='TBL' background='./p/";
		if( entry->posterId != DefaultPosterID )
		{
			file += entry->thisEntry.ToString().GetString();
		}
		else
		{
			file += L"0";
		}
		file += L".jpg'>\n";

		file += L"<tr>\n<td rowspan='4' class='YR'>";
		if( entry->year > 0 )
			file += ResFormat( L"%u", entry->year ).GetString();
		else
			file += L"____";

		file += L"</td>\n<td class='RU'>";
		if( isFolder )
			file += L"./";
		file += !entry->title.empty() ? entry->title.c_str() : L"&nbsp;";
		file += L"</td>\n";

		switch( _markType )
		{
			case 0:
				if( entry->numViews )
					file += L"<td>&nbsp;</td>\n";
				else
					file += L"<td class='VM'>&nbsp;</td>\n";
				break;
			case 1:
				if( !entry->numViews )
					file += L"<td>&nbsp;</td>\n";
				else
					file += L"<td class='VM'>&nbsp;</td>\n";
				break;
			case 2:
				file += L"<td>&nbsp;</td>\n";
				break;
		}

		file += L"</tr>\n<tr>\n<td class='EN'>";
		file += !entry->titleAlt.empty() ? entry->titleAlt.c_str() : L"&nbsp;";

		file += L"</td><td>&nbsp;</td>\n</tr>\n<tr>\n<td class='FG'>";

		if( entry->genres == EmptyGenres )
		{
			file += L"&nbsp;";
		}
		else
		{
			for( int g = (int)Genre::FirstGenre; g < (int)Genre::NumOfGenres; ++g )
			{
				if( Test( entry->genres, (Genre)g ) )
				{
					file += Descript( (Genre)g ).GetString();
					file += L", ";
				}
			}
		}

		file += L"</td><td>&nbsp;</td>\n</tr>\n<tr>\n<td class='FT'>";

		if( entry->tags.empty() )
		{
			file += L"&nbsp;";
		}
		else
		{
			for( TagId tagId : entry->tags )
			{
				file += tm.GetTag( tagId );
				file += L", ";
			}
		}

		file += L"</td><td>&nbsp;</td>\n</tr>\n</table>\n";
		
		if( buffer[0] != 0 )
		{
			file += L"<div class='FD' id='";
			file += buffer;
			file += L"'>\n";

			CString description =  entry->description.c_str();
			description.Replace( L"<", L"&lt;" );
			description.Replace( L">", L"&gt;" );
			description.Replace( L"\n", L"</BR>" );

			file += description;
			file += L"</div>\n";
		}

		file += L"</div>\n";
		if( isFolder )
		{
			file += L"</a>\n";

			ExportChilds( entry, fileName );
		}
	}

	file += L"<a href='./";
	file += fileParent.GetString();
	file += L"'><div class='Title'><img src='back.png'/>НАЗАД</div></a>\n";

	file += L"</body>\n</html>\n";
	SaveToFile( file, _folder + fileName );
}

void CreateExport::Lang( const CString & fileName, GroupFilms::Groups & data, const wchar_t * prefix )
{
	const unsigned count = (unsigned)data.size();
	const unsigned numCol = 3;
	const unsigned numRow = (count + 2) / numCol;

	std::wstring index;
	AddFileHeader( index, L"./ockham.css" );

	index += L"<body>\n<a href='../index.html'><div class='Title'><img src='back.png'/>МЕНЮ</div></a>\n";
	index += L"<table class='Index'>";

	GroupFilms::Groups::iterator ii = data.begin();

	for( unsigned y = 0; y < numRow; ++y )
	{
		index += L"<tr>";

		for( unsigned x = 0; x < numCol; ++x )
		{
			if( ii == data.end() )
				break;

			CString td;
			td.Format( L"<td><a href='./%s_%s.html'><div class='Alpha'>%s&nbsp;</div><div class='Count'>%u</div></a></td>\n",
				prefix,
				ii->first.c_str(),
				ii->first.c_str(),
				(unsigned)ii->second.size() );

			index += td;

			++ii;
		}

		index += L"</tr>";
	}
	
	index += L"</table>\n";
	index += L"<a href='../index.html'><div class='Title'><img src='back.png'/>МЕНЮ</div></a>\n";
	index += L"</body>\n</html>\n";

	SaveToFile( index, _folder + fileName );

	for( auto & group : data )
	{
		CString groupName;
		groupName.Format( L"%s_%s.html", prefix, group.first.c_str() );

		EntryFiles( fileName, groupName, group.second );
	}
}

void CreateExport::Years( const CString & fileName, GroupFilms::GroupYear & data )
{
	std::wstring index;
	AddFileHeader( index, L"./ockham.css" );

	index += L"<body>\n<a href='../index.html'><div class='Title'><img src='back.png'/>МЕНЮ</div></a>\n";
	index += L"<table class='Index'>";

	for( auto item = data.rbegin(); item != data.rend(); ++item )
	{	
		CString td;
		td.Format( L"<tr><td><a href='./%u.html'><div class='Alpha'>%u&nbsp;</div><div class='Count'>%u</div></a></td></tr>\n",
			item->first,
			item->first,
			(unsigned)item->second.size() );

		index += td;
	}

	index += L"</table>\n";
	index += L"<a href='../index.html'><div class='Title'><img src='back.png'/>МЕНЮ</div></a>\n";
	index += L"</body>\n</html>\n";

	SaveToFile( index, _folder + fileName );

	for( auto & group : data )
	{
		CString groupName;
		groupName.Format( L"%u.html", group.first );

		EntryFiles( fileName, groupName, group.second );
	}
}

void CreateExport::List( const CString & fileName, GroupFilms::Groups & data )
{
	std::wstring index;
	AddFileHeader( index, L"./ockham.css" );

	index += L"<body>\n<a href='../index.html'><div class='Title'><img src='back.png'/>МЕНЮ</div></a>\n";
	index += L"<table class='Index'>";

	for( const auto & item : data )
	{
		CString td;
		td.Format( L"<tr><td><a href='./%s.html'><div class='Alpha'>%s&nbsp;</div><div class='Count'>%u</div></a></td></tr>\n",
			item.first.c_str(),
			item.first.c_str(),
			(unsigned)item.second.size() );

		index += td;
	}

	index += L"</table>\n";
	index += L"<a href='../index.html'><div class='Title'><img src='back.png'/>МЕНЮ</div></a>\n";
	index += L"</body>\n</html>\n";

	SaveToFile( index, _folder + fileName );

	for( auto & group : data )
	{
		EntryFiles( fileName, CString(group.first.c_str()) + L".html", group.second );
	}
}

void CreateExport::FileTree( GroupFilms & data )
{
	// вложенный UL из гиперссылок на страницы с фильмами в каждой из папок
	// файл каждой папки именуется хэшем этой папки

	std::vector < std::pair < EntryHandle, std::wstring > > folderFilms;
	
	const int numRoots = _db.GetNumRoots();
	for( int i = 0; i < numRoots; ++i )
	{
		EntryHandle rootHandle = _db.GetRoot( i );
		const Entry * root = _db.FindEntry( rootHandle );
		if( !root )
			continue;

		folderFilms.emplace_back( rootHandle, root->title );
	}

	EntryFiles( L"../index.html", L"file-tree.html", folderFilms );
}

void CreateExport::Posters( const CString  & posterFolder, const GroupFilms & groups )
{
	ProgressController progress;
	progress.Create( L"Сохранение постеров..." );
	progress.SetProgressCount( (int)groups.allEntries.size() );

	{
		std::unique_ptr<Gdiplus::Bitmap> noPoster( LoadPng( IDB_NOPOSTER ) );
		SaveJpg( posterFolder + L"0.jpg", noPoster.get() );
	}

	const CString hashFileName( posterFolder + L"hash.txt" );

	// загружаем из файла хэши существующих постеров, обновляем только те, которых нет или они различаются
	std::map<CString, unsigned int> posterHashes;
	{
		std::wifstream readHash( hashFileName );

		while( readHash )
		{
			std::wstring str;
			unsigned int crc32 = 0;

			readHash >> str;
			if( str.empty() )
				break;

			readHash >> crc32;

			posterHashes.emplace( std::pair<CString, unsigned>( str.c_str(), crc32 ) );
		};
	}

	std::map<CString, unsigned int> newPosterHashes;

	const PosterManager & pm = _db.GetPosterManager();

	for( const EntryHandle & h : groups.allEntries )
	{
		progress.NextStep();

		const Entry * entry = _db.FindEntry( h );

		if( entry->posterId != DefaultPosterID )
		{
			const CString handle = entry->thisEntry.ToString();

			auto hash = posterHashes.find( handle );

			unsigned int crc32 = hash != posterHashes.end() ? hash->second : 0;

			if( pm.SaveToFile( entry->posterId, posterFolder + entry->thisEntry.ToString() + L".jpg", crc32 ) )
				newPosterHashes.emplace( std::make_pair( handle, crc32 ) );
		}
	}

	{
		std::wofstream writeHash( hashFileName );
		for( const auto & hash : newPosterHashes )
		{
			writeHash << hash.first.GetString() << L" " << hash.second << std::endl;
		}
	}
}

void Copy( const CString & copyFrom, const CString & copyTo, bool copySubfolder = false )
{
	WIN32_FIND_DATA ffd;

	HANDLE hFind = FindFirstFile( copyFrom + L"*.*", &ffd );
	FindGuard guard( hFind );

	do
	{
		if( ffd.cFileName[0] == L'.' )
			continue;

		CString src = copyFrom + ffd.cFileName;
		CString dest = copyTo + ffd.cFileName;

		if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if( copySubfolder )
			{
				//CreateDirectory( szOutFileName, nullptr );
				//Copy( szInFileName, szOutFileName );
			}
		} 
		else
		{
			CopyFile( src, dest, FALSE );
		}
	}
	while( FindNextFile( hFind, &ffd ) );
}

void ExportForAndroid( CollectionDB & cdb )
{
	CAndroidExportDlg dlg( cdb );

	const CString stylesFolder = GetGlobal().GetProgramDirectory() + L"plugins\\AndroidExport\\";
	{
		WIN32_FIND_DATA fd;

		HANDLE hFind = ::FindFirstFileEx( stylesFolder + L"*", FindExInfoStandard, &fd, FindExSearchLimitToDirectories, nullptr, 0 );

		if( hFind == INVALID_HANDLE_VALUE )
			return;

		FindGuard guard( hFind );
		do
		{
			// пропускаем текущую и родительскую папки
			if( !wcscmp( fd.cFileName, L"." ) || !wcscmp( fd.cFileName, L".." ) )
				continue;

			if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				continue;

			dlg.themeNames.push_back( fd.cFileName );
		}
		while( ::FindNextFile( hFind, &fd ) );

		std::sort( dlg.themeNames.begin(), dlg.themeNames.end() );
	}

	if( dlg.DoModal() != IDOK )
		return;

	CWaitCursor waiting;

	CString exportFolder( dlg.exportDir );

	if( exportFolder[exportFolder.GetLength() - 1] != L'\\' )
		exportFolder += L"\\";
	exportFolder += L"ockham.videocat\\";
	::CreateDirectory( exportFolder, nullptr ); // папка с файлом index.html

	CString dataFolder = exportFolder + L"data\\";
	::CreateDirectory( dataFolder, nullptr ); // папка с остальными страницами, и стилями

	CString posterFolder = dataFolder + L"p\\";
	::CreateDirectory( posterFolder, nullptr ); // папка с постерами

	if( !::PathFileExists( posterFolder ) )
	{
		AfxMessageBox( L"Ошибка создания директории.\nПроверьте подключение и права доступа.", MB_OK | MB_TOPMOST );
		return;
	}

	// копируем файлы стиля оформления
	Copy( stylesFolder + dlg.themeNames[dlg.themeIndex] + L"\\", dataFolder, false );

	GroupFilms groups;
	groups.tm = &cdb.GetTagManager();
	groups.byTree = dlg.filterTree;
	groups.byRu = dlg.filterRu;
	groups.byEn = dlg.filterEn;
	groups.byYear = dlg.filterYear;
	groups.byGenre = dlg.filterGenre;
	groups.byTags = dlg.filterTag;

	groups.exportMode = dlg.exportType;
	if( dlg.exportType != CAndroidExportDlg::ExportAllFilm )
	{
		for( const auto & tag : dlg.tags )
		{
			if( tag.second )
				groups.tags.insert( tag.first );
		}
	}

	// Выполняем обход всех фильмов в коллекции и группируем их по критериям
	cdb.ForEach( &groups );

	CreateExport htmlExport(cdb, dlg.markType, dlg.exportDescr != FALSE);
	htmlExport._folder = dataFolder;

	htmlExport.IndexFile( exportFolder + L"index.html", groups );

	if( dlg.filterTree )
		htmlExport.FileTree( groups );

	if( groups.byRu )
		htmlExport.Lang( L"name-ru.html", groups.groupRu, L"r" );

	if( groups.byEn )
		htmlExport.Lang( L"name-en.html", groups.groupEn, L"e" );

	if( groups.byYear )
		htmlExport.Years( L"year.html", groups.groupYear );

	if( groups.byGenre )
		htmlExport.List( L"genre.html", groups.groupGenre );

	if( groups.byTags )
		htmlExport.List( L"tags.html", groups.groupTags );

	htmlExport.Posters( posterFolder, groups );
}
