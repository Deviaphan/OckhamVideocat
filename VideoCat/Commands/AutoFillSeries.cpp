// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"

#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"
#include "Kinopoisk/LoadFilmInfo.h"


// заполнение эпизодов сериалов
void AutoFillSeries( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	if( !entry->IsTV() )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	LoadSeriesInfo( cdb, entry->thisEntry );

	doc->InvalidateDatabase();

	doc->GetVideoTreeView()->SelectItem( entry );

	VC_CATCH_ALL;
}
