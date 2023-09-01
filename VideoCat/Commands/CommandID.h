#pragma once


enum class CommandID: int
{

	None = 0,

	OpenEntry = 1,

	AboutVideocat,

	ManageCollections,
	ManageRoots,

	ImportFromArchivid,
	ExportToAndroid,
	ExportToCSV,


	// --- Команды, работающие только с реальными файлами и папками
	SynchronizeFolder,
	AutoSetFilmInfo,
	AutoFillSeries,
	AutoDownloadPosters,
	GenerateThumbnails,
	CreatePosterFromScreenshot,
	EraseFilmsInfo,
	CopyInfoFromFolder,
	RenameFile,
	RenameFiles,

	// ----------------

	// --- Команды, доступные только для персон
	AddVirtualPerson,
	DeleteVirtualPerson,
	UpdateVirtualPerson,
	// ----------------

	// --- Команды, доступные и для реальных и для виртуальных файлов
	EditSingleEntry,
	EditTagInfo,
	OpenEntryURL,
	OpenYoutube,
	ShowSimilarFilms,
	PrivateLockFolder,
	PrivateUnlockFolder,
	// ---------------

	// ---
	AddVirtualFolder,
	AddVirtualFilm,
	DeleteVirtualFilm,
	DeleteVirtualFolder,
	// ---------------

	ShowLastViewed,
	ShowRandomFilms,
	ShowNewFilms,

	ShowGenreStatistic,
	ShowYearStatistic,
	SearchDuplicates,
	EditTechInfo,

	SortByDefault,
	SortByYearDown,
	SortByYearUp,
	SortByRussianName,
	SortByOriginalName,
	SortByAddDate,
	SortByFilenameUp,
	SortByFilenameDown,
	//
	SortGlobalByYearDown,
	SortGlobalByYearUp,
	SortGlobalByRussianName,
	SortGlobalByOriginalName,
	SortGlobalByAddDate,
	SortGlobalByFilenameUp,
	SortGlobalByFilenameDown,

	OskFindByName,

	OpenForum,
	ProgramSettings,

	EditToolbar,

	OpenDonates,

	SortGlobal = 1000,
};

