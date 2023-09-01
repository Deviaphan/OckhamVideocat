﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "PersonInfo.h"

#include <filesystem>
#include "AddPerson.h"
#include "CollectionDB.h"
#include "Net/CurlBuffer.h"
#include "GlobalSettings.h"
#include "Kinopoisk/Kinopoisk.h"
#include "License/Blowfish.h"
#include "License/KeySerialize.h"
#include "PosterDownloader.h"
#include "ResString.h"
#include "VideoCatDoc.h"
#include "poster.h"
#include "Traverse/TraverseCollection.h"

namespace Traverse
{
	struct FindPesonById: public Base
	{
	public:
		std::vector<EntryHandle> handles;
		FilmId personId;

	public:
		Result Run( Entry & entry ) override
		{
			if( entry.thisEntry.IsPerson() )
			{
				if( entry.filmId == personId )
				{
					handles.push_back( entry.thisEntry );
				}
			}

			return CONTINUE;
		}
	};
}


PersonSearchInfo GetPersonId( FilmId personId, std::vector<FilmId>* allFilms )
{
	CAddPerson dlg;
	dlg._personId = personId;
	if( allFilms )
		dlg._filmList = *allFilms;

	const INT_PTR result = dlg.DoModal();
	if( IDCANCEL == result )
	{
		if( allFilms )
			*allFilms = dlg._filmList;

		return PersonSearchInfo();
	}

	if( dlg._personId == NO_FILM_ID )
	{
		return PersonSearchInfo();
	}

	return PersonSearchInfo( { (FilmId)dlg._personId, result == IDOK ? FALSE : TRUE } );
}

EntryHandle AddPersonEntry( CVideoCatDoc & doc, CollectionDB & cdb, const EntryHandle & parentHandle )
{
	const PersonSearchInfo personId = GetPersonId( NO_FILM_ID, nullptr );
	if( personId.id == NO_FILM_ID )
		return EntryHandle::EMPTY;

	Traverse::FindPesonById fpbi;
	fpbi.personId = personId.id;

	cdb.TraverseAll( parentHandle, &fpbi );

	if( !fpbi.handles.empty() )
	{
		std::wstring personName;
		Entry * entry = cdb.FindEntry( fpbi.handles[0] );
		if( entry )
		{
			personName = entry->title + L" / " + entry->titleAlt;
		}

		CString message;
		message.Format( L"В папке уже есть такая персона:\r\n%s\r\n\r\nВсё равно продолжить добавление?", personName.c_str() );

		if( IDNO == AfxMessageBox( message, MB_YESNO | MB_ICONQUESTION ) )
			return EntryHandle::EMPTY;
	}

	CWaitCursor waiting;


	Entry * parentEntry = cdb.FindEntry( parentHandle );
	if( !parentEntry )
		return EntryHandle::EMPTY;

	PersonInfo personInfo;
	Entry newPerson;

	if( personId.directLoad )
	{
		Kinopoisk::DownloadPersonInfoDirect( personId.id, personInfo.allFilms, newPerson );
	}
	else
	{
		Kinopoisk::DownloadPersonInfo( personId.id, personInfo.allFilms, newPerson );
	}

	// Не удалось получить информацию о персоне
	if( newPerson.filmId != personId.id )
		return EntryHandle::EMPTY;

	if( newPerson.title.empty() && newPerson.titleAlt.empty() )
	{
		AfxMessageBox( L"Не удалось получить данные о персоне", MB_OK | MB_ICONERROR );
		return EntryHandle::EMPTY;
	}

	newPerson.parentEntry = parentHandle;
	newPerson.rootEntry = cdb.FindEntry(parentHandle)->GetRootHandle();
	Set( newPerson.flags, EntryTypes::IsFolder );
	newPerson.thisEntry.type = EntryHandle::IS_PERSON;
	newPerson.thisEntry.hash = cdb.GetNextVirtualIndex();
	newPerson.urlLink = ResFormat( L"https://www.kinopoisk.ru/name/%u/", personId.id );

	personInfo.thisEntry = newPerson.thisEntry;

	// загружаем постер для персоны
	{
		PosterManager & pm = cdb.GetPosterManager();

		CStringA posterUrl;
		posterUrl.Format( "https://st.kp.yandex.net/images/actor_iphone/iphone360_%u.jpg", newPerson.filmId );

		std::vector<std::string> imageUrls;
		imageUrls.emplace_back( posterUrl.GetString() );

		Curl::Global cGuard;
		Curl curl( 15, false );

		Gdiplus::Bitmap * poster = nullptr;
		if( DownloadImage( curl, imageUrls, poster ) )
		{
			newPerson.posterId = pm.SaveImage( poster );
			delete poster;
		}
	}

	parentEntry->AddChild( newPerson.thisEntry );

	doc.AddPerson( newPerson, personInfo );

	cdb.UpdatePersons();

	return newPerson.thisEntry;
}

bool UpdatePersonEntry( CollectionDB & cdb, const EntryHandle & personHandle )
{
	PersonInfo * personInfo = cdb.FindPerson( personHandle );
	if( !personInfo )
		return false;

	Entry * personEntry = cdb.FindEntry( personHandle );
	if( !personEntry )
		return false;

	const PersonSearchInfo personId = GetPersonId( personEntry->filmId, &personInfo->allFilms );
	if( personId.id == NO_FILM_ID )
	{
		cdb.UpdatePersons();
		return true;
	}

	CWaitCursor waiting;

	PersonInfo updateInfo;
	Entry updatePerson;

	if( personId.directLoad )
	{
		Kinopoisk::DownloadPersonInfoDirect( personId.id, updateInfo.allFilms, updatePerson );
	}
	else
	{
		Kinopoisk::DownloadPersonInfo( personId.id, updateInfo.allFilms, updatePerson );
	}

	if( updatePerson.filmId != personEntry->filmId )
		return false;

	if( updatePerson.title.empty() && updatePerson.titleAlt.empty() )
	{
		AfxMessageBox( L"Не удалось получить данные о персоне", MB_OK | MB_ICONERROR );
		return false;
	}
	
	personEntry->title = updatePerson.title;
	personEntry->titleAlt = updatePerson.titleAlt;
	personEntry->urlLink = ResFormat( L"https://www.kinopoisk.ru/name/%u/", personId.id );

	personInfo->allFilms.swap( updateInfo.allFilms );

	// загружаем постер для персоны
	if( personEntry->posterId == DefaultPosterID )
	{
		PosterManager & pm = cdb.GetPosterManager();

		CStringA posterUrl;
		posterUrl.Format( "https://st.kp.yandex.net/images/actor_iphone/iphone360_%u.jpg", personEntry->filmId );

		std::vector<std::string> imageUrls;
		imageUrls.emplace_back( posterUrl.GetString() );

		Curl::Global cGuard;
		Curl curl( 15, false );

		Gdiplus::Bitmap * poster = nullptr;
		if( DownloadImage( curl, imageUrls, poster ) )
		{
			personEntry->posterId = pm.SaveImage( poster );
			delete poster;
		}
	}

	cdb.UpdatePersons();

	return true;
}

