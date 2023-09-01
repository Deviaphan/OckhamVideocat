// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "AviableFiles.h"
#include "Entry.h"
#include "GlobalSettings.h"
#include "ResString.h"
#include "VideoCatDoc.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "resource.h"

bool PlayMovie( const wchar_t * fullPath, const wchar_t * player, const wchar_t * commandLine );

// воспроизведение видео или переход в папку
void OpenEntry( CommandInfo * info )
{
	VC_TRY;

	if( AllCommandsDisabled() )
		return;

	// защита от случайных воспроизведений и/или даблклика по кнопке воспроизведения (кликать можно не чаще чем раз в 0.7 секунды
	{
		static DWORD lastPlayTime = 0;

		if( (GetTickCount() - lastPlayTime) < 700 )
			return;

		lastPlayTime = GetTickCount();
	}

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	Entry * entry = info->entry;
	if( !entry )
		return;

	// для папок переходим в указанную папку
	if( entry->IsFolder() || entry->IsPerson() )
	{
		CVideoTreeView * treeView = doc->GetVideoTreeView();
		if( treeView )
			treeView->SelectItem( entry );

		return;
	}
		
	// для виртуальных фильмов, вместо файла на диске открываем гиперссылку, если она есть. В браузере по умолчанию
	if( entry->filePath.empty() )
	{
		if( !entry->urlLink.empty() )
		{
			::ShellExecute( nullptr, L"open", entry->urlLink.c_str(), nullptr, nullptr, SW_SHOWNORMAL );
		}

		return;
	}

	const std::wstring filePath = doc->GetMoviePath( *entry );

	bool result = false;

	const AviableFiles & avi = GetGlobal().aviabledFiles;
	const ExtData * extData = avi.GetExtData( filePath.c_str() );
	if( !extData )
	{
		result = PlayMovie( filePath.c_str(), L"", L"" );
	}
	else
	{
		if( Test(entry->flags, EntryTypes::IsFilm3D) && extData->ignoreIn3d )
		{
			const ExtData * extData3D = avi.Get3DData();
			if( extData3D && !extData3D->player.IsEmpty() )
				extData = extData3D;
		}

		result = PlayMovie( filePath.c_str(), extData->player, extData->commandLine );
	}

	if( result )
	{
		// увеличиваю счётчик просмотров
		entry->numViews++;
		doc->AddViewedFile( *entry );

		if( GetGlobal().GetCloseBeforePlay() )
			doc->OnCloseDocument();
	}

	VC_CATCH_ALL;
}


bool PlayMovie( const wchar_t * fullPath, const wchar_t * player, const wchar_t * commandLine )
{
	if( !PathFileExists( fullPath ) )
	{
		CTaskDialog taskDlg(
			fullPath
			, ResString( IDS_TASK_PLAY_HEADER )
			, ResString( IDS_ERROR_TASK_PLAY_TITLE )
			, TDCBF_CLOSE_BUTTON
			, 0
			, ResString( IDS_TASK_PLAY_INFO )
		);

		taskDlg.SetDialogWidth( 220 );
		taskDlg.SetMainIcon( TD_ERROR_ICON );
		taskDlg.SetFooterIcon( TD_INFORMATION_ICON );
		taskDlg.DoModal();

		return false;
	}

	if( player )
	{
		::ShellExecute( nullptr, L"open", fullPath, nullptr, nullptr, SW_SHOWNORMAL );
	}
	else
	{
		CString command;
		if( !commandLine )
		{
			command.Format( L"\"%s\"", fullPath ); // Добавляем кавычки вокруг пути, чтобы пробелы правильно обрабатывались
		}
		else
		{
			command = commandLine;
			command.Replace( L"%VIDEO%", fullPath );
		}

		::ShellExecute( nullptr, L"open", player, command, nullptr, SW_SHOWNORMAL );
	}

	return true;
}
