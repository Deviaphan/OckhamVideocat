// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "About.h"
#include <vector>
#include <thread>
#include "GlobalSettings.h"
#include "License/KeySerialize.h"
#include "License/Blowfish.h"
#include "ResString.h"
#include "IconManager.h"
#include "resource.h"

#define UPDATE_DATA_MESSAGE (WM_APP + 1)

CAboutDlg::CAboutDlg()
	: CDialog( IDD_ABOUTBOX )
	, _versionText( ResString(IDS_NO_NEW_VERSION) )
	, _programVersion()
	, _hasLicense( false )
{
}

void CAboutDlg::DoDataExchange( CDataExchange * pDX )
{
	CDialog::DoDataExchange( pDX );

	DDX_Text( pDX, IDC_VERSION_BLOCK, _versionText );
	DDX_Control( pDX, IDC_LICENSE_INFO, _licenseInfo );
	DDX_Control( pDX, IDC_VK_SUBSCRIBE, _btnSubscribe );
	DDX_Control( pDX, IDC_BUY, _btnShopping );
}

BEGIN_MESSAGE_MAP( CAboutDlg, CDialog )
	ON_BN_CLICKED( IDC_BUY, &CAboutDlg::OnBuy )
	ON_BN_CLICKED( IDC_INSTALL_UPDATE, &CAboutDlg::OnInstallUpdate )
	ON_BN_CLICKED( IDC_VK_SUBSCRIBE, &CAboutDlg::OnVkSubscribe )
	ON_MESSAGE( UPDATE_DATA_MESSAGE, &CAboutDlg::TestMessageCall )
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CheckUpdateData( CAboutDlg * dlg )
{
	Version & versionInfo = GetGlobal().versionInfo;
	if( versionInfo.HasNewVersion() )
	{		
		dlg->_versionText = ResFormat( IDS_NEW_VERSION_FOUNDED, versionInfo.version.GetString() );
		dlg->_versionText += versionInfo.whatNew;
		dlg->_programVersion = versionInfo.GetVersionName();

		dlg->_downloadUrl.Format( L"http://videocat.maagames.ru/files/videocat_%i.7z", (int)versionInfo.build );

		dlg->AllowUpdateButton();

		dlg->SendMessage( UPDATE_DATA_MESSAGE, 0, 0 );
	}
}

void CAboutDlg::AllowUpdateButton()
{
	CWnd * wnd = GetDlgItem( IDC_INSTALL_UPDATE );
	if( wnd )
	{
		wnd->EnableWindow( TRUE );
	}
}

LRESULT CAboutDlg::TestMessageCall( WPARAM, LPARAM )
{
	GetDlgItem( IDC_INSTALL_UPDATE )->EnableWindow( TRUE );
	UpdateData( FALSE );
	return LRESULT();
}

BOOL CAboutDlg::OnInitDialog()
{
	_btnSubscribe._iconID = IconManager::VKLink;
	_btnShopping._iconID = IconManager::Shopping;

	CDialog::OnInitDialog();

	CString title;
	GetWindowText( title );
	Version & versionInfo = GetGlobal().versionInfo;
	title += L"  |  ";
	title += versionInfo.GetVersionName();
	SetWindowText( title );

	std::thread updThread( CheckUpdateData, this );
	updThread.detach();

	CWnd * logo = GetDlgItem( IDC_LOGO );
	if( logo )
	{
		CRect rect;
		logo->GetWindowRect( rect );
		ScreenToClient( rect );

		if( ::PathFileExists( GetGlobal().GetKeyFileName() ) )
		{
			_hasLicense = _licenseBox.Initialize( GetGlobal().GetKeyFileName(), rect );

			if( _hasLicense )
			{
				try
				{
					KeySerialize key;
					ReadBuffer( _licenseBox._keyImg, key.buffer );

					const std::string pass( "Please don't hack me :`(" );
					CBlowFish crypt( (const unsigned char * )pass.c_str(), pass.length(), SBlock( 1, 3 ) );
					crypt.Decrypt( key.buffer, 256, CBlowFish::CBC );

					if( !key.Deserialize() )
						throw std::exception();

					_licenseInfo.AddString( ResFormat( IDS_LICENSE_USERNAME, key.name.GetString() ) );
					_licenseInfo.AddString( L"" );

					_licenseInfo.AddString( ResFormat( IDS_LICENSE_EMAIL, key.email.GetString() ) );
					_licenseInfo.AddString( L"" );

					switch( (LicenseType)key.licenseType )
					{
						case LicenseType::Personal_1PC:
							_licenseInfo.AddString( ResString( IDS_LICENSE_TYPE_1 ) ); break;
						case LicenseType::Home_3PC:
							_licenseInfo.AddString( ResString( IDS_LICENSE_TYPE_2 ) ); break;
						case LicenseType::Network_AnyPC:
							_licenseInfo.AddString( ResString( IDS_LICENSE_TYPE_3 ) ); break;
						case LicenseType::Commercial:
							_licenseInfo.AddString( ResString( IDS_LICENSE_TYPE_4 ) ); break;
					}

					SetTimer( 1234, 40, nullptr );
				}
				catch( ... )
				{
					_hasLicense = false;
				}
			}
		}

		if( !_hasLicense )
		{
			_catHead.Initialize( rect );

			_licenseInfo.AddString( ResString( IDS_UNREGISTERED_VERSION ) );

			SetTimer( 2345, 25, nullptr );
		}
	}

	return TRUE;
}


void CAboutDlg::OnPaint()
{
	CPaintDC dc( this );

	if( !_hasLicense )
		_catHead.Draw( dc.GetSafeHdc() );
	else
		_licenseBox.Draw( dc.GetSafeHdc() );
}


void CAboutDlg::OnMouseMove( UINT nFlags, CPoint point )
{
	CDialog::OnMouseMove( nFlags, point );
}


void CAboutDlg::OnTimer( UINT_PTR nIDEvent )
{
	if( nIDEvent == 1234 )
	{
		_licenseBox.NextFrame();
		InvalidateRect( _licenseBox.GetRect() );
	}
	else if( nIDEvent == 2345 )
	{
		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );

		_catHead.MouseMove( point.x, point.y );
		InvalidateRect( _catHead.GetRect(), FALSE );
	}

	CDialog::OnTimer( nIDEvent );
}


void CAboutDlg::OnVkSubscribe()
{
	::ShellExecute( nullptr, L"open", L"https://vk.com/maagames", nullptr, nullptr, SW_SHOWNORMAL );
}

void CAboutDlg::OnBuy()
{
	::ShellExecute( nullptr, L"open", L"http://videocat.maagames.ru/purchase.html", nullptr, nullptr, SW_SHOWNORMAL );
}


void CAboutDlg::OnInstallUpdate()
{
	//_downloadUrl = L"http://videocat.maagames.ru/files/videocat_test.7z";

	CString updaterPath = GetGlobal().GetProgramDirectory();
	updaterPath += L"videocat\\updater.exe";

	CString params;
	params.Format( L"\"%s\" \"%s\\\"", _downloadUrl.GetString(), GetGlobal().GetProgramDirectory().GetString() );
	
	const bool isReadOnly = ::IsReadOnlyFolder( GetGlobal().GetProgramDirectory() );
	
	SHELLEXECUTEINFO shExInfo = { 0 };
	shExInfo.cbSize = sizeof( shExInfo );
	shExInfo.fMask = SEE_MASK_DEFAULT | SEE_MASK_UNICODE;
	shExInfo.hwnd = nullptr;
	shExInfo.lpVerb = isReadOnly ? L"runas" : L"open";	// Operation to perform
	shExInfo.lpFile = updaterPath.GetString();	// Application to start
	shExInfo.lpParameters = params.GetString();	// Additional parameters
	shExInfo.lpDirectory = nullptr;
	shExInfo.nShow = SW_SHOW;
	shExInfo.hInstApp = nullptr;

	ShellExecuteEx( &shExInfo );


	CWnd * parentWnd = GetParent();

	OnClose();

	parentWnd->PostMessageW( WM_CLOSE, 0, 0 );
}
