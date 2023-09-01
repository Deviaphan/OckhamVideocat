// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "Entry.h"
#include "Kinopoisk/Kinopoisk.h"
#include "ResString.h"
#include "resource.h"

void OpenEntryURL( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	if( !entry->urlLink.empty() )
	{
		::ShellExecute( nullptr, L"open", entry->urlLink.c_str(), nullptr, nullptr, SW_SHOWNORMAL );
	}
	else
	{
		if( entry->filmId == NO_FILM_ID )
		{
			return;
		}

		Kinopoisk::OpenSite( entry->filmId );
	}

	VC_CATCH_ALL;
}
