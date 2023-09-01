// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoTreeView.h"
#include "ResString.h"


void DeleteVirtualFilm( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	if( entry->IsFolder() )
		return;

	EntryHandle entryHandle = entry->thisEntry;
	if( !entryHandle.IsVirtual() || entryHandle.IsRoot() )
		return;	

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;
	
	if( IDYES != AfxMessageBox( ResFormat( IDS_CONFIRM_DELETE_FILM, entry->title.c_str() ), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST ) )
		return;

	EntryHandle parentHandle = entry->parentEntry;

	cdb->DeleteEntry( entryHandle );

	doc->InvalidateDatabase();

	Entry * parentEntry = cdb->FindEntry( parentHandle );
	doc->RebuildFilteredFiles( parentEntry );

	VC_CATCH_ALL;
}
