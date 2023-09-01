// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"
#include "PrivateLock.h"

void PrivateLockFolder( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	if( !entry->IsFolder() )
		return;

	if( !LockFolder( *doc->GetCurrentCollection(), entry->thisEntry ) )
		return;

	doc->InvalidateDatabase();
	Entry * parent = doc->GetCurrentCollection()->FindEntry( entry->parentEntry );
	doc->GetVideoTreeView()->RebuildTree();
	doc->GetVideoTreeView()->SelectItem( parent );	

	VC_CATCH_ALL;
}
