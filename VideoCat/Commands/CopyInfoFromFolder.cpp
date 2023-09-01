// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "CopyFolderDataDlg.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"

void CopyInfoFromFolder( CommandInfo * info )
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

	CopyFolderDataDlg copyDlg;
	if( IDOK != copyDlg.DoModal() )
		return;

	if( copyDlg.data.poster )
	{
		CopyPosters( *cdb, *parentEntry );
		doc->InvalidateDatabase();
	}

	if( copyDlg.data.description || copyDlg.data.tags || copyDlg.data.comments )
	{
		CopyCards( *cdb, *parentEntry, copyDlg.data );

		doc->InvalidateDatabase();
	}

	doc->GetVideoTreeView()->SelectItem( parentEntry );

	VC_CATCH_ALL;
}
