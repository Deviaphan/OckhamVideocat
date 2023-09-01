// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThumbsDlg.h"
#include <xiosbase>
#include <process.h>
#include <string>
#include <fstream>

#include "MediaInfo.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum
{
	THUMBNAIL_WIDTH = 96,
	THUMBNAIL_HEIGHT = 54,
	NUM_MANUAL_FRAMES = 72
};

static const CString g_ffmpeg = L"ffmpeg.exe";
static const CString g_shotName = L"01.png";


int GetEncoderClsid( const wchar_t * format, CLSID * pClsid )
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::GetImageEncodersSize( &num, &size );
	if( size == 0 )
		return -1;  // Failure

	Gdiplus::ImageCodecInfo * pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc( size ));
	if( pImageCodecInfo == nullptr )
		return -1;  // Failure

	Gdiplus::GetImageEncoders( num, size, pImageCodecInfo );

	for( UINT j = 0; j < num; ++j )
	{
		if( wcscmp( pImageCodecInfo[j].MimeType, format ) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free( pImageCodecInfo );
			return j;  // Success
		}
	}

	free( pImageCodecInfo );
	return -1;  // Failure
}

void SaveJ( const CImage & image, const CString & thumbsName )
{
		ULONG quality = 80;

		Gdiplus::Bitmap bitmap( (HBITMAP)image, nullptr );

		Gdiplus::REAL dpi = 96;
		bitmap.SetResolution( dpi, dpi );

		CLSID encoderClsid;
		GetEncoderClsid( L"image/jpeg", &encoderClsid );

		Gdiplus::EncoderParameters encoderParameters;

		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		encoderParameters.Parameter[0].Value = &quality;

		//Gdiplus::Status stat =
		bitmap.Save( thumbsName, &encoderClsid, &encoderParameters );
}

CString GetCurrentModulePath()
{
	wchar_t path[MAX_PATH];
	HMODULE thisModule = nullptr;
	VERIFY( GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&__FUNCTION__, &thisModule ) );
	VERIFY( GetModuleFileName( thisModule, path, MAX_PATH ) );
	VERIFY( PathRemoveFileSpec( path ) );
	CString currentDir = path;
	currentDir += L"\\";
	return currentDir;
}

CThumbsDlg::CThumbsDlg( CWnd* pParent /*=nullptr*/ )
	: CDialog( IDD_THUMBS_DIALOG, pParent )
	, _workingThread( INVALID_HANDLE_VALUE )
	, _currentStep( 0 )
	, _totalSteps( 0 )
	, _units( 1 )
	, _autoselect( 1 )
	, _selectedItem( -1 )
	, _skip( false )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void CThumbsDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDIT, _editCtrl );
	DDX_Control( pDX, IDC_PROGRESS, _progressCtrl );
	DDX_Control( pDX, IDC_LIST, _listCtrl );
	DDX_Control( pDX, IDC_LIST_FRAMES, _listFrames );
}

BEGIN_MESSAGE_MAP( CThumbsDlg, CDialog )
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_LIST_FRAMES, OnItemchangedListFrames )
	ON_BN_CLICKED( IDC_BTN_CREATE_THUMBS, OnBtnCreateThumbs )
	ON_BN_CLICKED( IDC_BTN_SKIP, OnBtnSkip )
END_MESSAGE_MAP()

void CThumbsDlg::CleanThread()
{
	_workingThread = INVALID_HANDLE_VALUE;
}

void __cdecl FilmThread( void * ptr )
{
	CThumbsDlg * dlg = (CThumbsDlg*)ptr;
	if( !dlg )
		return;

	dlg->ProcessFilms();
	dlg->CleanThread();

	dlg->PostMessage( WM_CLOSE, 0, 0 );
}

void __cdecl MultiframeFilmThread( void * ptr )
{
	CThumbsDlg * dlg = (CThumbsDlg*)ptr;
	if( !dlg )
		return;

	bool result = dlg->ProcessFilm();
	dlg->CleanThread();
}

BOOL CThumbsDlg::OnInitDialog()
{
	try
	{
		CDialog::OnInitDialog();

		SetIcon( m_hIcon, TRUE );
		SetIcon( m_hIcon, FALSE );

		if( _filePath.IsEmpty() )
			return FALSE;

		if( !PathFileExists( _filePath ) )
			return FALSE;

		_imageListThumb.Create( THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, ILC_COLOR24, 0, 1 );

		_listFrames.SetExtendedStyle( _listFrames.GetStyle() | LVS_EX_CHECKBOXES );
		_listFrames.SetImageList( &_imageListThumb, LVSIL_NORMAL );


		wchar_t unicodeBuffer[MAX_PATH];

		FilmInfo fi;

		std::string line;
		std::ifstream fin( _filePath, std::ios_base::in );

		std::getline( fin, line );
		if( line.empty() )
			return FALSE;

		int count = MultiByteToWideChar( CP_UTF8, 0, line.c_str(), line.size() + 1, unicodeBuffer, MAX_PATH );
		unicodeBuffer[MAX_PATH - 1] = 0;
		_thumbsDir = unicodeBuffer;


		std::getline( fin, line );
		if( line.empty() )
			return FALSE;
		_autoselect = atoi( line.c_str() );


		std::getline( fin, line );
		if( line.empty() )
			return FALSE;
		_units = atoi( line.c_str() );


		while( fin )
		{
			std::getline( fin, line );
			if( line.empty() )
				break;

			count = MultiByteToWideChar( CP_UTF8, 0, line.c_str(), line.size() + 1, unicodeBuffer, MAX_PATH );
			unicodeBuffer[MAX_PATH - 1] = 0;
			fi.path = unicodeBuffer;

			std::getline( fin, line );
			if( line.empty() )
				break;

			fi.hash = line.c_str();
			_films.push_back( fi );
		};

		_currentStep = 0;
		_totalSteps = (int)_films.size();

		_progressCtrl.SetRange( 0, (short)_totalSteps * (_autoselect ? 10 : NUM_MANUAL_FRAMES) );
		_progressCtrl.SetPos( 0 );
		_progressCtrl.SetStep( 1 );

		_workingThread = (HANDLE)_beginthread( _autoselect ? FilmThread : MultiframeFilmThread, 0, this );
		if( _workingThread == INVALID_HANDLE_VALUE )
			return FALSE;

		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

void CThumbsDlg::OnPaint()
{
	if( IsIconic() )
	{
		CPaintDC dc( this );

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0 );

		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect( &rect );
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon( x, y, m_hIcon );
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CThumbsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Узнать продолжительность видео в секундах
unsigned GetVideoLength( const CString & fileName )
{
	return media::GetVideoLength( fileName );
}

bool GenerateThumbnail( const CString & currentDir, const CString & utilPath, const CString & fileName, const CString & tempDir, unsigned shotTime )
{
	wchar_t cmdLine[2048];
	//wsprintf( cmdLine, L"%s -slave \"%s\" -really-quiet -nosound -vo jpeg:outdir=\\\"%s\\\" -frames 1 -ss %u", utilPath.GetString(), fileName.GetString(), tempDir.GetString(), shotTime );

	wsprintf( cmdLine, L"\"%s\" -ss %u -noaccurate_seek -i \"%s\" -vframes 1 -compression_level 0 \"%s\"", utilPath.GetString(), shotTime, fileName.GetString(), tempDir.GetString() );

	PROCESS_INFORMATION piProcInfo;
	ZeroMemory( &piProcInfo, sizeof( PROCESS_INFORMATION ) );
	STARTUPINFO siStartInfo;
	ZeroMemory( &siStartInfo, sizeof( STARTUPINFO ) );
	siStartInfo.cb = sizeof( STARTUPINFO );

	BOOL bSuccess = CreateProcess(
		utilPath.GetString(),
		cmdLine,
		nullptr,          // process security attributes 
		nullptr,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,             // creation flags 
		nullptr,          // use parent's environment 
		currentDir.GetString(),          // use parent's current directory 
		&siStartInfo,
		&piProcInfo
	);

	if( !bSuccess )
		return false;

	if( ::WaitForSingleObject( piProcInfo.hProcess, 20000 ) == WAIT_TIMEOUT )
		return false;

	return true;
}

bool CThumbsDlg::ProcessFilms()
{
	try
	{
		CString currentDir = GetCurrentModulePath();

		media::SetMediaInfoPath( (currentDir + L"MediaInfo.dll").GetString() );

		CreateDirectory( _thumbsDir, nullptr );

		const CString mplayer = currentDir + g_ffmpeg;

		if( !::PathFileExists( mplayer.GetString() ) )
		{
			CString errorMsg;
			errorMsg.Format( L"Невозможно получить кадры, отсутствует файл:\n%s", mplayer.GetString() );
			AfxMessageBox( errorMsg, MB_OK | MB_ICONERROR | MB_TOPMOST );
			return false;
		}

		wchar_t path[MAX_PATH];
		GetTempPath( MAX_PATH, path );
		CString tempDir = path;
		tempDir += L"vc_thumb\\";

		const CString screenName = tempDir + g_shotName;

		CPen blackPen( PS_SOLID, 1, RGB( 0, 0, 0 ) );

		bool hasErrors = false;

		static const std::pair<int,int> resolutions[4] =
		{
			{ 100000, 0 },
			{ 1280, 720 },
			{ 854, 480 },
			{ 480, 270 }
		};

		int frameWidth = resolutions[2].first;
		int frameHeight = resolutions[2].second;

		for( const FilmInfo & filmInfo : _films )
		{
			++_currentStep;
			CString titleText;
			titleText.Format( L"Обрабатывается %i из %i фильмов", _currentStep, _totalSteps );
			_editCtrl.SetWindowText( titleText );
			_listCtrl.AddString( filmInfo.path );

			wchar_t shortName[MAX_PATH] = {};
			GetShortPathName( filmInfo.path.GetString(), shortName, MAX_PATH );
			if( shortName[0] == 0 )
			{
				_listCtrl.AddString( L"--- Ошибка. Файл не найден." );
				hasErrors = true;

				for( int i = 0; i < 10; ++i )
					_progressCtrl.StepIt();

				continue;
			}

			//получение длины не всегда срабатывает. Сделаем вид, что длина фильма 90 минут
			unsigned videoLength = GetVideoLength( filmInfo.path );
			if( videoLength == 0 )
				videoLength = 11 * 490;

			const unsigned shotOffset = videoLength / 11;

			CRect dest;

			CImage thumbs;
			HDC dc = nullptr;
			bool noThumbs = false;

			CRect srcRect;

			for( int i = 1; i < 11; ++i )
			{
				::DeleteFile( screenName );

				if( !GenerateThumbnail( currentDir, mplayer, filmInfo.path, screenName, shotOffset * i ) )
				{
					noThumbs = true;
					break;
				}

				CImage src;
				src.Load( screenName );
				if( src.IsNull() )
				{
					noThumbs = true;
					break;
				}

				if( i == 1 )
				{
					srcRect.SetRect( 0, 0, src.GetWidth(), src.GetHeight() );

					if( _units == 1 )
					{
						frameWidth = src.GetWidth();
						frameHeight = (frameWidth * 720) / 1280;
					}
					else
					{
						for( int unitIndex = 3; unitIndex >= (_units - 1); --unitIndex )
						{
							if( srcRect.Width() < resolutions[unitIndex].first )
								break;

							frameWidth = resolutions[unitIndex].first;
							frameHeight = resolutions[unitIndex].second;
						}
					}

					if( srcRect.Width() != frameWidth || srcRect.Height() != frameHeight )
					{
						if( ((float)srcRect.Width() / (float)srcRect.Height()) <= (1280.0f / 720.0f) )
						{
							// меньший аспект, чем 1280*720
							float scale = (float)srcRect.Width() / (float)frameWidth;
							int h = (int)(frameHeight * scale);
							int offset = abs( h - srcRect.Height() ) / 2;
							srcRect.top += offset;
							srcRect.bottom -= offset;
						}
						else
						{
							// более широкий, чем 1280*720

							float scale = (float)srcRect.Height() / (float)frameHeight;
							int w = (int)(frameWidth * scale);
							int offset = abs( w - srcRect.Width() ) / 2;
							srcRect.left += offset;
							srcRect.right -= offset;
						}
					}

					dest.SetRect( 0, 0, frameWidth, frameHeight );

					thumbs.Create( frameWidth, frameHeight * 10, 24 );
					dc = thumbs.GetDC();
					::SelectObject( dc, blackPen.GetSafeHandle() );
				}

				if( _skip )
					break;

				{
					// Рисуем текущий кадр, чтобы было веселее ждать
					CClientDC frameDc( this );
					frameDc.SetStretchBltMode( COLORONCOLOR );

					CRect rect;
					CWnd * wnd = GetDlgItem( IDC_PREVIEW );
					wnd->GetWindowRect( rect );
					ScreenToClient( rect );

					src.StretchBlt( frameDc.GetSafeHdc(), rect, SRCCOPY );
				}

				::SetStretchBltMode( dc, HALFTONE );

				src.StretchBlt( dc, dest, srcRect, SRCCOPY );

				dest.OffsetRect( 0, frameHeight );

				_progressCtrl.StepIt();
			}

			if( _skip )
			{
				_skip = false;
				continue;
			}

			if( noThumbs )
			{
				if( !thumbs.IsNull() )
					thumbs.ReleaseDC();
				continue;
			}

			thumbs.ReleaseDC();

			CString thumbsName = _thumbsDir + filmInfo.hash + L".jpg";
			SaveJ( thumbs, thumbsName );
		}

		if( hasErrors )
			AfxMessageBox( L"В процессе получения кадров возникли ошибки.", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND );

		return true;
	}
	catch( ... )
	{
		return false;
	}
}

bool CThumbsDlg::ProcessFilm()
{
	try
	{
		CString currentDir = GetCurrentModulePath();

		media::SetMediaInfoPath( (currentDir + L"MediaInfo.dll").GetString() );

		CreateDirectory( _thumbsDir, nullptr );

		const CString mplayer = currentDir + g_ffmpeg;

		if( !::PathFileExists( mplayer.GetString() ) )
		{
			CString errorMsg;
			errorMsg.Format( L"Невозможно получить кадры, отсутствует файл:\n%s", mplayer.GetString() );
			AfxMessageBox( errorMsg, MB_OK | MB_ICONERROR | MB_TOPMOST );
			return false;
		}

		wchar_t path[MAX_PATH];
		GetTempPath( MAX_PATH, path );
		CString tempDir = path;
		tempDir += L"vc_thumb\\";

		const CString screenName = tempDir + g_shotName;

		bool hasErrors = false;


		const FilmInfo & filmInfo = _films.at( 0 );
		{
			++_currentStep;
			CString titleText;
			titleText.Format( L"Обрабатывается %i из %i фильмов", _currentStep, _totalSteps );
			_editCtrl.SetWindowText( titleText );
			_listCtrl.AddString( filmInfo.path );

			wchar_t shortName[MAX_PATH] = {};
			GetShortPathName( filmInfo.path.GetString(), shortName, MAX_PATH );
			if( shortName[0] == 0 )
			{
				_listCtrl.AddString( L"--- Ошибка. Файл не найден." );
				hasErrors = true;

				for( int i = 0; i < 10; ++i )
					_progressCtrl.StepIt();

				return false;
			}

			//получение длины не всегда срабатывает. Сделаем вид, что длина фильма 90 минут
			unsigned videoLength = GetVideoLength( filmInfo.path );
			if( videoLength == 0 )
				videoLength = 5400;

			const unsigned startOffset = videoLength / 18;
			const unsigned shotOffset = videoLength / (NUM_MANUAL_FRAMES+5);

			bool noThumbs = false;

			_frameList.clear();

			for( int i = 0; i < NUM_MANUAL_FRAMES; ++i )
			{
				::DeleteFile( screenName );

				if( !GenerateThumbnail( currentDir, mplayer, filmInfo.path, screenName, startOffset + shotOffset * i ) )
				{
					noThumbs = true;
					break;
				}

				std::shared_ptr<CImage> src = std::make_shared<CImage>();
				src->Load( screenName );
				if( src->IsNull() )
				{
					noThumbs = true;
					break;
				}

				_frameList.push_back( src );

				UpdateFrameList();

				_progressCtrl.StepIt();

				/*
				if( i == 1 )
				{
					srcRect.SetRect( 0, 0, src.GetWidth(), src.GetHeight() );

					for( int unitIndex = 2; unitIndex >= (_units - 1); --unitIndex )
					{
						if( srcRect.Width() <= resolutions[unitIndex].first )
							break;

						frameWidth = resolutions[unitIndex].first;
						frameHeight = resolutions[unitIndex].second;
					}

					if( srcRect.Width() != frameWidth || srcRect.Height() != frameHeight )
					{
						if( ((float)srcRect.Width() / (float)srcRect.Height()) <= (1280.0f / 720.0f) )
						{
							// меньший аспект, чем 1280*720
							float scale = (float)srcRect.Width() / (float)frameWidth;
							int h = frameHeight * scale;
							int offset = abs( h - srcRect.Height() ) / 2;
							srcRect.top += offset;
							srcRect.bottom -= offset;
						}
						else
						{
							// более широкий, чем 1280*720

							float scale = (float)srcRect.Height() / (float)frameHeight;
							int w = frameWidth * scale;
							int offset = abs( w - srcRect.Width() ) / 2;
							srcRect.left += offset;
							srcRect.right -= offset;
						}
					}

					dest.SetRect( 0, 0, frameWidth, frameHeight );
					dest.OffsetRect( 0, 18 );

					thumbs.Create( frameWidth, frameHeight * 10 + 18, 24 );
					dc = thumbs.GetDC();
					::SelectObject( dc, blackPen.GetSafeHandle() );
				}

				::SetStretchBltMode( dc, HALFTONE );

				src.StretchBlt( dc, destTop, srcRect, SRCCOPY );
				src.StretchBlt( dc, dest, srcRect, SRCCOPY );

				::MoveToEx( dc, destTop.right - 1, destTop.top, 0 );
				::LineTo( dc, destTop.right - 1, destTop.bottom - 1 );

				dest.OffsetRect( 0, frameHeight );
				destTop.OffsetRect( 32, 0 );
				*/
			}

			if( noThumbs )
			{
				return false;
			}

			/*
			::MoveToEx( dc, 0, 17, 0 );
			::LineTo( dc, 0, 0 );
			::LineTo( dc, 319, 0 );
			::LineTo( dc, 319, 17 );

			thumbs.ReleaseDC();

			CString thumbsName = _thumbsDir + filmInfo.hash + L".jpg";
			thumbs.Save( thumbsName );
			*/
		}

		if( hasErrors )
			AfxMessageBox( L"В процессе получения кадров возникли ошибки.", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND );

		return true;
	}
	catch( ... )
	{
		return false;
	}
}

void CThumbsDlg::OnCancel()
{
	if( _workingThread != INVALID_HANDLE_VALUE )
		CloseHandle( _workingThread );

	CDialog::OnCancel();
}

void CThumbsDlg::UpdateFrameList()
{
	//for( unsigned i = 0; i < _frameList.size(); ++i )
	auto lastFrame = _frameList.back();
	{
		int index = _frameList.size()-1;
		CImage img;
		img.Create( THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, 24, BI_RGB );

		HDC dc = img.GetDC();
		::SetStretchBltMode( dc, HALFTONE );

		lastFrame->StretchBlt( dc, CRect( 0, 0, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT ), SRCCOPY );
		img.ReleaseDC();

		_imageListThumb.Add( CBitmap::FromHandle(img.Detach() ), COLORREF() );

		_listFrames.InsertItem( index, L"", index );
	}

	_listFrames.Invalidate();
}

void CThumbsDlg::DrawSelected()
{
	if( _selectedItem < 0 )
		return;

	CClientDC dc( this );
	dc.SetStretchBltMode( HALFTONE );

	CRect rect;
	CWnd * wnd = GetDlgItem( IDC_PREVIEW );
	wnd->GetWindowRect( rect );
	ScreenToClient( rect );

	_frameList[_selectedItem]->StretchBlt( dc.GetSafeHdc(), rect, SRCCOPY );
}

void CThumbsDlg::OnItemchangedListFrames( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	_selectedItem = pNMLV->iItem;

	CheckButton();
	DrawSelected();
}

void CThumbsDlg::CheckButton()
{
	const int numItems = _listFrames.GetItemCount();
	int numSelected = 0;
	_selectedImages.clear();

	for( int i = 0; i < numItems; ++i )
	{
		if( _listFrames.GetCheck( i ) )
		{
			numSelected += 1;
			_selectedImages.push_back( i );
		}
	}

	CWnd * wnd = GetDlgItem( IDC_BTN_CREATE_THUMBS );
	wnd->EnableWindow( numSelected == 10 );

	CString btnTitle;
	btnTitle.Format( L"Назначить выбранные кадры (%i / 10)", (int)_selectedImages.size() );
	wnd->SetWindowText( btnTitle );
}


void CThumbsDlg::OnBtnCreateThumbs()
{
	UpdateData();

	int frameWidth = 0;
	int frameHeight = 0;
	CRect dest;
	CRect srcRect;

	// вычисление размера кадров
	{
		static const std::pair<int, int> resolutions[4] =
		{
			{ 10000, 0 },
			{ 1280, 720 },
			{ 854, 480 },
			{ 480, 270 }
		};

		frameWidth = resolutions[2].first;
		frameHeight = resolutions[2].second;

		CImage * src = _frameList.at( 0 ).get();

		srcRect.SetRect( 0, 0, src->GetWidth(), src->GetHeight() );

		if( _units == 1 )
		{
			frameWidth = srcRect.Width();
			frameHeight = (frameWidth * 720) / 1280;
		}
		else
		{
			for( int unitIndex = 3; unitIndex >= (_units - 1); --unitIndex )
			{
				if( srcRect.Width() <= resolutions[unitIndex].first )
					break;

				frameWidth = resolutions[unitIndex].first;
				frameHeight = resolutions[unitIndex].second;
			}
		}

		if( srcRect.Width() != frameWidth || srcRect.Height() != frameHeight )
		{
			if( ((float)srcRect.Width() / (float)srcRect.Height()) <= (1280.0f / 720.0f) )
			{
				// меньший аспект, чем 1280*720
				float scale = (float)srcRect.Width() / (float)frameWidth;
				int h = (int)(frameHeight * scale);
				int offset = abs( h - srcRect.Height() ) / 2;
				srcRect.top += offset;
				srcRect.bottom -= offset;
			}
			else
			{
				// более широкий, чем 1280*720

				float scale = (float)srcRect.Height() / (float)frameHeight;
				int w = (int)(frameWidth * scale);
				int offset = abs( w - srcRect.Width() ) / 2;
				srcRect.left += offset;
				srcRect.right -= offset;
			}
		}

		dest.SetRect( 0, 0, frameWidth, frameHeight );
	}

	CPen blackPen( PS_SOLID, 1, RGB( 0, 0, 0 ) );

	CImage thumbs;
	thumbs.Create( frameWidth, frameHeight * 10, 24 );
	HDC dc = thumbs.GetDC();

	::SelectObject( dc, blackPen.GetSafeHandle() );
	::SetStretchBltMode( dc, HALFTONE );

	for( unsigned i = 0; i < 10; ++i )
	{
		CImage * src = _frameList.at( _selectedImages.at( i ) ).get();

		src->StretchBlt( dc, dest, srcRect, SRCCOPY );

		dest.OffsetRect( 0, frameHeight );
	}

	thumbs.ReleaseDC();

	const FilmInfo & filmInfo = _films.at( 0 );
	CString thumbsName = _thumbsDir + filmInfo.hash + L".jpg";
	SaveJ( thumbs, thumbsName );

	//thumbs.Save( thumbsName );

	OnCancel();
}


void CThumbsDlg::OnBtnSkip()
{
	_skip = true;
}
