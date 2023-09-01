// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "VideoCat.h"
#include "VideoCatDoc.h"
#include "GlobalSettings.h"
#include "CardFilmInfo.h"
#include "Entry.h"
#include "Kinopoisk/Kinopoisk.h"
#include "SynchronizeFiles.h"
#include "ErrorDlg.h"
#include "ResString.h"
#include "resource.h"


void BrowseForFolderCtrl::OnBrowse()
{
	if( ::PathFileExists( openFolder ) )
	{
		::ShellExecute( nullptr, L"open", openFolder, nullptr, nullptr, SW_SHOWNORMAL );
	}
	else
	{
		CTaskDialog taskDlg(
			openFolder
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
	}
}

IMPLEMENT_DYNAMIC( CCardFilmInfo, CMFCPropertyPage )

CCardFilmInfo::CCardFilmInfo()
	: CMFCPropertyPage( IDD_TAB_FILM_INFO )
	, _titleYear( 0 )
	, _film3D( FALSE )
	, _season(0)
	, _episode(0)
	, _rating( 0 )
	, _numViews( 0 )
{
	ZeroMemory(&_date, sizeof(FILETIME));

}

CCardFilmInfo::~CCardFilmInfo()
{
}

void CCardFilmInfo::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange( pDX );

	DDX_Text( pDX, IDC_TITLE_RUSSIAN, _titleRussian );
	DDX_Text( pDX, IDC_TITLE_ORIGINAL, _titleOriginal );
	DDX_Text( pDX, IDC_TITLE_YEAR, _titleYear );
	DDX_Check( pDX, IDC_CHECK_3DMOVIE, _film3D );
	DDX_Control( pDX, IDC_OBJECT_PATH, _filmPath );

	DDX_Text( pDX, IDC_SEASON, _season );
	DDX_Text( pDX, IDC_EPISODE, _episode );

	DDX_Control( pDX, IDC_GENRES, _genres );

	DDX_Control( pDX, IDC_URL, _browseUrl );
	DDX_Text( pDX, IDC_EDIT_RATING, _rating );
	DDX_DateTimeCtrl( pDX, IDC_DATE, _date );
	DDX_Text( pDX, IDC_NUM_VIEWS, _numViews );
}


BEGIN_MESSAGE_MAP(CCardFilmInfo, CMFCPropertyPage)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


BOOL CCardFilmInfo::OnInitDialog()
{
	__super::OnInitDialog();

	return TRUE;
}

void CCardFilmInfo::ReinitTab()
{
	VC_TRY;

	__super::ReinitTab();

	_titleRussian = entry->title.c_str();
	_titleOriginal = entry->titleAlt.c_str();
	_titleYear = entry->year;
	_rating = (double)entry->rating / 10.0;

	if( entry->GetRootHandle() != entry->thisEntry )
	{
		Entry * rootEntry = doc->GetCurrentCollection()->FindEntry( entry->GetRootHandle() );
		_filmFullPath = rootEntry->filePath.c_str();
	}

	_filmFullPath += entry->filePath.c_str();

	if( entry->IsFolder() )
	{
		_filmPath.openFolder = _filmFullPath;
	}
	else
	{
		int pos = _filmFullPath.ReverseFind( L'\\' );
		if( pos > 0 )
		{
			_filmPath.openFolder = _filmFullPath.Left( pos );
		}
		else
		{
			_filmPath.openFolder = _filmFullPath;
		}
	}

	if( entry->seriesData )
	{
		_season = entry->GetSeason();
		_episode = entry->GetEpisode();
	}

	_film3D = entry->IsFilm3D();

	_date = entry->date;
	_numViews = entry->numViews;

	if( entry->seriesData )
	{
		GetDlgItem( IDC_SEASON )->EnableWindow( TRUE );
		GetDlgItem( IDC_EPISODE )->EnableWindow( TRUE );
	}

	if( !entry->IsFolder() && entry->thisEntry.IsVirtual() )
	{
		_filmPath.SetReadOnly( FALSE );
	}

	_filmPath.SetWindowText( _filmFullPath );
	_browseUrl.SetWindowText( entry->urlLink.c_str() );

	RefreshGenreList( entry->genres );

	UpdateData( FALSE );

	VC_CATCH( ... )
	{
		;
	}
}

bool CCardFilmInfo::SaveData()
{
	VC_TRY;

	if( !initialized )
		return false;

	UpdateData();

	entry->title = _titleRussian;
	entry->titleAlt = _titleOriginal;
	entry->year = (unsigned short)_titleYear;
	entry->rating = (std::min)((unsigned char)100, (unsigned char)(_rating * 10));

	entry->urlLink = _browseUrl.GetURL();
	if( !entry->IsFolder() && entry->thisEntry.IsVirtual() )
	{
		CString txt;
		_filmPath.GetWindowText( txt );
		entry->filePath = txt;
	}

	// проверяем только изменения в жанрах, пропуская остальные флаги
	{
		entry->genres = EmptyGenres;

		for( int i = 0; i < _genres.GetCount(); ++i )
		{
			CString genreName;
			_genres.GetText( i, genreName );
			if( _genres.GetCheck( i ) )
			{
				Genre genre = (Genre)_genres.GetItemData( i );
				Set( entry->genres, genre );
			}
		}
	}

	if( _film3D )
		Set( entry->flags, EntryTypes::IsFilm3D );
	else
		Clear( entry->flags, EntryTypes::IsFilm3D );

	entry->SetEpisode( _season, _episode );

	entry->date = _date;
	entry->numViews = _numViews;

	return true;

	VC_CATCH( ... )
	{
		return false;
	}
}


void CCardFilmInfo::RefreshGenreList( Genres genres )
{
	_genres.ResetContent();
	_genres.SetCheckStyle( BS_AUTOCHECKBOX );

	for( int i = (int)Genre::FirstGenre; i < (int)Genre::NumOfGenres; ++i )
	{
		Genre genre = (Genre)i;

		int itemIndex = _genres.AddString( Descript( genre ) );
		_genres.SetItemData( itemIndex, (DWORD_PTR)i );
		_genres.SetCheck( itemIndex, Test( genres, genre ) );
	}
}

void CCardFilmInfo::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if( bShow == FALSE )
		SaveData();

	__super::OnShowWindow( bShow, nStatus );
}
