#pragma once

#include "ViewStyle.h"
#include "ThemeBgPage.h"
#include "ThemeShadowPage.h"
#include "ThemeTilePage.h"
#include "ThemeListPage.h"
#include "ThemeTreePage.h"
#include "ThemeMenuPage.h"

class CViewStyleDlg;

class ThemeEditor : public CDialog
{
	DECLARE_DYNAMIC(ThemeEditor)

public:
	ThemeEditor( CWnd * topDlg, CViewStyleDlg * pParent );
	~ThemeEditor() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_EDIT };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL DestroyWindow() override;
	void OnOK() override;
	void OnCancel() override;

	void HideAllPages( CWnd * newActivePage );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEditBg();
	afx_msg void OnBnClickedEditList();
	afx_msg void OnBnClickedEditMenu();
	afx_msg void OnBnClickedEditShadow();
	afx_msg void OnBnClickedEditTile();
	afx_msg void OnBnClickedEditTree();
	afx_msg void OnSaveAs();
	afx_msg void OnSave();

public:
	afx_msg void OnThemePreview();

protected:
	CMFCButton _btnBG;
	CMFCButton _btnShadow;
	CMFCButton _btnTile;
	CMFCButton _btnList;
	CMFCButton _btnTree;
	CMFCButton _btnMenu;
	CMFCButton _btnSave;
	CMFCButton _btnSaveAs;

	ThemeBgPage _bgPage;
	ThemeShadowPage _shadowPage;
	ThemeTilePage _tilePage;
	ThemeListPage _listPage;
	ThemeTreePage _treePage;
	ThemeMenuPage _menuPage;

	CWnd * _activePage;
	CViewStyleDlg * _parent;
	CWnd * _topDlg;

public:
	static int refCount;

public:
	CString themeName;
};
