// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "EraseDlg.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"

void EraseFilmsInfo( CommandInfo * info )
{
	VC_TRY;

	CWaitCursor waiting;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * parentEntry = info->entry;
	if( !parentEntry )
		return;

	// пакетная очистка только для фильмов в папках
	if( !parentEntry->IsFolder() )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	CEraseDlg eraseDlg;
	if( IDOK != eraseDlg.DoModal() )
		return;

	if( eraseDlg.data.erasePosters )
	{
		ErasePosters( *cdb, *parentEntry );
		doc->InvalidateDatabase();
	}

	if( eraseDlg.data.eraseThumbs )
		EraseThumbs( *cdb, *parentEntry );

	if( eraseDlg.data.eraseKinopoisk || eraseDlg.data.eraseTags || eraseDlg.data.eraseCounters || eraseDlg.data.eraseTechInfo )
	{
		if( eraseDlg.data.eraseKinopoisk || eraseDlg.data.eraseTags || eraseDlg.data.eraseTechInfo )
			doc->InvalidateDatabase();

		if( eraseDlg.data.eraseCounters )
			doc->InvalidateProgressInfo();

		EraseCards( *cdb, *parentEntry,	eraseDlg.data );
	}

	doc->GetVideoTreeView()->SelectItem( parentEntry );

	VC_CATCH_ALL;
}
