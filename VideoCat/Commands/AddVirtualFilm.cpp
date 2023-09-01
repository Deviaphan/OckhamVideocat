// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include <filesystem>
#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "VideoTreeView.h"
#include "EditCard.h"
#include "GlobalSettings.h"
#include "License/KeySerialize.h"
#include "License/Blowfish.h"
#include "PrivateLock.h"
#include "ResString.h"
#include "resource.h"


// заполнение эпизодов сериалов
void AddVirtualFilm( CommandInfo * info )
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
		throw (int)1;
	}	

	if( !parentEntry->IsFolder() && !parentEntry->thisEntry.IsRoot() )
	{
		// если выбран фильм, то пытаемся получить родительскую папку и добавить фильм в эту папку
		parentEntry = cdb->FindEntry( parentEntry->parentEntry );
		if( !parentEntry )
			throw (int)2;

		if( !parentEntry->IsFolder() && !parentEntry->thisEntry.IsRoot() )
			throw (int)3;
	}

	// Если папка защищена паролем, сперва нужно ввести пароль
	if( parentEntry->IsPrivate() && cdb->PrivateLocked() )
	{
		const CString & syncPassword = cdb->GetPassword();

		CPrivateLock lockDlg;
		lockDlg.hideDeleteLock = true;
		if( syncPassword.IsEmpty() )
			lockDlg.hideCollectionPass = true;

		if( IDOK != lockDlg.DoModal() )
			return;

		if( syncPassword != lockDlg.collectionPassword )
			return;

		if( cdb->GetPrivatePassword() != lockDlg.privatePassword )
			return;

		cdb->UnlockPrivate( lockDlg.privatePassword );
	}

	Entry newEntry;
	newEntry.parentEntry = parentEntry->thisEntry;
	newEntry.rootEntry = parentEntry->GetRootHandle();
	newEntry.thisEntry.type = EntryHandle::IS_VIRTUAL;
	newEntry.thisEntry.hash = cdb->GetNextVirtualIndex();

	parentEntry->AddChild( newEntry.thisEntry );

	cdb->InsertEntry( newEntry, false );
	
	CEditCard dlg;
	dlg.SetDocument( doc );
	dlg.SetEntry( cdb->FindEntry( newEntry.thisEntry ) );
	dlg.SetDefaultTab( CardTabName::FilmInfo );

	if( dlg.DoModal() != IDOK )
	{
		cdb->DeleteEntry( newEntry.thisEntry );
	}

	doc->InvalidateDatabase();

	CWaitCursor wait;

	if( dlg.GetEntry()->filmId != NO_FILM_ID )
	{
		// если фильм распознан, то обновляем папки актёров, чтобы этот фильм попал в список
		cdb->UpdatePersons();
	}

	doc->RebuildFilteredFiles( parentEntry );
	//	RefreshDisplayList( item._info );

	VC_CATCH( ... )
	{		
		AfxMessageBox( L"Ошибка добавления фильма.\r\nДобавить фильм можно только в виртуальную папку.", MB_OK | MB_ICONINFORMATION );
	}
}
