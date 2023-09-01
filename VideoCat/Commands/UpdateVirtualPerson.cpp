// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include <filesystem>
#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoTreeView.h"
#include "ErrorDlg.h"
#include "ResString.h"
#include "resource.h"


// заполнение эпизодов сериалов
void UpdateVirtualPerson( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	if( !entry->IsPerson() )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	if( !UpdatePersonEntry( *cdb, entry->thisEntry ) )
		return;
	
	doc->InvalidateDatabase();

	doc->GetVideoTreeView()->RebuildTree();
	doc->GetVideoTreeView()->SelectItem( entry );	

	VC_CATCH( ... )
	{
		bool ignore = false;
		ShowError( L"Ошибка обновления данных о персоне", ignore );
	}
}
