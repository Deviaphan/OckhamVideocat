// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include <filesystem>
#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoTreeView.h"
#include "EditCard.h"
#include "ResString.h"
#include "resource.h"


// заполнение эпизодов сериалов
void AddVirtualFolder( CommandInfo * info )
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
		throw (int)1;

	if( !parentEntry->IsFolder() && !parentEntry->thisEntry.IsRoot() )
	{
		// если выбран фильм, то пытаемся получить родительскую папку и добавить папку в эту папку
		parentEntry = cdb->FindEntry( parentEntry->parentEntry );
		if( !parentEntry )
			throw (int)2;

		if( !parentEntry->IsFolder() && !parentEntry->thisEntry.IsRoot() )
			throw (int)3;
	}

	Entry newFolder;
	newFolder.parentEntry = parentEntry->thisEntry;
	newFolder.rootEntry = parentEntry->GetRootHandle();
	Set( newFolder.flags, EntryTypes::IsFolder );
	newFolder.thisEntry.type = EntryHandle::IS_VIRTUAL;
	newFolder.thisEntry.hash = cdb->GetNextVirtualIndex();

	parentEntry->AddChild( newFolder.thisEntry );

	cdb->InsertEntry( newFolder, false );
	
	CEditCard dlg;
	dlg.SetDocument( doc );
	dlg.SetEntry( cdb->FindEntry( newFolder.thisEntry ) );
	dlg.SetDefaultTab( CardTabName::FilmInfo );
	if( dlg.DoModal() != IDOK )
	{
		cdb->DeleteEntry( newFolder.thisEntry );
	}

	doc->InvalidateDatabase();

	CWaitCursor wait;

	if( dlg.GetEntry()->filmId != NO_FILM_ID )
	{
		// если фильм распознан, то обновляем папки актёров, чтобы этот фильм попал в список
		cdb->UpdatePersons();
	}

	doc->GetVideoTreeView()->RebuildTree();
	doc->GetVideoTreeView()->SelectItem( parentEntry );

	VC_CATCH(...)
	{
		AfxMessageBox( L"Ошибка добавления папки.\r\nДобавить папку можно только в виртуальную директорию или в другую виртуальную папку.", MB_OK | MB_ICONINFORMATION );
	}
}
