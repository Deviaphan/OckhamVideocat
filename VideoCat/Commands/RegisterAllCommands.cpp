// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "RegisterAllCommands.h"
#include "CommandExecute.h"
#include "IconManager.h"
#include "ResString.h"
#include "resource.h"

void RegisterAllCommands()
{
	CommandExecute & ce = CommandExecute::Instance();

#define COMMAND( ID_FUNC_NAME, ICON, TITLE ) extern void ID_FUNC_NAME(CommandInfo * info); ce.Add( CommandID::ID_FUNC_NAME, ID_FUNC_NAME, ICON, ResString(TITLE) );

	COMMAND( AboutVideocat, IconManager::About, IDS_ABOUT );
	COMMAND( AddVirtualFilm, IconManager::FilmAdd, IDS_ADD_FILM );
	COMMAND( AddVirtualFolder, IconManager::FolderAdd, IDS_ADD_FOLDER );
	COMMAND( AddVirtualPerson, IconManager::PersonAdd, IDS_ADD_PERSON );
	COMMAND( AutoFillSeries, IconManager::TVSeriesFill, IDS_AUTOFILL_SERIES );
	COMMAND( AutoSetFilmInfo, IconManager::EditEntry, IDS_AUTOFILL );
	COMMAND( CreatePosterFromScreenshot, IconManager::PosterFromScreenshot, IDS_SCREENSHOT );
	COMMAND( CopyInfoFromFolder, IconManager::SaveAs, IDS_COPY_FROM_FOLDER );
	COMMAND( DeleteVirtualFilm, IconManager::FilmRemove, IDS_REMOVE_FILM );
	COMMAND( DeleteVirtualFolder, IconManager::FolderRemove, IDS_REMOVE_FOLDER );
	COMMAND( DeleteVirtualPerson, IconManager::PersonRemove, IDS_REMOVE_PERSON );
	COMMAND( EditSingleEntry, IconManager::EditEntry, IDS_FILM_EDIT );
	COMMAND( EditTagInfo, IconManager::EditTagInfo, IDS_TAG_EDIT );
	COMMAND( EditTechInfo, IconManager::EditTechInformation, IDS_READ_FILE_INFO );
	COMMAND( EditToolbar, IconManager::Dot3, IDS_EDIT_TOOLBAR );
	COMMAND( EraseFilmsInfo, IconManager::Erase, IDS_ERASE_FOLDER );
	COMMAND( ExportToAndroid, IconManager::ExportToAndroid, IDS_EXPORT_ANDROID );
	COMMAND( ExportToCSV, IconManager::File, IDS_EXPORT_EXCEL );
	COMMAND( GenerateThumbnails, IconManager::UpdateThumbnails, IDS_UPDATE_THUMBNAILS );
	COMMAND( ImportFromArchivid, IconManager::CollectionCheck, IDS_IMPORT_FROM_ARCHIVID );
	COMMAND( ManageCollections, IconManager::Collections, IDS_COLLECTION );
	COMMAND( ManageRoots, IconManager::FolderClosed, IDS_ROOT_DIRECTORY );
	COMMAND( OpenEntry, IconManager::Play, IDS_PLAY );
	COMMAND( OpenEntryURL, IconManager::FilmLinkedPage, IDS_FILM_WEBPAGE );
	COMMAND( OpenForum, IconManager::HomePage, IDS_FEEDBACK );
	COMMAND( OpenYoutube, IconManager::YouTubeLink, IDS_YOUTUBE );
	COMMAND( OskFindByName, IconManager::OnscreenKeyboard, IDS_OSK_FIND_BY_NAME );
	COMMAND( PrivateLockFolder, IconManager::LockFolder, IDS_LOCK_FOLDER );
	COMMAND( PrivateUnlockFolder, IconManager::UnlockFolder, IDS_UNLOCK_FOLDER );
	COMMAND( ProgramSettings, IconManager::Settings, IDS_SETTINGS );
	COMMAND( RenameFile, IconManager::Rename, IDS_RENAME_FILE );
	COMMAND( RenameFiles, IconManager::Rename, IDS_RENAME_FILES );
	COMMAND( SearchDuplicates, IconManager::FindDuplicates, IDS_STATISTICS_DUPLICATES );
	COMMAND( ShowGenreStatistic, IconManager::Pie, IDS_STATISTICS_GENRE );
	COMMAND( ShowLastViewed, IconManager::LastView, IDS_LAST_VIEWED );
	COMMAND( ShowNewFilms, IconManager::NewFilm, IDS_SHOW_NEW_FILMS );
	COMMAND( ShowRandomFilms, IconManager::Random, IDS_RANDOM );
	COMMAND( ShowSimilarFilms, IconManager::SimilarFilms, IDS_SIMILAR_FILMS );
	COMMAND( ShowYearStatistic, IconManager::Pie, IDS_STATISTIC_YEAR );
	COMMAND( SynchronizeFolder, IconManager::FolderUpdate, IDS_SYNCHRONIZE );
	COMMAND( UpdateVirtualPerson, IconManager::PersonUpdate, IDS_UPDATE_PERSON );
	COMMAND( OpenDonates, IconManager::Donate, IDS_DONATE );


	// сортировка в выбранной папке
	COMMAND( SortByDefault, IconManager::Sorting, IDS_SORT_BY_DEFAULT );
	COMMAND( SortByYearDown, IconManager::SortYearDown, IDS_SORT_BY_YEAR_DOWN );
	COMMAND( SortByYearUp, IconManager::SortYearUp, IDS_SORT_BY_YEAR_UP );
	COMMAND( SortByRussianName, IconManager::SortRu, IDS_SORT_BY_RUSSIAN_NAME );
	COMMAND( SortByOriginalName, IconManager::SortEn, IDS_SORT_BY_ORIGINAL_NAME );
	COMMAND( SortByAddDate, IconManager::SortByDate, IDS_SORT_BY_DATE );
	COMMAND( SortByFilenameUp, IconManager::SortFilenameUp, IDS_SORT_BY_FILENAME_UP );
	COMMAND( SortByFilenameDown, IconManager::SortFilenameDown, IDS_SORT_BY_FILENAME_DOWN );
	// глобальная сортировка "по умолчанию" для системы
	COMMAND( SortGlobalByYearDown, IconManager::SortYearDown, IDS_SORT_GLOBAL_BY_YEAR_DOWN );
	COMMAND( SortGlobalByYearUp, IconManager::SortYearUp, IDS_SORT_GLOBAL_BY_YEAR_UP );
	COMMAND( SortGlobalByRussianName, IconManager::SortRu, IDS_SORT_GLOBAL_BY_RUSSIAN_NAME );
	COMMAND( SortGlobalByOriginalName, IconManager::SortEn, IDS_SORT_GLOBAL_BY_ORIGINAL_NAME );
	COMMAND( SortGlobalByAddDate, IconManager::SortByDate, IDS_SORT_GLOBAL_BY_DATE );
	COMMAND( SortGlobalByFilenameUp, IconManager::SortFilenameUp, IDS_SORT_GLOBAL_BY_FILENAME_UP );
	COMMAND( SortGlobalByFilenameDown, IconManager::SortFilenameDown, IDS_SORT_GLOBAL_BY_FILENAME_DOWN );

}
