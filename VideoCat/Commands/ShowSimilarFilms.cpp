// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "Entry.h"
#include "Kinopoisk/Kinopoisk.h"
#include "resource.h"

void ShowSimilarFilms( CommandInfo * info )
{
	VC_TRY;

	const Entry * entry = info->entry;
	if( !entry )
		return;

	if( entry->filmId == NO_FILM_ID )
	{
		AfxMessageBox( L"Фильм должен быть связан с Кинопоиском, чтобы найти похожие.", MB_OK | MB_ICONINFORMATION );
		return;
	}

	Kinopoisk::OpenSimilarSite( entry->filmId );

	VC_CATCH_ALL;
}
