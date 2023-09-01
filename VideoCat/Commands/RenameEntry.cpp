// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "RenameDialog.h"
#include "VideoTreeView.h"
#include "ProgressDlg.h"
#include "ResString.h"
#include <filesystem>
#include "GlobalSettings.h"
#include "resource.h"

void RenameFiles( CVideoCatDoc * doc, EntryHandle topHandle, std::vector<EntryHandle> & allFilms );

void RenameFile( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	// переименовать можно только реальный файл на диске
	if( !entry->IsFile() )
	{
		AfxMessageBox( L"Переменовать можно только существующий на диске файл", MB_OK | MB_ICONINFORMATION );
		return;
	}

	EntryHandle topHandle = entry->parentEntry;

	std::vector<EntryHandle> singleFilm = { entry->thisEntry };

	RenameFiles( doc, topHandle, singleFilm );

	VC_CATCH_ALL;
}


struct GetFilms: public Traverse::Base
{
	std::vector<EntryHandle> films;

	Traverse::Result Run( Entry & entry ) override
	{
		if( entry.IsFile() )
		{
			if( !Test(entry.flags, EntryTypes::InnerFile) || !entry.childs.empty() )
				films.push_back( entry.thisEntry );
		}

		return Traverse::CONTINUE;
	}
};

void RenameFiles( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	CollectionDB * db = doc->GetCurrentCollection();
	if( !db )
		return;	

	GetFilms allFilms;
	db->TraverseAll( entry->thisEntry, &allFilms );

	RenameFiles( doc, entry->thisEntry, allFilms.films );

	VC_CATCH_ALL;
}


void RenameFiles( CVideoCatDoc * doc, EntryHandle topHandle, std::vector<EntryHandle> & allFilms )
{
	if( allFilms.empty() )
	{
		AfxMessageBox( ResString(IDS_NO_FILES_TO_RENAME), MB_OK | MB_ICONINFORMATION );
		return;
	}

	CollectionDB * db = doc->GetCurrentCollection();
	if( !db )
		return;

	CRenameDialog renameDlg;
	renameDlg.numFiles = (unsigned)allFilms.size();

	renameDlg.exampleNames.reserve( allFilms.size() );

	const wchar_t episodeTemplate[] = L"s%02ue%02u";

	for( const EntryHandle & h : allFilms )
	{
		Entry * child = db->FindEntry( h );
		if( !child )
			continue;

		std::wstring episode;
		if( child->IsTV() )
		{
			// эпизоды всегда именутся в формате sXXeXX
			episode = ResFormat( episodeTemplate, child->GetSeason(), child->GetEpisode() );
		}

		renameDlg.exampleNames.emplace_back( child->title, child->titleAlt, child->year, episode );
	}

	if( renameDlg.DoModal() != IDOK )
		return;

	if( IDYES != AfxMessageBox( ResString(IDS_CONFIRM_RENAME_FILES), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST ) )
		return;

	CWaitCursor waiting;

	ProgressController pc;
	pc.Create( ResString(IDS_RENAMING_FILES) );
	pc.SetProgressCount( (int)allFilms.size() );

	int countRenamed = 0;

	for( const EntryHandle & h : allFilms )
	{
		Entry * child = db->FindEntry( h );
		if( child )
		{
			std::wstring episode;
			if( child->IsTV() )
			{
				episode = ResFormat( episodeTemplate, child->GetSeason(), child->GetEpisode() );
			}

			EntryHandle newHandle = db->RenameFile( h, BuildNewName( renameDlg.nameFormat, child->title, child->titleAlt, child->year, episode ) );
			if( newHandle != h )
				++countRenamed;
		}

		pc.NextStep();
	}
	   
	// после переименования нужно пометить базу на сохранение
	doc->InvalidateDatabase();
	doc->InvalidateProgressInfo();

	// все дескрипторы поменялись и указатели стали неправильными, поэтому нужно перестроить дерево и окн фильмов
	doc->GetVideoTreeView()->RebuildTree();
	doc->GetVideoTreeView()->SelectItem( db->FindEntry( topHandle ) );

	CString msg = ResFormat( IDS_FILE_RENAME_INFO, countRenamed, (int)allFilms.size() );
	AfxMessageBox( msg, MB_OK | MB_ICONINFORMATION | MB_TOPMOST );
}
