﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoTreeView.h"
#include "ResString.h"


void DeleteVirtualFolder( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	EntryHandle entryHandle = entry->thisEntry;
	if( !entryHandle.IsVirtual() || entryHandle.IsRoot() )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	if( IDYES != AfxMessageBox( ResFormat( IDS_CONFIRM_DELETE_FOLDER, entry->title.c_str() ), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST ) )
		return;

	EntryHandle parentHandle = entry->parentEntry;

	cdb->DeleteEntry( entryHandle );

	doc->InvalidateDatabase();

	doc->GetVideoTreeView()->RebuildTree();

	Entry * parentEntry = cdb->FindEntry( parentHandle );
	doc->GetVideoTreeView()->SelectItem( parentEntry );

	VC_CATCH_ALL;
}
