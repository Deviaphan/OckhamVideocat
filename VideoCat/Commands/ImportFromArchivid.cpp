// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include <vector>
#include <map>
#include <fstream>
#include "CommandInfo.h"
#include "plugin/ArchividImport.h"
#include "VideoCatDoc.h"
#include "CollectionDB.h"
#include "ErrorDlg.h"
#include "CardAutoFill.h"
#include "CollectionDB.h"
#include "Cyrillic.h"
#include "Kinopoisk/FilmDataBase.h"
#include "Kinopoisk/Kinopoisk.h"
#include "PosterDownloader.h"
#include "SynchronizeFiles.h"
#include "TagManager.h"
#include "WinVerTest.h"
#include "genres.h"
#include "ErrorDlg.h"
#include "ProgressDlg.h"
#include "ResString.h"
#include "resource.h"

struct AtrchiVidInfo
{
	CString group; // группа - будет преобразована в папку. Иначе фильм будет в корне

	CString nameRU;
	CString nameEN;
	std::vector<CString> genres;
	std::vector<CString> tags;

	CString description;
	CString diskNumber; // номер диска

	CString posterFile;

	unsigned short year;
};

bool ParseFile( const CString & filename, AtrchiVidInfo & avInfo );
void ImportEntries( const CString & dirPath, std::vector<AtrchiVidInfo> & entries );
void Import( const CString & archividDir, CollectionDB * cdb );


void ImportFromArchivid( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	CArchividImport dlg;
	if( dlg.DoModal() != IDOK )
		return;

	Import( dlg.archividDir, cdb );

	doc->InvalidateDatabase();

	//doc->GetVideoTreeView()->RebuildTree();
	//doc->GetVideoTreeView()->SelectItem( nullptr );

	VC_CATCH_ALL;
}

void Import( const CString & archividDir, CollectionDB * cdb )
{
	VC_TRY;

	Entry * rootArchiVid = nullptr;

	// ищем корневую директорию с именем 'ArchiVid'.
	unsigned numRoots = cdb->GetNumRoots();
	for( unsigned ri = 0; ri < numRoots; ++ri )
	{
		EntryHandle hRoot = cdb->GetRoot( ri );
		Entry * root = cdb->FindEntry( hRoot );
		if( !root )
			continue;

		if( root->title == L"ArchiVid" )
		{
			rootArchiVid = root;
			break;
		}
	}

	// Если такой нет, то создаём новую
	if( !rootArchiVid )
	{
		EntryHandle r = cdb->InsertVirtualRoot( L"ArchiVid" );
		rootArchiVid = cdb->FindEntry( r );

		cdb->CreateRootInfo( r );
	}

	if( !rootArchiVid )
	{
		return;
	}

	std::vector<AtrchiVidInfo> importedEntries;

	ImportEntries( archividDir, importedEntries );

	if( importedEntries.empty() )
	{
		return;
	}

	std::map<CString, Entry *> allGroups;

	//сперва находим все группы и создаём для них группирующие папки
	for( AtrchiVidInfo & avInfo : importedEntries )
	{
		if( avInfo.group.IsEmpty() )
			continue;

		auto ii = allGroups.find( avInfo.group );
		if( ii != allGroups.end() )
			continue;

		Entry groupEntry;
		groupEntry.title = avInfo.group;
		groupEntry.rootEntry = rootArchiVid->GetRootHandle();
		groupEntry.parentEntry = rootArchiVid->thisEntry;
		groupEntry.thisEntry.hash = cdb->GetNextVirtualIndex();
		groupEntry.thisEntry.type = EntryHandle::IS_VIRTUAL;
		Set( groupEntry.flags, EntryTypes::IsFolder );

		rootArchiVid->AddChild( groupEntry.thisEntry );

		cdb->InsertEntry( groupEntry, false );
		allGroups[avInfo.group] = cdb->FindEntry( groupEntry.thisEntry );
	}

	Curl::Global cGuard;
	Curl curl( 15, false );

	Kinopoisk & kinopoisk = GetKinopoisk();

	ProgressController progress;
	progress.Create( L"Импорт фильмов" );
	progress.SetProgressCount( (int)importedEntries.size() );

	for( AtrchiVidInfo & avInfo : importedEntries )
	{
		progress.NextStep();

		Entry * parentEntry = nullptr;
		if( avInfo.group.IsEmpty() )
		{
			parentEntry = rootArchiVid;
		}
		else
		{
			parentEntry = allGroups[avInfo.group];
		}

		Entry entry;
		entry.rootEntry = parentEntry->GetRootHandle();
		entry.parentEntry = parentEntry->thisEntry;
		entry.thisEntry.hash = cdb->GetNextVirtualIndex();
		entry.thisEntry.type = EntryHandle::IS_VIRTUAL;
		entry.pluginID = (PluginID)PluginManual;

		entry.title = avInfo.nameRU;
		entry.titleAlt = avInfo.nameEN;
		entry.year = avInfo.year;
		entry.description = avInfo.description;
		if( !avInfo.diskNumber.IsEmpty() )
		{
			CString fmt;
			fmt.Format( L"№ диска: %s", avInfo.diskNumber.GetString() );
			entry.tagline = fmt;
		}

		// проверяем поддерживаемый список жанров
		for( const CString & g : avInfo.genres )
		{
			Genre genre = Find( g );
			if( genre != Genre::None )
			{
				Set( entry.genres, genre );
			}
		}

		TagManager & tm = cdb->GetTagManager();
		for( const CString & t : avInfo.tags )
		{
			TagId tag = tm.FindTag( t.GetString() );
			if( tag != EMPTY_TAG )
			{
				entry.tags.push_back( tag );
				continue;
			}

			TagId newTag = tm.AddTag( t.GetString() );
			entry.tags.push_back( newTag );
		}

		if( !avInfo.posterFile.IsEmpty() )
		{
			CString posterPath = archividDir + L"\\" + avInfo.posterFile;
			std::unique_ptr<Gdiplus::Bitmap> posterFile( Gdiplus::Bitmap::FromFile( posterPath ) );
			if( posterFile )
			{
				Gdiplus::Bitmap * poster = nullptr;
				if( ResizePoster( posterFile.get(), poster ) )
				{
					entry.posterId = cdb->GetPosterManager().SaveImage( poster );
					delete poster;
				}
			}
		}

		{
			// попробуем дозаполнить то, что не заполнено
			CString fullName;
			if( entry.year )
			{
				fullName.Format( L"%i ", (int)entry.year );
			}
			if( !entry.title.empty() )
			{
				fullName += entry.title.c_str();
				fullName += L" ";
			}
			if( !entry.titleAlt.empty() )
			{
				fullName += entry.titleAlt.c_str();
			}

			const KinopoiskInfo & ki = kinopoisk.SearchInfo( fullName, false );

			if( CompareNoCase( entry.titleAlt, ki.nameRU ) == 0 )
			{
				entry.filmId = ki.filmID;

				entry.urlLink = ResFormat( L"https://www.kinopoisk.ru/film/%u/", entry.filmId );
				entry.genres = ki.genres;
				entry.rating = ki.rating;
				entry.year = ki.year;
				entry.pluginID = PluginKinopoisk;

				if( entry.description.empty() )
				{
					entry.description = ki.description;
				}
				if( entry.tagline.empty() )
					entry.tagline = ki.tagline;

				if( entry.posterId == DefaultPosterID )
				{
					// адресс постера на кинопоиске. Если кинопоиск что-то поменяет, тут нужно скорректировать
					char buffer[64];
					sprintf_s( buffer, 64, "http://st.kp.yandex.net/images/film_big/%u.jpg", entry.filmId );
					std::vector<std::string> url;
					url.push_back( buffer );

					Gdiplus::Bitmap * poster = nullptr;
					if( DownloadImage( curl, url, poster ) )
					{
						entry.posterId = cdb->GetPosterManager().SaveImage( poster );
						delete poster;
					}
				}
			}
		}

		if( cdb->InsertEntry( entry, false ) )
			parentEntry->AddChild( entry.thisEntry );
	}

	VC_CATCH( ... )
	{
		ShowError( L"Ошибка импорта базы данных ArchiVid", false );
	}
}


void ImportEntries( const CString & dirPath, std::vector<AtrchiVidInfo> & entries )
{
	static bool checkWindowsVersion = true;
	static bool isWin7plus = true;
	if( checkWindowsVersion )
	{
		checkWindowsVersion = false;
		isWin7plus = IsWin7Plus();
	}

	const FINDEX_INFO_LEVELS infoLevel = isWin7plus ? FindExInfoBasic : FindExInfoStandard;
	const FINDEX_SEARCH_OPS searchOp = FindExSearchNameMatch;
	const DWORD useLargeFetch = isWin7plus ? FIND_FIRST_EX_LARGE_FETCH : 0;

	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFileEx( dirPath + L"\\*.html", infoLevel, &fd, searchOp, nullptr, useLargeFetch );

	if( hFind == INVALID_HANDLE_VALUE )
		return;

	FindGuard guard( hFind );
	do
	{
		// пропускаем текущую и родительскую папки
		if( !wcscmp( fd.cFileName, L"." ) || !wcscmp( fd.cFileName, L".." ) )
			continue;

		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;

		AtrchiVidInfo e;
		if( ParseFile( dirPath + L"\\" + fd.cFileName, e ) )
			entries.push_back( e );
	}
	while( ::FindNextFile( hFind, &fd ) );

}

bool ParseFile( const CString & filename, AtrchiVidInfo & avInfo )
{
	std::ifstream readFile( filename, std::ios::binary | std::ios::ate );
	std::streamsize size = readFile.tellg();
	readFile.seekg( 0, std::ios::beg );

	std::vector<char> buffer( (size_t)size );
	if( readFile.read( buffer.data(), size ).bad() )
		return false;

	std::wstring unicode;
	unicode.resize( buffer.size() * 4 );

	int count = MultiByteToWideChar( 1251, 0, std::data( buffer ), buffer.size(), &unicode[0], unicode.size() );
	if( count <= 0 )
		return false;

	// год считываю из названия файла - так проще
	{
		int year = _wtoi( filename.GetString() + (filename.GetLength() - 10) ); // rus (en) (1995).html
		if( year > 1900 )
			avInfo.year = (unsigned short)year;
	}

	size_t curPos = 0;

	size_t pos = unicode.find( L"=name>", curPos );
	if( pos != unicode.npos )
	{
		pos += wcslen( L"=name>" );

		size_t endPos = unicode.find( L"</span>", pos );
		if( pos == unicode.npos )
			return false;

		avInfo.nameRU = unicode.substr( pos, endPos - pos ).c_str();
		avInfo.nameRU.Replace( L"&nbsp;", L"" );

		curPos = endPos;
	}

	pos = unicode.find( L"=original>", curPos );
	if( pos != unicode.npos )
	{
		pos += wcslen( L"=original>" );

		size_t endPos = unicode.find( L"</span>", pos );
		if( pos == unicode.npos )
			return false;

		avInfo.nameEN = unicode.substr( pos, endPos - pos ).c_str();
		avInfo.nameEN.Replace( L"&nbsp;", L"" );

		curPos = endPos;
	}

	if( avInfo.nameRU.IsEmpty() && avInfo.nameEN.IsEmpty() )
		return false;

	pos = unicode.find( L"=genre>", curPos );
	if( pos != unicode.npos )
	{
		pos += wcslen( L"=genre>" );
		if( unicode[pos] == L'&' )
			pos += 6; // &nbsp;

		size_t endPos = unicode.find( L"</span>", pos );
		if( pos == unicode.npos )
			return false;

		if( unicode[endPos - 1] == L';' )
			endPos -= 6; // &nbsp;

		if( pos < endPos )
		{
			std::wstring allGenres = unicode.substr( pos, endPos - pos );
			curPos = endPos;

			unsigned start = 0;
			wchar_t token[64];
			while( true )
			{
				Tokenize( allGenres, start, token );
				if( !token[0] )
					break;

				avInfo.genres.push_back( token );

			};
		}
	}

	pos = unicode.find( L"frame\"><a href=\"", curPos );
	if( pos != unicode.npos )
	{
		pos += wcslen( L"frame\"><a href=\"" );
		size_t endPos = unicode.find( L"\">", pos );
		if( pos == unicode.npos )
			return false;

		avInfo.posterFile = unicode.substr( pos, endPos - pos ).c_str();

		curPos = endPos;
	}

	pos = unicode.find( L"№ диска:", curPos );
	if( pos != unicode.npos )
	{
		pos = unicode.find( L"fieldvalue\">", pos );
		if( pos != unicode.npos )
		{
			pos += wcslen( L"fieldvalue\">" );

			size_t endPos = unicode.find( L"</TD>", pos );
			if( pos == unicode.npos )
				return false;

			avInfo.diskNumber = unicode.substr( pos, endPos - pos ).c_str();

			curPos = endPos;
		}
	}

	pos = unicode.find( L"Группа:", curPos );
	if( pos != unicode.npos )
	{
		pos = unicode.find( L"fieldvalue\">", pos );
		if( pos != unicode.npos )
		{
			pos += wcslen( L"fieldvalue\">" );

			size_t endPos = unicode.find( L"</TD>", pos );
			if( pos == unicode.npos )
				return false;

			avInfo.group = unicode.substr( pos, endPos - pos ).c_str();

			curPos = endPos;
		}
	}

	pos = unicode.find( L"Носитель:", curPos );
	if( pos != unicode.npos )
	{
		pos = unicode.find( L"fieldvalue\">", pos );
		if( pos != unicode.npos )
		{
			pos += wcslen( L"fieldvalue\">" );

			size_t endPos = unicode.find( L"</TD>", pos );
			if( pos == unicode.npos )
				return false;

			avInfo.diskNumber += L" (";
			avInfo.diskNumber += unicode.substr( pos, endPos - pos ).c_str();
			avInfo.diskNumber += L")";

			curPos = endPos;
		}
	}

	pos = unicode.find( L"Категории:", curPos );
	if( pos != unicode.npos )
	{
		pos = unicode.find( L"fieldvalue\">", pos );
		if( pos != unicode.npos )
		{
			pos += wcslen( L"fieldvalue\">" );

			size_t endPos = unicode.find( L"</TD>", pos );
			if( pos == unicode.npos )
				return false;

			CString tags = unicode.substr( pos, endPos - pos ).c_str();
			tags.Replace( L"<br>", L" " );

			std::wstring allTags = tags.GetString();
			curPos = endPos;

			unsigned start = 0;
			wchar_t token[64];
			while( true )
			{
				Tokenize( allTags, start, token );
				if( !token[0] )
					break;

				CString tagName = token;
				ToLower( tagName );
				avInfo.tags.push_back( tagName );
			};
		}
	}

	pos = unicode.find( L"description\">", curPos );
	if( pos != unicode.npos )
	{
		pos += wcslen( L"description\">" );

		size_t endPos = unicode.find( L"</div>", pos );
		if( pos == unicode.npos )
			return false;

		avInfo.description = unicode.substr( pos, endPos - pos ).c_str();
		avInfo.description.Replace( L"<BR>", L"\n" );
		avInfo.description.Replace( L"<BR/>", L"\n" );
	}

	return true;
}
