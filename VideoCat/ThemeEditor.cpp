// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeEditor.h"
#include "GlobalSettings.h"
#include "ViewStyleDlg.h"
#include "ThemeName.h"
#include "FileStore/writer.h"
#include "CStringStore.h"
#include <fstream>
#include <filesystem>
#include "resource.h"

int ThemeEditor::refCount = 0;

IMPLEMENT_DYNAMIC(ThemeEditor, CDialog)

ThemeEditor::ThemeEditor( CWnd * topDlg, CViewStyleDlg* pParent )
	: CDialog(IDD_THEME_EDIT, pParent)
	, _activePage( nullptr )
	, _topDlg( topDlg )
	, _parent(pParent)
{

	_topDlg->ShowWindow( SW_HIDE );

	++refCount;
}

ThemeEditor::~ThemeEditor()
{
	GetGlobal().theme = _parent->_currentTheme;

	--refCount;
}

void ThemeEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDIT_BG, _btnBG );
	DDX_Control( pDX, IDC_EDIT_SHADOW, _btnShadow );
	DDX_Control( pDX, IDC_EDIT_TILE, _btnTile );
	DDX_Control( pDX, IDC_EDIT_LIST, _btnList );
	DDX_Control( pDX, IDC_EDIT_TREE, _btnTree );
	DDX_Control( pDX, IDC_EDIT_MENU, _btnMenu );
	DDX_Control( pDX, IDC_SAVE, _btnSave );
	DDX_Control( pDX, IDC_SAVE_AS, _btnSaveAs );
}


BEGIN_MESSAGE_MAP(ThemeEditor, CDialog)
	ON_BN_CLICKED( IDC_EDIT_BG, &ThemeEditor::OnBnClickedEditBg )
	ON_BN_CLICKED( IDC_EDIT_LIST, &ThemeEditor::OnBnClickedEditList )
	ON_BN_CLICKED( IDC_EDIT_MENU, &ThemeEditor::OnBnClickedEditMenu )
	ON_BN_CLICKED( IDC_EDIT_SHADOW, &ThemeEditor::OnBnClickedEditShadow )
	ON_BN_CLICKED( IDC_EDIT_TILE, &ThemeEditor::OnBnClickedEditTile )
	ON_BN_CLICKED( IDC_EDIT_TREE, &ThemeEditor::OnBnClickedEditTree )
	ON_BN_CLICKED( IDC_THEME_PREVIEW, &ThemeEditor::OnThemePreview )
	ON_BN_CLICKED( IDC_SAVE_AS, &ThemeEditor::OnSaveAs )
	ON_BN_CLICKED( IDC_SAVE, &ThemeEditor::OnSave )
END_MESSAGE_MAP()



BOOL ThemeEditor::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// запрещаю редактировать (стандартные) темы оформления. Но в дебаге разрешаю, для удобства редактирования мной
#ifndef _DEBUG
	if( !themeName.IsEmpty() && themeName[0] == L'.' )
	{
		_btnSave.EnableWindow( FALSE );
	}
#endif

	CRect rc;

	GetDlgItem( IDC_TAB_RECT )->GetWindowRect( rc );
	ScreenToClient( rc );

	_bgPage.Create( IDD_THEME_BG, this );
	_bgPage.MoveWindow( &rc );
	_bgPage.WriteData( _parent->_theme );

	_shadowPage.Create( IDD_THEME_SHADOW, this );
	_shadowPage.MoveWindow( &rc );
	_shadowPage.WriteData( _parent->_theme );

	_tilePage.Create( IDD_THEME_TILE, this );
	_tilePage.MoveWindow( &rc );
	_tilePage.WriteData( _parent->_theme );

	_listPage.Create( IDD_THEME_LIST, this );
	_listPage.MoveWindow( &rc );
	_listPage.WriteData( _parent->_theme );

	_treePage.Create( IDD_THEME_TREE, this );
	_treePage.MoveWindow( &rc );
	_treePage.WriteData( _parent->_theme );

	_menuPage.Create( IDD_THEME_MENU, this );
	_menuPage.MoveWindow( &rc );
	_menuPage.WriteData( _parent->_theme );

	OnBnClickedEditBg();

	return TRUE;
}

void ThemeEditor::HideAllPages( CWnd * newActivePage )
{
	_btnBG.SetImage( 0u );
	_btnShadow.SetImage( 0u );
	_btnTile.SetImage( 0u );
	_btnList.SetImage( 0u );
	_btnTree.SetImage( 0u );
	_btnMenu.SetImage( 0u );

	Invalidate();

	if( _activePage )
		_activePage->ShowWindow( SW_HIDE );

	_activePage = newActivePage;
	if( _activePage )
		_activePage->ShowWindow( SW_SHOW );
}

void ThemeEditor::OnBnClickedEditBg()
{
	HideAllPages( &_bgPage );	
	_btnBG.SetImage( IDB_VIEW_SETTINGS_16 );
}

void ThemeEditor::OnBnClickedEditShadow()
{
	HideAllPages(&_shadowPage);
	_btnShadow.SetImage( IDB_VIEW_SETTINGS_16 );
}

void ThemeEditor::OnBnClickedEditTile()
{
	HideAllPages(&_tilePage);
	_btnTile.SetImage( IDB_VIEW_SETTINGS_16 );
}

void ThemeEditor::OnBnClickedEditList()
{
	HideAllPages(&_listPage);
	_btnList.SetImage( IDB_VIEW_SETTINGS_16 );
}

void ThemeEditor::OnBnClickedEditTree()
{
	HideAllPages(&_treePage);
	_btnTree.SetImage( IDB_VIEW_SETTINGS_16 );
}

void ThemeEditor::OnBnClickedEditMenu()
{
	HideAllPages(&_menuPage);
	_btnMenu.SetImage( IDB_VIEW_SETTINGS_16 );
}


void ThemeEditor::OnThemePreview()
{
	CWaitCursor waiting;

	_bgPage.ReadData( _parent->_theme );
	_shadowPage.ReadData( _parent->_theme );
	_tilePage.ReadData( _parent->_theme );
	_listPage.ReadData( _parent->_theme );
	_treePage.ReadData( _parent->_theme );
	_menuPage.ReadData( _parent->_theme );

	_parent->_theme.Initialize( true );
	_parent->_theme.UpdateFonts();

	_parent->UpdateView();
}


BOOL ThemeEditor::DestroyWindow()
{
	BOOL result = CDialog::DestroyWindow();

	delete this;

	return result;
}


void ThemeEditor::OnOK()
{
	//CDialog::OnOK();

	OnCancel();
}


void ThemeEditor::OnCancel()
{
	_topDlg->ShowWindow( SW_SHOW );
	_parent->BuildThemeList();

	CDialog::OnCancel();

	DestroyWindow();
}


void SaveTheme( ViewStyle & theme, const CString & themeFolder )
{
	VC_TRY;

	const CString copyImageFrom = theme.bgImageFile;

	const CString themeFile = themeFolder + L"videocat.theme";
	const CString bgImageFile = themeFolder + L"bg.jpg";

	if( copyImageFrom.FindOneOf(L":") >= 0 && copyImageFrom != bgImageFile )
	{
		::CopyFile( copyImageFrom, bgImageFile, FALSE );
	}

	std::ofstream fout( themeFile, std::ios_base::binary );
	FileStore::Writer writer( fout );

	writer & theme;

	VC_CATCH_ALL;
}


void ThemeEditor::OnSaveAs()
{
	ThemeName nameDlg;
	if( nameDlg.DoModal() != IDOK )
		return;

	const CString folder = GetGlobal().GetThemeDirectory();
	
	// создаём папку, если её нет
	if( !PathFileExists( folder ) )
	{
		if( !::CreateDirectory( folder, nullptr ) )
		{
			AfxMessageBox( L"Не удалось создать папку :(", MB_OK | MB_TOPMOST );
			return;
		}
	}

	const CString themeFolder = folder + nameDlg.themeName + L"\\";
	if( PathFileExists( themeFolder ) )
	{
		AfxMessageBox( L"Тема с таким названием уже существует", MB_OK | MB_TOPMOST );
		return;
	}

	if( !::CreateDirectory( themeFolder, nullptr ) )
	{
		AfxMessageBox( L"Не удалось создать папку :(", MB_OK | MB_TOPMOST );
		return;
	}

	themeName = nameDlg.themeName;

	SaveTheme( _parent->_theme, themeFolder );
}


void ThemeEditor::OnSave()
{
	OnThemePreview();

	if( themeName.IsEmpty() )
	{
		OnSaveAs();
		return;
	}

	const CString folder = GetGlobal().GetThemeDirectory();
	const CString themeFolder = folder + themeName + L"\\";

	SaveTheme( _parent->_theme, themeFolder );
}
