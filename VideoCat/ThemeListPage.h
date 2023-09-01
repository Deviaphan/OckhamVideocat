#pragma once
#include "XPGroupBox.h"

class ViewStyle;

class ThemeListPage : public CDialog
{
	DECLARE_DYNAMIC(ThemeListPage)

public:
	explicit ThemeListPage( CWnd* pParent = nullptr );
	~ThemeListPage() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_LIST };
#endif

public:
	void WriteData( const ViewStyle & style );
	void ReadData( ViewStyle & style );

protected:
	void DoDataExchange(CDataExchange* pDX) override; 

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDeleteFont();

protected:
	CXPGroupBox _listGroup;
	CMFCFontComboBox _fontName;
	CMFCColorButton _btnSelectedColor;
	CMFCColorButton _btnSelectedHoverColor;
	CMFCColorButton _colorLine1;
	CMFCColorButton _colorSelectedLine1;
	CMFCColorButton _colorLine2;
	CMFCColorButton _colorSelectedLine2;
	CMFCColorButton _colorGenre;
	CMFCColorButton _colorSelectedGenre;
	CMFCColorButton _colorTag;
	CMFCColorButton _colorSelectedTag;
	int _sizeLine1;
	int _sizeLine2;
	int _sizeGenre;
	int _sizeTag;
};
