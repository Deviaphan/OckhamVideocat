// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "CollectionDB.h"
#include "DuplicateDlg.h"
#include "ResString.h"
#include "VideoCatDoc.h"
#include "ShowPie.h"
#include "resource.h"

struct Duplicates: public Traverse::Base
{
	std::map< std::pair<PluginID, FilmId>, std::vector<Entry *> > data;

	virtual Traverse::Result Run( Entry & entry ) override
	{
		if( entry.filmId != NO_FILM_ID )
		{
			data[std::make_pair( entry.pluginID, entry.filmId )].push_back( &entry );
		}

		return Traverse::CONTINUE;
	}
};


void SearchDuplicates( CommandInfo * info )
{
	VC_TRY;

	CWaitCursor waiting;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;	   

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	Duplicates duplicates;
	cdb->ForEach( &duplicates );

	CDuplicateDlg dlg;
	dlg.cdb = cdb;
	for( auto & item : duplicates.data )
	{
		if( item.second.size() > 1 )
		{
			dlg.data[item.first.second].swap( item.second );
		}
	}

	if( dlg.data.empty() )
		AfxMessageBox( ResString( IDS_NO_DUPLICATES ), MB_OK | MB_TOPMOST );
	else
		dlg.DoModal();

	VC_CATCH_ALL;
}

void ShowGenreStatistic( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;


	ShowGenrePie( cdb );

	VC_CATCH_ALL;
}

void ShowYearStatistic( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	ShowYearPie( cdb );

	VC_CATCH_ALL;
}
