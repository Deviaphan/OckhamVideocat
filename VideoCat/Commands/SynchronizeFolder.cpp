// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "CommandExecute.h"

#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"
#include "Kinopoisk/LoadFilmInfo.h"
#include "SelectAutofillPlugin.h"
#include "SynchronizeFiles.h"


// Синхронизировать содержимое папки в каталоге с папкой на диске
void SynchronizeFolder( CommandInfo * info )
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

	if( entry->thisEntry.IsVirtual() || (!entry->IsFolder() && !entry->thisEntry.IsRoot() ) )
	{
		AfxMessageBox( L"Синхронизация возможна только для папок, существующих на диске.", MB_OK | MB_ICONINFORMATION );
		return;
	}

	EntryHandle currentHandle = entry->thisEntry;
	//const CString title = !entry->nameRU.IsEmpty() ? entry->nameRU : entry->nameEN;

	SynchronizeFiles sf( cdb );
	if( !sf.Process( *entry ) )
		return;

	int numMarked = 0;
	if( sf.autoFill )
	{
		if( sf.fillPlugin != PluginManual )
		{
			CommandExecute::Instance().Process( CommandID::AutoSetFilmInfo , info );
		}
		else
		{
			numMarked = MarkAsUserDefinedInfo( cdb, currentHandle );
		}
	}

	if( sf.fillTechInfo )
	{
		CommandExecute::Instance().Process( CommandID::EditTechInfo, info );
	}

	if( sf.generateThumbs )
	{
		CommandExecute::Instance().Process( CommandID::GenerateThumbnails, info );
	}

	doc->InvalidateDatabase();
	doc->GetVideoTreeView()->RebuildTree();

	doc->GetVideoTreeView()->SelectItem( entry );
	doc->RebuildFilteredFiles( entry );

	VC_CATCH_ALL;
}
