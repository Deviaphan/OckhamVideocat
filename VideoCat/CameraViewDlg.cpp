// // This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// // PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "CameraViewDlg.h"
#include "resource.h"


// CameraViewDlg dialog
enum
{
	TIMER_ID = 6431
	, ELAPSE = 10
};

IMPLEMENT_DYNAMIC( CameraViewDlg, CDialog )

int CameraViewDlg::cameraFilterType = 2;

CameraViewDlg::CameraViewDlg( CWnd* pParent /*=nullptr*/ )
	: CDialog( IDD_CAMERA_DIALOG, pParent )
	, _timerId( 0 )
{
}

CameraViewDlg::~CameraViewDlg()
{
}

void CameraViewDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio( pDX, IDC_WEBCAM_FILTER_1, cameraFilterType );
	DDX_Control( pDX, IDOK, _btnOK );
}


BEGIN_MESSAGE_MAP( CameraViewDlg, CDialog )
	ON_WM_TIMER() ON_WM_PAINT()END_MESSAGE_MAP()


// CameraViewDlg message handlers


BOOL CameraViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect btnSize;
	_btnOK.GetWindowRect( btnSize );

	CRect wndSize( 0, 0, 1280, 720 + btnSize.Height() + 16 );
	constexpr DWORD styleFlag = WS_CAPTION | WS_OVERLAPPEDWINDOW;
	AdjustWindowRect( wndSize, styleFlag, FALSE );

	this->SetWindowPos( nullptr, 0, 0, wndSize.Width(), wndSize.Height(), SWP_NOMOVE | SWP_SHOWWINDOW );

	StartCapture();

	return TRUE;
}


void CameraViewDlg::OnTimer( UINT_PTR nIDEvent )
{
	if( nIDEvent == TIMER_ID )
	{
		KillTimer( TIMER_ID );

		UpdateData();

		Invalidate( FALSE );
		UpdateWindow();
	}
	else
	{
		CDialog::OnTimer( nIDEvent );
	}
}


void CameraViewDlg::OnPaint()
{
	CPaintDC dc( this );

	if( _image.IsNull() )
	{
		return;
	}

	if( ! _vi.isDeviceSetup( 0 ) )
		return;

	if( _vi.isFrameNew( 0 ) )
	{
		static DWORD lastTime = GetTickCount();

		unsigned char* buffer = _vi.getPixels( 0, false, true );

		const int h = _image.GetHeight();
		const int w = _image.GetWidth();

		for( int y = 0; y < h; ++y )
		{
			for( int x = 0; x < w; ++x )
			{
				_imageData.AddPixel( _curSample, x, y, buffer[0], buffer[1], buffer[2] );

				buffer += 3;
			}
		}

		++_totalFrames;
		_curSample = _totalFrames % _numSamples;

		if( _totalFrames < _numSamples )
		{
			_timerId = SetTimer( TIMER_ID, ELAPSE, nullptr );

			return;
		}

		switch( cameraFilterType )
		{
			case 0:
				_imageData.ProcessSigmaClipping();
				break;
			case 1:
				_imageData.ProcessMean();
				break;
			default:
				_imageData.ProcessNone();
				break;
		}


		int pitch = _image.GetPitch();
		unsigned char* bits = (unsigned char*)_image.GetBits();

		for( int y = 0; y < h; ++y )
		{
			_imageData.GetLine( y, bits );

			bits += pitch;
		}

		//_image.BitBlt( dc.GetSafeHdc(), 0, 0, SRCCOPY );

		CRect dcRect;
		//CRect cliRect;
		//GetClientRect( cliRect );

		double aspect = (double)_image.GetHeight() / (double)_image.GetWidth();
		dcRect.SetRect( 0, 0, int( 720 / aspect ), 720 );

		//dc.SetStretchBltMode( HALFTONE );
		_image.StretchBlt( dc.GetSafeHdc(), dcRect, SRCCOPY );

		{
			// 160 * 200
			// 576 * 720
			CPen pen( PS_SOLID, 1, RGB( 0, 255, 192 ) );
			auto oldPen = dc.SelectObject( pen );

			dc.MoveTo( 0, 00 );
			dc.LineTo( 352, 60 );
			dc.LineTo( 352, 660 );
			dc.LineTo( 0, 720 );

			dc.MoveTo( 1280, 00 );
			dc.LineTo( 928, 60 );
			dc.LineTo( 928, 660 );
			dc.LineTo( 1280, 720 );

			dc.SelectObject( oldPen );
		}

		DWORD curTime = GetTickCount();

		double ticks = (curTime - lastTime);
		CString time;
		time.Format( L"[%i x %i px] fps = %.1f", w, h, 1000.0 / ticks );
		dc.SetBkMode( TRANSPARENT );
		dc.SetTextColor( RGB( 0, 0, 0 ) );
		dc.TextOut( 10, 10, time );
		dc.SetTextColor( RGB( 200, 200, 200 ) );
		dc.TextOut( 9, 9, time );


		lastTime = curTime;
	}

	_timerId = SetTimer( TIMER_ID, ELAPSE, nullptr );
}

void CameraViewDlg::StartCapture()
{
	CSize sizeTotal;

	_vi.setUseCallback( true );
	_vi.setRequestedMediaSubType( VI_MEDIASUBTYPE_YV12 );

	_vi.setupDevice( 0, 1280, 720 );
	_vi.setIdealFramerate( 0, 50 );

	int w = _vi.getWidth( 0 );
	int h = _vi.getHeight( 0 );

	_image.Create( w, h, 24, BI_RGB );

	_imageData.Create( w, h );

	_timerId = SetTimer( TIMER_ID, ELAPSE, nullptr );
}

void CameraViewDlg::StopCapture()
{
	KillTimer( TIMER_ID );

	_vi.stopDevice( 0 );
}


void CameraViewDlg::OnOK()
{
	StopCapture();

	if( !_image.IsNull() )
	{
		HBITMAP hBitmap = _image;

		DIBSECTION ds;
		GetObject( hBitmap, sizeof(ds), &ds );
		ds.dsBmih.biCompression = BI_RGB;
		HDC hDC = ::GetDC( NULL );
		HBITMAP hDDB = CreateDIBitmap( hDC, &ds.dsBmih, CBM_INIT, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih
										, DIB_RGB_COLORS );
		::ReleaseDC( NULL, hDC );

		OpenClipboard();
		EmptyClipboard();
		SetClipboardData( CF_BITMAP, hDDB );
		CloseClipboard();
		DeleteObject( hBitmap );
	}

	CDialog::OnOK();
}


void CameraViewDlg::OnCancel()
{
	StopCapture();

	CDialog::OnCancel();
}
