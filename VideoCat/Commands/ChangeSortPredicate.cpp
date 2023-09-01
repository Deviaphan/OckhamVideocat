// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"

#include "VideoCatDoc.h"
#include "Entry.h"
#include "SortPredicates.h"

void ChangeGlobalSortPredicate( CVideoCatDoc * doc, sort_predicates::SortType sortType )
{
	VC_TRY;

	if( !doc )
		return;

	doc->SetSortPredicate( sortType );

	doc->InvalidateDatabase();

	VC_CATCH_ALL;
}

void ChangeFolderSortPredicate( CVideoCatDoc * doc, Entry * entry, sort_predicates::SortType sortType )
{
	VC_TRY;

	if( !doc )
		return;

	if( !entry )
		return;

	if( !entry->IsFolder() && !entry->thisEntry.IsPerson() )
		return;

	entry->sortType = sortType;

	doc->RebuildFilteredFiles( entry );

	doc->InvalidateDatabase();

	VC_CATCH_ALL;
}

void SortByDefault( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortDefault );
}

void SortByYearDown( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortYearDown );
}

void SortByYearUp( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortYearUp );
}

void SortByRussianName( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortRu );
}

void SortByOriginalName( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortEn );
}

void SortByAddDate( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortDate );
}

void SortByFilenameUp( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortFileUp );
}

void SortByFilenameDown( CommandInfo * info )
{
	ChangeFolderSortPredicate( info->doc, info->entry, sort_predicates::SortFileDown );
}


void SortGlobalByYearDown( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortYearDown );
}

void SortGlobalByYearUp( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortYearUp );
}

void SortGlobalByRussianName( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortRu );
}

void SortGlobalByOriginalName( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortEn );
}

void SortGlobalByAddDate( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortDate );
}

void SortGlobalByFilenameUp( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortFileUp );
}

void SortGlobalByFilenameDown( CommandInfo * info )
{
	ChangeGlobalSortPredicate( info->doc, sort_predicates::SortFileDown );
}
