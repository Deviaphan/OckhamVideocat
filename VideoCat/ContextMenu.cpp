// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ContextMenu.h"
#include "ResString.h"
#include "IconManager.h"
#include "SortPredicates.h"
#include "IconManager.h"
#include "Commands/CommandExecute.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ADD_SORT_MENU() AddItem( SortMenu( entry, menu.GetIconSize() ), new adv_mfc::AdvancedTextItem( ResString( IDS_SORT ), IconManager::Sorting ) )


void CreateMainMenu( adv_mfc::AdvancedMenu& menu, CVideoCatDoc* doc )
{
	menu.AddItem( CommandID::ManageCollections );

	if( doc->Ready() )
	{
		menu.AddItem( CommandID::ManageRoots );

		menu.AddSeparator();
		{
			adv_mfc::AdvancedMenu* statistic = new adv_mfc::AdvancedMenu( menu.GetIconSize() );
			statistic->CreatePopupMenu();
			statistic->AddItem( CommandID::SearchDuplicates );
			statistic->AddSeparator();
			statistic->AddItem( CommandID::ShowGenreStatistic );
			statistic->AddItem( CommandID::ShowYearStatistic );

			menu.AddItem( statistic, new adv_mfc::AdvancedTextItem( ResString( IDS_STATISTICS ), IconManager::Statistics ) );
		}
	}

	menu.AddSeparator();

	menu.AddItem( CommandID::ProgramSettings );

	menu.AddItem( CommandID::OpenForum );
	menu.AddItem( CommandID::OpenDonates );
	menu.AddItem( CommandID::AboutVideocat );
}

adv_mfc::AdvancedMenu* SortMenu( const Entry& entry, int iconSize )
{
	adv_mfc::AdvancedMenu* sortMenu = new adv_mfc::AdvancedMenu( iconSize );
	sortMenu->CreatePopupMenu();

	sortMenu->AddItem( CommandID::SortByDefault );
	sortMenu->AddItem( CommandID::SortByYearDown );
	sortMenu->AddItem( CommandID::SortByYearUp );
	sortMenu->AddItem( CommandID::SortByRussianName );
	sortMenu->AddItem( CommandID::SortByOriginalName );
	sortMenu->AddItem( CommandID::SortByAddDate );
	sortMenu->AddItem( CommandID::SortByFilenameUp );
	sortMenu->AddItem( CommandID::SortByFilenameDown );

	static const CommandID convertSort[] = {
		CommandID::SortByYearDown, // SortYearDown = 0
		CommandID::SortByYearUp, // SortYearUp
		CommandID::SortByRussianName, // SortRu
		CommandID::SortByOriginalName, // SortEn
		CommandID::SortByAddDate, // SortDate
		CommandID::SortByFilenameUp, // SortFileUp
		CommandID::SortByFilenameDown // SortFileDown
		// SortDefault = 0xff
	};

	sortMenu->SetDefaultItem( (entry.sortType < _countof( convertSort )) ? (UINT)convertSort[entry.sortType] : (UINT)CommandID::SortByDefault );

	return sortMenu;
}


void CreateFolderMenu( adv_mfc::AdvancedMenu& menu, const Entry& entry, bool extendedMenu )
{
	if( entry.thisEntry.IsRoot() )
	{
		menu.AddItem( CommandID::ManageRoots );
		menu.AddSeparator();
	}

	if( extendedMenu )
	{
		// синхронизация и прочее доступно только для реальной папки, иначе синхронизировать не с чем
		if( entry.thisEntry.IsReal() )
		{
			menu.AddItem( CommandID::SynchronizeFolder );
			menu.AddItem( CommandID::AutoSetFilmInfo );

			if( entry.IsTV() )
			{
				menu.AddItem( CommandID::AutoFillSeries );
			}

			menu.AddItem( CommandID::GenerateThumbnails );
			menu.AddItem( CommandID::CreatePosterFromScreenshot );
			menu.AddItem( CommandID::EditTechInfo );
			menu.AddSeparator();
			menu.AddItem( CommandID::CopyInfoFromFolder );
			menu.AddSeparator();

			menu.ADD_SORT_MENU();
			menu.AddSeparator();

			menu.AddItem( CommandID::RenameFiles );
			menu.AddItem( CommandID::EraseFilmsInfo );
		}
		else if( entry.thisEntry.IsVirtual() )
		{
			menu.AddItem( CommandID::AddVirtualFolder );
			menu.AddItem( CommandID::AddVirtualFilm );
			menu.AddItem( CommandID::AddVirtualPerson );

			if( !entry.thisEntry.IsRoot() )
			{
				menu.AddSeparator();
				menu.AddItem( CommandID::DeleteVirtualFolder );
			}
			menu.AddSeparator();

			menu.ADD_SORT_MENU();
		}
		else if( entry.thisEntry.IsPerson() )
		{
			menu.AddItem( CommandID::UpdateVirtualPerson );
			menu.AddItem( CommandID::DeleteVirtualPerson );
			menu.AddSeparator();

			menu.ADD_SORT_MENU();
		}
	}
	else
	{
		menu.AddItem( CommandID::EditSingleEntry );

		menu.AddSeparator();

		menu.AddSeparator();
		if( entry.IsPrivate() )
			menu.AddItem( CommandID::PrivateUnlockFolder );
		else
			menu.AddItem( CommandID::PrivateLockFolder );

		if( entry.thisEntry.IsReal() )
		{
			menu.AddSeparator();

			// получить техническую информацию можно только для реального файла (файлов в папке)
			menu.AddItem( CommandID::EditTechInfo );
			menu.AddItem( CommandID::GenerateThumbnails );
			menu.AddSeparator();

			menu.ADD_SORT_MENU();
			menu.AddSeparator();

			menu.AddItem( CommandID::RenameFiles );
		}
		else if( entry.thisEntry.IsVirtual() )
		{
			menu.AddSeparator();
			menu.AddItem( CommandID::AddVirtualFilm );
			menu.AddSeparator();

			menu.ADD_SORT_MENU();
		}
		else if( entry.thisEntry.IsPerson() )
		{
			menu.AddSeparator();
			menu.AddItem( CommandID::UpdateVirtualPerson );
			menu.AddSeparator();

			menu.ADD_SORT_MENU();
		}
	}
}

void CreateFileMenu( adv_mfc::AdvancedMenu& menu, const Entry& entry )
{
	menu.AddItem( CommandID::OpenEntry );

	menu.AddItem( CommandID::EditSingleEntry );
	menu.AddSeparator();

	menu.AddItem( CommandID::OpenEntryURL );
	menu.AddItem( CommandID::OpenYoutube );

	if( entry.thisEntry.IsReal() )
	{
		// получить техническую информацию можно только для реального файла (файлов в папке)
		menu.AddItem( CommandID::EditTechInfo );

		menu.AddItem( CommandID::GenerateThumbnails );
		menu.AddSeparator();
		menu.AddItem( CommandID::RenameFile );
	}
	else if( entry.thisEntry.IsVirtual() )
	{
		menu.AddSeparator();
		menu.AddItem( CommandID::DeleteVirtualFilm );
	}

	menu.SetDefaultItem( (UINT)CommandID::OpenEntry );
}

void CreatePersonMenu( adv_mfc::AdvancedMenu& menu, const Entry& entry )
{
	menu.AddItem( CommandID::AddVirtualPerson );

	if( entry.filmId != NO_FILM_ID )
	{
		menu.AddItem( CommandID::UpdateVirtualPerson );
		menu.AddItem( CommandID::DeleteVirtualPerson );
	}
}

void CreateImportExportMenu( CMenu& menu )
{
	constexpr auto ADD_ITEM_M = []( CMenu& menu, CommandID ID )
	{
		menu.AppendMenu( MF_STRING, (UINT_PTR)(ID), CommandExecute::Instance().GetCommand( ID ).commandTitle );
	};

	ADD_ITEM_M( menu, CommandID::ExportToAndroid );
	menu.AppendMenu( MF_SEPARATOR, 0, L"" );
	ADD_ITEM_M( menu, CommandID::ExportToCSV );
	menu.AppendMenu( MF_SEPARATOR, 0, L"" );
	ADD_ITEM_M( menu, CommandID::ImportFromArchivid );
}
