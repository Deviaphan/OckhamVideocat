// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "ThumbGenerator.h"

void CreatePosterFromScreenshot( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	// кадры есть только у реально существующих фильмов
	if( !entry->thisEntry.IsReal() )
	{
		AfxMessageBox( L"Получить кадры можно только для фильмов на диске", MB_OK | MB_ICONINFORMATION );
		return;
	}

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	GeneratePostersFromThumbs( cdb, entry->thisEntry );

	doc->InvalidateDatabase();
	doc->RebuildFilteredFiles( entry );

	VC_CATCH_ALL;
}
