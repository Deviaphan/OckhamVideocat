#pragma once

using IconID = unsigned int;
class IconManager
{
public:
	IconManager();
	~IconManager();
	void Destroy();

public:
	void Draw( Gdiplus::Graphics & gdi, IconID id, int posX, int posY, int size, COLORREF color );

	void DrawStars( Gdiplus::Graphics & gdi, unsigned int rating, int centerX, int topY, int size );

protected:
	Gdiplus::Bitmap * _iconMap;
	Gdiplus::Bitmap * _stars;

	static const int _size = 64; // ширина иконки

	int _numRows; // количество иконок по высоте (число строк)
	int _numCols; // количество иконок по ширине (число столбцов)
	

public:

	enum DefaultIconID
	{
		ProgramIcon = 0,
		About,

		CollectionAdd,
		CollectionRemove,
		Collections,
		CollectionCheck,
		CollectionEdit,
		PersonAdd,
		PersonRemove,
		PersonUpdate,
		Persons,
		FolderAdd,
		FolderRemove,
		FolderEdit,
		FolderClosed,
		FolderOpened,

		TVSeries,
		TVSeriesFill,
		Settings,
		FilmAdd,
		FilmRemove,
		EditEntry,
		MenuButton,
		YouTubeLink,
		VKLink,
		Save,
		SaveAs,
		FolderUpdate,
		Play,
		Random,
		LastView,
		NewFilm,

		Statistics,
		Sorting,
		SortRu,
		SortEn,
		SortByDate,
		SortFilenameUp,
		SortFilenameDown,
		SortYearDown,
		SortYearUp,
		ExportToAndroid,
		Rename,
		Erase,
		Pie,
		Refresh,
		EditTechInformation,
		EditTagInfo,

		HomePage,
		FindDuplicates,
		UpdateThumbnails,
		PosterFromScreenshot,
		LockFolder,
		UnlockFolder,
		FilmLinkedPage,
		File,
		GoBack,
		Shopping,
		Dot3,
		Tutorial,
		SimilarFilms,
		OnscreenKeyboard,
		Donate
	};
};

IconManager & GetIconManager();


