// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"

#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"
#include "Kinopoisk/LoadFilmInfo.h"
#include "SelectAutofillPlugin.h"


// заполнение карточек фильмов
void AutoSetFilmInfo( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;
	
	if( !entry->IsFolder() )
		return;

	CSelectAutofillPlugin selectPlugin;

	if( selectPlugin.DoModal() != IDOK )
		return;

	CWaitCursor waiting;

	EntryHandle currentHandle = entry->thisEntry;
	EntryHandle rootHandle = entry->GetRootHandle();

	if( selectPlugin.GetPluginID() != PluginManual )
	{
		LoadFilmsInfo( cdb, currentHandle );
		doc->GetCurrentCollection()->ResortCollection( rootHandle );
	}
	else
	{
		MarkAsUserDefinedInfo( cdb, currentHandle );
	}

	// По идее указатель не должен поменяться, но на всякий случай лучше заново найду объект.
	entry = doc->GetCurrentCollection()->FindEntry( currentHandle );
	// на null можно не проверять, нулевой указатель можно передавать

	doc->InvalidateDatabase();
	doc->GetVideoTreeView()->RebuildTree();

	doc->GetVideoTreeView()->SelectItem( entry );
	doc->RebuildFilteredFiles( entry );

	VC_CATCH_ALL;
}
