// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ShortcutDlg.h"
#include "GlobalSettings.h"
#include "ResString.h"
#include "resource.h"



IMPLEMENT_DYNAMIC(CShortcutDlg, CPropertyPage)

CShortcutDlg::CShortcutDlg()
	: _shortcutName(L"= VideoCat =")
{

}

CShortcutDlg::~CShortcutDlg()
{
}

void CShortcutDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_SHORTCUT_NAME, _shortcutName );
}


BEGIN_MESSAGE_MAP(CShortcutDlg, CPropertyPage)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(ID_CREATE_SHORTCUT, &CShortcutDlg::OnCreateShortcut)
END_MESSAGE_MAP()


BOOL CShortcutDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CWnd * logo = GetDlgItem( IDC_LOGO );
	if( logo )
	{
		CRect rect;
		logo->GetWindowRect( rect );
		ScreenToClient( rect );

		_catHead.Initialize( rect );
	}

	return TRUE;
}


void CreateLink( const CString & targetFullPath, const CString & workingDir, const CString & shortcutFullPath, const CString & description, const CString & arguments, int showCmd )
{
	HRESULT hres;
	IShellLink *NewLink;
	CoInitialize( nullptr );
	hres = CoCreateInstance( CLSID_ShellLink, nullptr,	CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&NewLink );
	if( SUCCEEDED( hres ) )
	{
		NewLink->SetPath( targetFullPath );
		NewLink->SetWorkingDirectory( workingDir );
		NewLink->SetDescription( description );
		NewLink->SetArguments( arguments );
		NewLink->SetShowCmd( showCmd );
		IPersistFile *ppf;
		NewLink->QueryInterface( IID_IPersistFile, (void**)&ppf );
		ppf->Save( shortcutFullPath, true );

		NewLink->Release();
	}
	CoUninitialize();
}

void CShortcutDlg::OnCreateShortcut()
{
	VC_TRY;

	UpdateData();

	wchar_t shortcutDir[MAX_PATH] = L"";
	LPITEMIDLIST pidl;
	SHGetSpecialFolderLocation( nullptr, CSIDL_DESKTOP, &pidl );
	SHGetPathFromIDList( pidl, shortcutDir );

	CString shortcutPath = ResFormat( L"%s\\%s.lnk", shortcutDir, _shortcutName.GetString() );

	CreateLink( GetGlobal().programFullPath, GetGlobal().GetProgramDirectory(), shortcutPath, L"", L"", SW_SHOWNORMAL );

	VC_CATCH( ... )
	{
		AfxMessageBox( ResString(IDS_ERROR_SHORTCUT_ERROR), MB_OK | MB_ICONHAND | MB_TOPMOST );
	}
}

void CShortcutDlg::OnPaint()
{
	CPaintDC dc( this );

	_catHead.Draw( dc.GetSafeHdc() );
}


void CShortcutDlg::OnMouseMove( UINT nFlags, CPoint point )
{
	_catHead.MouseMove( point.x, point.y );

	CDialog::OnMouseMove( nFlags, point );

	InvalidateRect( _catHead.GetRect(), FALSE );
}


void CShortcutDlg::OnOK()
{
	; // stub
}


void CShortcutDlg::OnCancel()
{
	// stub
}
