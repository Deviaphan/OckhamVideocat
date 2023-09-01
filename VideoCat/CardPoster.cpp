// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CardPoster.h"
#include "PosterDownloader.h"
#include "LoadPng.h"
#include "VideoCatDoc.h"
#include "ResString.h"
#include "CameraViewDlg.h"
#include "resource.h"

enum PosterMode
{
	CreateByName = 0,
	CreateFromUrl,
	CreateFromFile,
	CreateFromClipboard,
	NoChange,
	DeletePoster
};

// поиск по имени - открывает непосредственно страницу актёра
// https://www.kinopoisk.ru/index.php?first=yes&kp_query=%D1%E8%E3%F3%F0%ED%E8+%F3%E8%E2%E5%F0

IMPLEMENT_DYNAMIC( CCardPoster, CMFCPropertyPage )

CCardPoster::CCardPoster()
	: CMFCPropertyPage( IDD_TAB_POSTER )
	, _posterMode( PosterMode::NoChange )
	, _usedPosterMode( PosterMode::NoChange )
	, _oldPosterID( DefaultPosterID )
	, _preview( nullptr )
{

}

CCardPoster::~CCardPoster()
{
	delete _preview;
	_preview = nullptr;
}

void CCardPoster::DoDataExchange( CDataExchange * pDX )
{
	CMFCPropertyPage::DoDataExchange( pDX );
	DDX_Radio( pDX, IDC_RADIO_POSTER_1, _posterMode );

	DDX_Text( pDX, IDC_EDIT_POSTER_URL, _posterUrl );
	DDX_Control( pDX, IDC_POSTER_PATH, _posterPath );

}


BEGIN_MESSAGE_MAP( CCardPoster, CMFCPropertyPage )
	ON_BN_CLICKED( IDC_LOAD_POSTER, &CCardPoster::OnBnClickedLoadPoster )
	ON_BN_CLICKED( IDC_SAVE_TO_DISK, &CCardPoster::OnSaveToDisk )
	ON_EN_SETFOCUS( IDC_EDIT_POSTER_URL, &CCardPoster::OnEnSetfocusEditPosterUrl )
	ON_EN_SETFOCUS( IDC_POSTER_PATH, &CCardPoster::OnEnSetfocusPosterPath )
	ON_WM_PAINT()
	ON_BN_CLICKED( IDC_COPY_FROM_CLIPBOARD, &CCardPoster::OnCopyFromClipboard )
	ON_BN_CLICKED( IDC_COPY_WEBCAM, &CCardPoster::OnCopyWebcam )
END_MESSAGE_MAP()


BOOL CCardPoster::OnInitDialog()
{
	VC_TRY;

	CMFCPropertyPage::OnInitDialog();

	_posterPath.EnableFileBrowseButton( ResString( IDS_FILE_FILTER_IMAGES ) );

	_oldPosterID = entry->posterId;

	OnBnClickedLoadPoster();

	VC_CATCH( ... )
	{
		return FALSE;
	}

	return TRUE;
}

void CCardPoster::OnEnSetfocusEditPosterUrl()
{
	UpdateData();
	_posterMode = PosterMode::CreateFromUrl;
	UpdateData( FALSE );
}


void CCardPoster::OnEnSetfocusPosterPath()
{
	UpdateData();
	_posterMode = PosterMode::CreateFromFile;
	UpdateData( FALSE );
}

void CCardPoster::ReinitTab()
{
	__super::ReinitTab();

	if( _posterMode != _usedPosterMode )
		OnBnClickedLoadPoster();
}

bool CCardPoster::SaveData()
{
	if( !initialized )
		return false;

	if( _usedPosterMode == PosterMode::NoChange )
		return false;

	if( !doc )
		return false;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return false;

	cdb->GetPosterManager().RemovePoster( entry->posterId );

	entry->posterId = DefaultPosterID;

	if( _usedPosterMode != PosterMode::DeletePoster )
	{
		entry->posterId = cdb->GetPosterManager().SaveImage( _preview );
	}

	return true;
}

void CCardPoster::OnPaint()
{
	if( !_preview )
		return;

	CPaintDC dc( this );

	CRect imgRect;
	GetDlgItem( IDC_POSTER_IMG )->GetWindowRect( imgRect );
	ScreenToClient( imgRect );

	std::unique_ptr<Gdiplus::Graphics> gdi( Gdiplus::Graphics::FromHDC( dc.GetSafeHdc() ) );

	Gdiplus::RectF dest( imgRect.left, imgRect.top, _preview->GetWidth(), _preview->GetHeight() );

	gdi->DrawImage( _preview, dest, 0, 0, _preview->GetWidth(), _preview->GetHeight(), Gdiplus::Unit::UnitPixel );
}

void CCardPoster::LoadPosterById()
{
	initialized = true;

	delete _preview;
	_preview = nullptr;

	if( entry->filmId == NO_FILM_ID )
	{
		_preview = LoadPng( IDB_NOPOSTER );

		_posterMode = PosterMode::DeletePoster;
		_usedPosterMode = _posterMode;
	}
	else
	{
		CStringA url;
		std::vector<std::string> pathList;

		url.Format( "http://st.kp.yandex.net/images/film_big/%u.jpg", entry->filmId );
		pathList.emplace_back( url.GetString() );

		url.Format( "http://st.kp.yandex.net/images/film/%u.jpg", entry->filmId );
		pathList.emplace_back( url.GetString() );

		Curl::Global guard;
		Curl curl( 15, false );
		if( DownloadImage( curl, pathList, _preview ) )
		{
			_posterMode = PosterMode::CreateByName;
			_usedPosterMode = _posterMode;
		}
		else
		{
			_preview = LoadPng( IDB_NOPOSTER );

			_posterMode = PosterMode::DeletePoster;
			_usedPosterMode = _posterMode;
		}
	}

	UpdateData( FALSE );
}

void CCardPoster::OnBnClickedLoadPoster()
{
	VC_TRY;
	CWaitCursor waiting;

	UpdateData();

	delete _preview;
	_preview = nullptr;

	switch( _posterMode )
	{
		case PosterMode::CreateByName:
			LoadPosterById();
			break;

		case PosterMode::CreateFromUrl:
		{
			if( _posterUrl.IsEmpty() )
			{
				AfxMessageBox( ResString( IDS_ERROR_NO_IMAGE_URL ), MB_OK | MB_TOPMOST | MB_SETFOREGROUND );
				return;
			}

			CStringA url( _posterUrl );
			std::vector<std::string> pathList;
			pathList.emplace_back( url.GetString() );

			Curl::Global guard;
			Curl curl( 15, false );
			DownloadImage( curl, pathList, _preview );
			break;

		}

		case PosterMode::CreateFromFile:
		{
			CString imagePath;
			_posterPath.GetWindowText( imagePath );

			if( imagePath.IsEmpty() )
			{
				AfxMessageBox( ResString( IDS_ERROR_NO_IMAGE_PATH ), MB_OK | MB_TOPMOST );
				return;
			}

			std::unique_ptr<Gdiplus::Bitmap> fileImage( Gdiplus::Bitmap::FromFile( imagePath ) );
			ResizePoster( fileImage.get(), _preview );

			break;
		}

		case PosterMode::CreateFromClipboard:
		{
			OnCopyFromClipboard();
			break;
		}

		case PosterMode::DeletePoster:
		{
			_preview = LoadPng( IDB_NOPOSTER );
			break;
		}

		default:
		{
			if( !doc )
				return;

			CollectionDB * cdb = doc->GetCurrentCollection();
			if( !cdb )
				return;

			cdb->GetPosterManager().LoadPoster( entry->posterId, _preview );
			if( !_preview )
			{
				_preview = LoadPng( IDB_NOPOSTER );
			}

			break;
		}
	}

	_usedPosterMode = _posterMode;

	Invalidate();
	VC_CATCH( ... )
	{}
}


void CCardPoster::OnCopyFromClipboard()
{
	VC_TRY;
	CWaitCursor waiting;

	delete _preview;
	_preview = nullptr;

	std::unique_ptr<Gdiplus::Bitmap> clipImage;

	if( OpenClipboard() )
	{
		HBITMAP hBitmap = (HBITMAP)GetClipboardData( CF_BITMAP );
		if( hBitmap )
		{
			clipImage.reset( Gdiplus::Bitmap::FromHBITMAP( hBitmap, nullptr ) );
		}

		CloseClipboard();
	}

	if( clipImage )
	{
		ResizePoster( clipImage.get(), _preview );

		_posterMode = PosterMode::CreateFromClipboard;
	}
	else
	{
		_posterMode = PosterMode::NoChange;
	}
	

	VC_CATCH( ... )
	{
	}
}

void CCardPoster::OnSaveToDisk()
{
	if( !_preview )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();

	CString filename;
	filename.Format( L"T:\\%i.jpg", (int)entry->posterId );

	unsigned crc32 = 0;

	cdb->GetPosterManager().SaveToFile( entry->posterId, filename, crc32 );

}


void CCardPoster::OnCopyWebcam()
{
	_posterMode = PosterMode::CreateFromClipboard;
	
	UpdateData(FALSE);

	CameraViewDlg dlg;
	if( dlg.DoModal() == IDOK )
	{
		OnBnClickedLoadPoster();
	}
}
