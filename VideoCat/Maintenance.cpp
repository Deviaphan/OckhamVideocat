// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CollectionDB.h"

#include <set>
#include <algorithm>
#include <filesystem>

void GetAllChilds( CollectionDB & cdb, Entry & parent, std::set<EntryHandle> & allEntries )
{
	// проверяю наличие дубликатов. Из-за ошибок такое могло произойти
	std::sort( parent.childs.begin(), parent.childs.end() );
	auto iiUnique = std::unique( parent.childs.begin(), parent.childs.end() );
	parent.childs.erase( iiUnique, parent.childs.end() );

	for( const EntryHandle & eh : parent.childs )
	{
		Entry * entry = cdb.FindEntry( eh );
		if( !entry )
			continue;

		allEntries.insert( eh );

		GetAllChilds( cdb, *entry, allEntries );
	}
}

int CollectionDB::CleanBadEntry( std::set<EntryHandle> & allEntries )
{
	allEntries.clear();

	for( auto & root : _roots )
	{
		Entry * entry = FindEntry( root );
		if( !entry )
			continue;

		allEntries.insert( root );

		GetAllChilds( *this, *entry, allEntries );
	}

	if( _entries.size() == allEntries.size() )
		return 0;

	for( auto ii = _entries.begin(); ii != _entries.end();  )
	{
		if( allEntries.find( ii->thisEntry ) == allEntries.end() )
		{
			ii = _entries.erase( ii );
		}
		else
		{
			++ii;
		}
	}

	return (int)(_entries.size() - allEntries.size());
}

int CollectionDB::CleanBadPoster()
{
	std::set<PosterID> usedPosters;

	for( const Entry & entry : _entries )
	{
		usedPosters.insert( entry.posterId );
	}

	return GetPosterManager().Maintenance( usedPosters );
}

int CollectionDB::CleanBadFrames( const std::set<EntryHandle> & allEntries, const CString & thumbDir )
{
	// Находим все файлы скриншотов в папке, затем находим те, которые больше не используются

	std::set<std::wstring> allNames;

	for( const EntryHandle & eh : allEntries )
	{
		allNames.emplace( eh.ToBase64().GetString() );
	}

	std::filesystem::path searchPath( thumbDir.GetString() );

	std::vector<std::filesystem::path> oldThumbs;

	std::error_code errCode;
	for( const std::filesystem::directory_entry & item : std::filesystem::directory_iterator( searchPath, errCode ) )
	{
		const std::wstring filename = item.path().stem().wstring();
		if( allNames.find( filename ) == allNames.end() )
		{
			oldThumbs.push_back( item.path() );
		}
	}

	for( const auto & p : oldThumbs )
	{
		std::filesystem::remove( p, errCode );
	}

	return (int)oldThumbs.size();
}
