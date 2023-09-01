#pragma once
#include "XPGroupBox.h"

class ViewStyle;

class ThemeTilePage : public CDialog
{
	DECLARE_DYNAMIC(ThemeTilePage)

public:
	explicit ThemeTilePage( CWnd* pParent = nullptr );
	~ThemeTilePage() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_TILE };
#endif

public:
	void WriteData( const ViewStyle & style );
	void ReadData( ViewStyle & style );

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDeleteFont();

protected:
	CXPGroupBox _tileGroup;
	CMFCColorButton _btnSelectedColor;
	CMFCColorButton _btnSelectedHoverColor;
	CMFCFontComboBox _fontName;
	CMFCColorButton _colorLine1;
	CMFCColorButton _colorSelectedLine1;
	CMFCColorButton _colorLine2;
	CMFCColorButton _colorSelectedLine2;
	int _sizeLine1;
	int _sizeLine2;
};
