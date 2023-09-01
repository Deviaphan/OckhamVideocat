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

#define ADD_ITEM( ID ) AddItem( (UINT_PTR)(ID), new adv_mfc::AdvancedTextItem( CommandExecute::Instance().GetCommand( ID ).commandTitle, CommandExecute::Instance().GetCommand( ID ).iconID ) )

#define ADD_SORT_MENU() AddItem( SortMenu( entry, menu.GetIconSize() ), new adv_mfc::AdvancedTextItem( ResString( IDS_SORT ), IconManager::Sorting ) )

#define ADD_SEPARATOR() AddSeparator( new adv_mfc::AdvancedTextItem )


void CreateMainMenu( adv_mfc::AdvancedMenu & menu,  CVideoCatDoc* doc )
{
	menu.ADD_ITEM( CommandID::ManageCollections );

	if( doc->Ready() )
	{
		menu.ADD_ITEM( CommandID::ManageRoots );

		menu.ADD_SEPARATOR();
		{
			adv_mfc::AdvancedMenu * statistic = new adv_mfc::AdvancedMenu( menu.GetIconSize() );
			statistic->CreatePopupMenu();
			statistic->ADD_ITEM( CommandID::SearchDuplicates );
			statistic->ADD_SEPARATOR();
			statistic->ADD_ITEM( CommandID::ShowGenreStatistic );
			statistic->ADD_ITEM( CommandID::ShowYearStatistic );

			menu.AddItem( statistic, new adv_mfc::AdvancedTextItem( ResString( IDS_STATISTICS ), IconManager::Statistics ) );
		}
	}

	menu.ADD_SEPARATOR();

	menu.ADD_ITEM( CommandID::ProgramSettings );	

	menu.ADD_ITEM( CommandID::OpenForum );
	menu.ADD_ITEM( CommandID::OpenDonates );
	menu.ADD_ITEM( CommandID::AboutVideocat );
}

adv_mfc::AdvancedMenu * SortMenu( const Entry & entry, int iconSize )
{
	adv_mfc::AdvancedMenu * sortMenu = new adv_mfc::AdvancedMenu( iconSize );
	sortMenu->CreatePopupMenu();

	sortMenu->ADD_ITEM( CommandID::SortByDefault );
	sortMenu->ADD_ITEM( CommandID::SortByYearDown );
	sortMenu->ADD_ITEM( CommandID::SortByYearUp );
	sortMenu->ADD_ITEM( CommandID::SortByRussianName );
	sortMenu->ADD_ITEM( CommandID::SortByOriginalName );
	sortMenu->ADD_ITEM( CommandID::SortByAddDate );
	sortMenu->ADD_ITEM( CommandID::SortByFilenameUp );
	sortMenu->ADD_ITEM( CommandID::SortByFilenameDown );

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


void CreateFolderMenu( adv_mfc::AdvancedMenu & menu, const Entry & entry, bool extendedMenu )
{
	if( entry.thisEntry.IsRoot() )
	{
		menu.ADD_ITEM( CommandID::ManageRoots );
		menu.ADD_SEPARATOR();
	}

	if( extendedMenu )
	{
		// синхронизация и прочее доступно только для реальной папки, иначе синхронизировать не с чем
		if( entry.thisEntry.IsReal() )
		{
			menu.ADD_ITEM( CommandID::SynchronizeFolder );
			menu.ADD_ITEM( CommandID::AutoSetFilmInfo );

			if( entry.IsTV() )
			{
				menu.ADD_ITEM( CommandID::AutoFillSeries );
			}

			menu.ADD_ITEM( CommandID::GenerateThumbnails );
			menu.ADD_ITEM( CommandID::CreatePosterFromScreenshot );
			menu.ADD_ITEM( CommandID::EditTechInfo );
			menu.ADD_SEPARATOR();
			menu.ADD_ITEM( CommandID::CopyInfoFromFolder );
			menu.ADD_SEPARATOR();

			menu.ADD_SORT_MENU();
			menu.ADD_SEPARATOR();

			menu.ADD_ITEM( CommandID::RenameFiles );
			menu.ADD_ITEM( CommandID::EraseFilmsInfo );
		}
		else if( entry.thisEntry.IsVirtual() )
		{
			menu.ADD_ITEM( CommandID::AddVirtualFolder );
			menu.ADD_ITEM( CommandID::AddVirtualFilm );
			menu.ADD_ITEM( CommandID::AddVirtualPerson );
			
			if( !entry.thisEntry.IsRoot() )
			{
				menu.ADD_SEPARATOR();
				menu.ADD_ITEM( CommandID::DeleteVirtualFolder );
			}
			menu.ADD_SEPARATOR();

			menu.ADD_SORT_MENU();
		}
		else if( entry.thisEntry.IsPerson() )
		{
			menu.ADD_ITEM( CommandID::UpdateVirtualPerson );
			menu.ADD_ITEM( CommandID::DeleteVirtualPerson );
			menu.ADD_SEPARATOR();

			menu.ADD_SORT_MENU();
		}
	}
	else
	{
		menu.ADD_ITEM( CommandID::EditSingleEntry );

		menu.ADD_SEPARATOR();

		menu.ADD_SEPARATOR();
		if( entry.IsPrivate() )
			menu.ADD_ITEM( CommandID::PrivateUnlockFolder );
		else
			menu.ADD_ITEM( CommandID::PrivateLockFolder );

		if( entry.thisEntry.IsReal() )
		{
			menu.ADD_SEPARATOR();

			// получить техническую информацию можно только для реального файла (файлов в папке)
			menu.ADD_ITEM( CommandID::EditTechInfo );
			menu.ADD_ITEM( CommandID::GenerateThumbnails );
			menu.ADD_SEPARATOR();

			menu.ADD_SORT_MENU();
			menu.ADD_SEPARATOR();

			menu.ADD_ITEM( CommandID::RenameFiles );
		}
		else if( entry.thisEntry.IsVirtual() )
		{
			menu.ADD_SEPARATOR();
			menu.ADD_ITEM( CommandID::AddVirtualFilm );
			menu.ADD_SEPARATOR();

			menu.ADD_SORT_MENU();
		}
		else if( entry.thisEntry.IsPerson() )
		{
			menu.ADD_SEPARATOR();
			menu.ADD_ITEM( CommandID::UpdateVirtualPerson );
			menu.ADD_SEPARATOR();

			menu.ADD_SORT_MENU();
		}
	}
}

void CreateFileMenu( adv_mfc::AdvancedMenu & menu, const Entry & entry )
{
	menu.ADD_ITEM( CommandID::OpenEntry );

	menu.ADD_ITEM( CommandID::EditSingleEntry );
	menu.ADD_SEPARATOR();

	menu.ADD_ITEM( CommandID::OpenEntryURL );
	menu.ADD_ITEM( CommandID::OpenYoutube );

	if( entry.thisEntry.IsReal() )
	{
		// получить техническую информацию можно только для реального файла (файлов в папке)
		menu.ADD_ITEM( CommandID::EditTechInfo );

		menu.ADD_ITEM( CommandID::GenerateThumbnails );
		menu.ADD_SEPARATOR();
		menu.ADD_ITEM( CommandID::RenameFile );
	}
	else if( entry.thisEntry.IsVirtual() )
	{
		menu.ADD_SEPARATOR();
		menu.ADD_ITEM( CommandID::DeleteVirtualFilm );
	}

	menu.SetDefaultItem( (UINT)CommandID::OpenEntry );
}

void CreatePersonMenu( adv_mfc::AdvancedMenu & menu, const Entry & entry )
{
	menu.ADD_ITEM( CommandID::AddVirtualPerson );

	if( entry.filmId != NO_FILM_ID )
	{
		menu.ADD_ITEM( CommandID::UpdateVirtualPerson );
		menu.ADD_ITEM( CommandID::DeleteVirtualPerson );
	}
}

void CreateImportExportMenu( CMenu & menu )
{
#define ADD_ITEM_M( ID ) AppendMenu( MF_STRING, (UINT_PTR)(ID), CommandExecute::Instance().GetCommand( ID ).commandTitle )

	menu.ADD_ITEM_M( CommandID::ExportToAndroid );
	menu.AppendMenu( MF_SEPARATOR, 0, L"" );
	menu.ADD_ITEM_M( CommandID::ExportToCSV );
	menu.AppendMenu( MF_SEPARATOR, 0, L"" );
	menu.ADD_ITEM_M( CommandID::ImportFromArchivid );
}
