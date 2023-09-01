#pragma once
#include "AdvancedToolTipCtrl.h"
#include "ViewStyle.h"
#include <vector>

class CVideoCatDoc;

enum
{
	ViewTileIndex = 0,
	ViewSeriesIndex = 1,
	ViewListIndex = 2
};

class CViewStyleDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CViewStyleDlg)

public:
	CViewStyleDlg();
	virtual ~CViewStyleDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEW_STYLE };
#endif

public:
	void UpdateView();
	void BuildThemeList();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL PreTranslateMessage( MSG* pMsg );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnApplyTheme();
	afx_msg void OnCreateTheme();
	afx_msg void OnDeleteTheme();
	afx_msg void OnEditTheme();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar * pScrollBar );
	afx_msg void OnSelchangeTheme();

private:
	AdvancedToolTipCtrl _tooltip;
	CListBox _themeList;

	std::vector<CString> _themeNames;
	CSliderCtrl _scaleSlider;

public:
	ViewStyle _theme;

	CVideoCatDoc * _doc;
	ViewStyle * _currentTheme;

	int viewType;
	int viewScale;
};

void SetWin10ThemeByColor( CVideoCatDoc * doc, COLORREF color );
