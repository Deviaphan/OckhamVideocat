// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "ResString.h"
#include "ThumbGenerator.h"
#include "resource.h"

void GenerateThumbnails( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;
	
	Entry * entry = info->entry;
	if( !entry )
		return;

	if( entry->thisEntry.IsPerson() )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;
		
	int multiplier = 1;
	BOOL autoselect = !entry->IsFolder();
	ThumbsMode::Enum mode = GetThumbsGenerationMode( entry->title.c_str(), multiplier, autoselect );

	if( mode == ThumbsMode::Disallow )
		return;

	// для конкретного файла всегда пересоздаём, даже если существует
	if( !entry->IsFolder() )
		mode = ThumbsMode::AllFiles;

	::GenerateThumbnails( cdb, entry->thisEntry, mode, multiplier, entry->IsFolder() ? TRUE : autoselect );

	VC_CATCH_ALL;
}
