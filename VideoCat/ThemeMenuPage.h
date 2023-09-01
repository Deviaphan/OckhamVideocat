#pragma once
#include "XPGroupBox.h"

class ViewStyle;

class ThemeMenuPage : public CDialog
{
	DECLARE_DYNAMIC(ThemeMenuPage)

public:
	explicit ThemeMenuPage( CWnd* pParent = nullptr );
	~ThemeMenuPage() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_MENU };
#endif

public:
	void WriteData( const ViewStyle & style );
	void ReadData( ViewStyle & style );

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDeleteFont();

protected:
	CXPGroupBox _menuGroup;
	CMFCFontComboBox _font;
	CMFCColorButton _iconColor;
	CMFCColorButton _iconSelectedColor;
	CMFCColorButton _textColor;
	CMFCColorButton _textSelectedColor;

	int _sizeIcon;
	int _sizeText;
};
