// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include <filesystem>
#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoTreeView.h"
#include "GlobalSettings.h"


void AddVirtualPerson( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * parentEntry = info->entry;
	if( !parentEntry )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	if( parentEntry->IsPerson() )
	{
		// если выбрана персона, то пытаемся получить родительскую папку и добавить персону в эту папку
		parentEntry = cdb->FindEntry( parentEntry->parentEntry );
		if( !parentEntry )
			throw (int)1;
	}

	if( !parentEntry->thisEntry.IsVirtual() )
	{
		throw (int)2;
	}

	if( !parentEntry->IsFolder() && !parentEntry->thisEntry.IsRoot() )
		throw (int)3;

	const EntryHandle newEntry = AddPersonEntry( *doc, *cdb, parentEntry->thisEntry );
	if( newEntry != EntryHandle::EMPTY )
	{
		doc->GetVideoTreeView()->RebuildTree();
		doc->GetVideoTreeView()->SelectItem( parentEntry );
	}

	VC_CATCH( ... )
	{
		AfxMessageBox( L"Ошибка добавления персоны.\r\nДобавить персону можно только в виртуальную папку.", MB_OK | MB_ICONINFORMATION );
	}
}
