// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "ViewStyleDlg.h"
#include <algorithm>
#include "VideoCat.h"
#include "GlobalSettings.h"
#include "ThemeEditor.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "VideoCatDoc.h"
#include "MainFrm.h"

#include "SynchronizeFiles.h" // FileGuard

#include "FileStore/reader.h"
#include "CStringStore.h"
#include <fstream>
#include <filesystem>
#include "SetWin10Theme.h"


#include "resource.h"

const int sliderTable[7] = { 100, 125, 150, 175, 200, 250, 300 };
int GetIndex( int scale )
{
	for( int i = 0; i < 7; ++i )
	{
		if( scale <= sliderTable[i] )
			return i;
	}

	return 0;
}

int GetValue( int index )
{
	return sliderTable[index];
}

void SetWin10ThemeByColor( CVideoCatDoc * doc, COLORREF color )
{
	BYTE r = GetRValue( color );
	BYTE g = GetGValue( color );
	BYTE b = GetBValue( color );

	BYTE gray = (r >> 2) + (g >> 1) + (b >> 2);

	if( gray < 128 )
	{
		doc->GetVideoTreeView()->SetDarkTheme( true );
		doc->GetVideoCatView()->SetDarkTheme( true );
	}
	else
	{
		doc->GetVideoTreeView()->SetDarkTheme( false );
		doc->GetVideoCatView()->SetDarkTheme( false );
	}
}

IMPLEMENT_DYNAMIC(CViewStyleDlg, CPropertyPage)

CViewStyleDlg::CViewStyleDlg()
	: CPropertyPage(IDD_VIEW_STYLE)
	, viewType( ViewTileIndex )
	, _doc(nullptr)
	, _currentTheme( nullptr )
	, viewScale( 100 )
{
	_currentTheme = GetGlobal().theme;
	viewScale = GetGlobal().viewScale;
}

CViewStyleDlg::~CViewStyleDlg()
{
	GetGlobal().theme = _currentTheme;

	UpdateView();
}

void CViewStyleDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange( pDX );
	DDX_Radio( pDX, IDC_VIEW_TILES, viewType );
	DDX_Control( pDX, IDC_THEME_LIST, _themeList );
	DDX_Text( pDX, IDC_EDIT_SCALE, viewScale );
	DDX_Control( pDX, IDC_SLIDER_SCALE, _scaleSlider );
}


BEGIN_MESSAGE_MAP(CViewStyleDlg, CPropertyPage)
	ON_BN_CLICKED( IDC_EDIT_THEME, &CViewStyleDlg::OnEditTheme )
	ON_BN_CLICKED( IDC_APPLY_THEME, &CViewStyleDlg::OnApplyTheme )
	ON_BN_CLICKED( IDC_CREATE_THEME, &CViewStyleDlg::OnCreateTheme )
	ON_BN_CLICKED( IDC_DELETE_THEME, &CViewStyleDlg::OnDeleteTheme )
	ON_LBN_SELCHANGE( IDC_THEME_LIST, &CViewStyleDlg::OnSelchangeTheme )
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


BOOL CViewStyleDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();


	BuildThemeList();

	_tooltip.Create( this, TTS_ALWAYSTIP );

	CMFCToolTipInfo params;
	params.m_bBalloonTooltip = FALSE;
	params.m_bVislManagerTheme = TRUE;
	params.m_bDrawSeparator = TRUE;
	_tooltip.SetParams( &params );

	_scaleSlider.SetRange( 0, 6 );
	_scaleSlider.SetPos( GetIndex( viewScale ) );

	return TRUE;
}

BOOL CViewStyleDlg::PreTranslateMessage( MSG* pMsg )
{
	_tooltip.RelayEvent( pMsg );

	return CPropertyPage::PreTranslateMessage( pMsg );
}

void CViewStyleDlg::OnApplyTheme()
{
	VC_TRY;
	CWaitCursor waiting;

	const int index = _themeList.GetCurSel();
	if( index < 0 )
		return;

	CString themePath = GetGlobal().GetThemeDirectory();
	themePath += _themeNames[index];
	themePath += L"\\videocat.theme";

	if( !::PathFileExists( themePath ) )
		return;

	std::ifstream fin( themePath, std::ios_base::binary );
	FileStore::Reader reader( fin );

	reader & *_currentTheme;
	_currentTheme->bgImageFile = _themeNames[index];

	_currentTheme->Initialize( true );
	_currentTheme->UpdateFonts();

	theApp.WriteProfileString( L"", L"usedTheme", _themeNames[index] );

	VC_CATCH_ALL;
}

void CViewStyleDlg::OnCreateTheme()
{
	CWaitCursor waiting;

	DefaultTheme( _theme );
	GetGlobal().theme = &_theme;

	ThemeEditor * editor = new ThemeEditor( GetParent()->GetParent(), this );

	editor->Create( IDD_THEME_EDIT );
	editor->ShowWindow( SW_SHOWNORMAL );

	editor->OnThemePreview();
} //-V773

void CViewStyleDlg::OnEditTheme()
{
	const int index = _themeList.GetCurSel();
	if( index < 0 )
		return;

	CWaitCursor waiting;
	
	GetGlobal().theme = &_theme;

	ThemeEditor * editor = new ThemeEditor( GetParent()->GetParent(), this );

	editor->themeName = _themeNames[index];

	editor->Create( IDD_THEME_EDIT );
	editor->ShowWindow( SW_SHOWNORMAL );

	editor->OnThemePreview();
}

void CViewStyleDlg::OnDeleteTheme()
{
	const int index = _themeList.GetCurSel();
	if( index < 0 )
		return;

	if( AfxMessageBox( L"Вы уверены, что хотите удалить тему оформления?", MB_YESNO | MB_TOPMOST ) != IDYES )
		return;

	CWaitCursor waiting;

	CString themePath = GetGlobal().GetThemeDirectory();
	themePath += _themeNames[index];
	themePath += L"\\";

	std::error_code errCode;

	if( !std::filesystem::exists( themePath.GetString(), errCode ) )
		return;

	std::filesystem::remove_all( themePath.GetString(), errCode );

	BuildThemeList();
}

void CViewStyleDlg::UpdateView()
{
	VC_TRY;

	SetWin10ThemeByColor( _doc, GetGlobal().theme->bgBottomColor );

	_doc->GetVideoTreeView()->RebuildTree();

	CMainFrame * mainFrm = dynamic_cast<CMainFrame *>(AfxGetMainWnd());
	if( mainFrm )
	{
		mainFrm->m_wndSplitter.Invalidate();
	}
	
	VC_CATCH_ALL;
}

void CViewStyleDlg::BuildThemeList()
{
	_themeNames.clear();
	_themeList.ResetContent();

	CString themeDir = GetGlobal().GetThemeDirectory() + L"*";

	const FINDEX_INFO_LEVELS infoLevel = FindExInfoStandard;
	const FINDEX_SEARCH_OPS searchOp = FindExSearchNameMatch;
	const DWORD useLargeFetch = 0;

	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFileEx( themeDir, infoLevel, &fd, searchOp, nullptr, useLargeFetch );

	if( hFind != INVALID_HANDLE_VALUE )
	{
		FindGuard guard( hFind );
		do
		{
			// пропускаем скрытые и системные
			if( fd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM) )
				continue;

			if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				CString name = fd.cFileName;
				if( name == L"." || name == L".." )
					continue;

				_themeNames.emplace_back( fd.cFileName );
			}
		}
		while( ::FindNextFile( hFind, &fd ) );

		guard.Close();
	}

	std::sort( _themeNames.begin(), _themeNames.end() );

	for( const CString & name : _themeNames )
	{
		_themeList.AddString( name );
	}
}


void CViewStyleDlg::OnSelchangeTheme()
{
	VC_TRY;

	CWaitCursor waiting;

	const int index = _themeList.GetCurSel();
	if( index < 0 )
	{
		GetGlobal().theme = _currentTheme;
	}
	else
	{
		CString themePath = GetGlobal().GetThemeDirectory();
		themePath += _themeNames[index];
		themePath += L"\\videocat.theme";

		if( !::PathFileExists( themePath ) )
			return;

		std::ifstream fin( themePath, std::ios_base::binary );
		FileStore::Reader reader( fin );

		reader & _theme;
		_theme.bgImageFile = _themeNames[index];

		GetGlobal().theme = &_theme;

		_theme.Initialize( true );
		_theme.UpdateFonts();
	}

	UpdateView();

	VC_CATCH_ALL;
}


void CViewStyleDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar * pScrollBar )
{
	CSliderCtrl * pSlider = reinterpret_cast<CSliderCtrl *>(pScrollBar);

	if( pSlider == &_scaleSlider )
	{
		viewScale = GetValue( pSlider->GetPos() );
		UpdateData( FALSE );

		GetGlobal().viewScale = viewScale;

		_doc->GetVideoCatView()->RecalcScrollLayout();
		_doc->GetVideoCatView()->UpdateView();

	}
	else
	{
		CPropertyPage::OnHScroll( nSBCode, nPos, pScrollBar );
	}
}
