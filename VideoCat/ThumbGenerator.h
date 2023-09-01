#pragma once

#include "EntryHandle.h"

class CollectionDB;

namespace ThumbsMode
{
	enum Enum
	{
		Disallow = 10,
		OnlyNew = 20,
		AllFiles = 30,

		Units1280p = 100,
		Units480p = 101,
		Units270p = 102,
	};
}

ThumbsMode::Enum GetThumbsGenerationMode( const CString & title, int & units, BOOL & autoselect );

void GenerateThumbnails( CollectionDB * cdb, const EntryHandle & root, ThumbsMode::Enum mode, int multiplier, BOOL autoselect );

void GeneratePostersFromThumbs( CollectionDB * cdb, const EntryHandle & root );
