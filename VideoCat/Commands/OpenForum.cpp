// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "ResString.h"
#include "resource.h"

class CommandInfo;

void OpenForum( CommandInfo * )
{
	VC_TRY;

	::ShellExecute( nullptr, L"open", ResString( IDS_SITE_FORUM ), nullptr, nullptr, SW_SHOWNORMAL );

	VC_CATCH_ALL;
}

void OpenDonates( CommandInfo* )
{
	VC_TRY;

	::ShellExecute( nullptr, L"open", L"https://videocat.maagames.ru/purchase.html", nullptr, nullptr, SW_SHOWNORMAL );

	VC_CATCH_ALL;
}

