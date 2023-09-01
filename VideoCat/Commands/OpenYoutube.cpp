// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "Entry.h"
#include "ResString.h"
#include "resource.h"

void OpenYoutube( CommandInfo * info )
{
	VC_TRY;

	if( !info->entry )
		return;

	CString url;
	if( info->entry->year )
	{
		url = ResFormat( IDS_YOUTUBE_SEARCH_REVIEW_YEAR, info->entry->title.c_str(), (unsigned int)info->entry->year );
	}
	else
	{
		url = ResFormat( IDS_YOUTUBE_SEARCH_REVIEW, info->entry->title.c_str() );
	}

	url.Replace( L'"', L'\'' );

	::ShellExecute( nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL );

	VC_CATCH_ALL;
}
