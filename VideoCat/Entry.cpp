// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "Entry.h"
#include "GlobalSettings.h"
#include "PosterDownloader.h"
#include "CollectionDB.h"
#include "Net/CurlBuffer.h"
#include "ErrorDlg.h"
#include "ResString.h"
#include "HashItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool g_forceInvalidateDatabase = false;

extern const wchar_t DVD_IFO_FILE_NAME[];

void Entry::CopyUserData( const Entry & rhs )
{
	(EntryBase &)*this = (const EntryBase&)rhs;

	flags = rhs.flags;

	posterId = rhs.posterId;
}

std::wstring Entry::GetMoviePath( const std::wstring& rootDir, CollectionDB & db ) const
{
	std::wstring moviePath( rootDir );
	moviePath += filePath;
	if( Test(flags, EntryTypes::DvdArchive) )
	{
		moviePath += DVD_IFO_FILE_NAME;
	}
	else if( Test(flags, EntryTypes::InnerFile) && !childs.empty() )
	{
		const Entry * e = db.FindEntry( childs[0] );
		if( e )
		{
			moviePath = rootDir + e->filePath;
		}
	}

	return moviePath;	
}

void Entry::SetCurrentDate()
{
	SYSTEMTIME st;
	ZeroMemory( &st, sizeof( st ) );
	::GetSystemTime( &st );
	::SystemTimeToFileTime( &st, &date );
}

void Entry::AddChild( const EntryHandle & handle )
{
	for( const EntryHandle & h : childs )
	{
		if( h == handle )
			return;
	}

	childs.push_back( handle );
}

void Entry::RemoveChild( const EntryHandle & h )
{
	for( auto ii = childs.begin(); ii != childs.end(); ++ii )
	{
		if( *ii == h )
		{
			childs.erase( ii );
			break;
		}
	}
}

PathHashType Entry::BuildHash() const
{
	std::wstring fullPath = rootEntry.ToBase64().GetString();
	fullPath += filePath;

	return HashingPath( fullPath.c_str(), (unsigned int)fullPath.length() );
}




bool AfterEditEqual( const Entry & lhs, const Entry & rhs )
{
	if( lhs.thisEntry != rhs.thisEntry || lhs.parentEntry != rhs.parentEntry  || lhs.rootEntry != rhs.rootEntry )
		throw std::exception( "Not the same Entries comparsion" );

#define FIELD_CMPR( X ) if( lhs.X != rhs.X ) return true;

	// plug-in
	FIELD_CMPR( pluginID );
	FIELD_CMPR( filmId );

	// вкладка фильм
	FIELD_CMPR( flags );
	FIELD_CMPR( title );
	FIELD_CMPR( titleAlt );

	FIELD_CMPR( year );
	FIELD_CMPR( rating );

	FIELD_CMPR( seriesData );

	FIELD_CMPR( filePath );
	FIELD_CMPR( urlLink );

	FIELD_CMPR( genres );
	FIELD_CMPR( date.dwLowDateTime );
	FIELD_CMPR( date.dwHighDateTime );
	FIELD_CMPR( numViews );

	// описание
	FIELD_CMPR( tagline );
	FIELD_CMPR( description );
	FIELD_CMPR( comment );
	FIELD_CMPR( tags );

	// постер

	FIELD_CMPR( posterId );

	// тех.инфо
	FIELD_CMPR( techInfo );

	// дополнительно
	//FIELD_CMPR( additional );



	// Редактируемые поля объектов идентичны
	return false;
}

DisplayItemComparator GetEntryPredicate( DisplayItemComparator current, CollectionDB * db, const Entry & entry )
{
	if( entry.sortType != sort_predicates::SortDefault )
		return sort_predicates::GetPredicate( entry.sortType );

	const Entry * curEntry = &entry;

	while( curEntry->thisEntry != curEntry->GetRootHandle() )
	{
		curEntry = db->FindEntry( curEntry->parentEntry );
		if( !curEntry )
			break;

		if( curEntry->sortType != sort_predicates::SortDefault )
			return sort_predicates::GetPredicate( curEntry->sortType );
	}

	return current;
}
