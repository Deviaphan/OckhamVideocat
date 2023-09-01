#pragma once

#include "XPGroupBox.h"

class ViewStyle;


class ThemeTreePage : public CDialog
{
	DECLARE_DYNAMIC(ThemeTreePage)

public:
	explicit ThemeTreePage( CWnd* pParent = nullptr );
	~ThemeTreePage() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_TREE };
#endif
public:
	void WriteData( const ViewStyle & style );
	void ReadData( ViewStyle & style );

protected:
	void DoDataExchange( CDataExchange* pDX ) override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDeleteFont();

protected:
	CXPGroupBox _treeGroup;
	CMFCFontComboBox _fontName;
	CMFCColorButton _color;
	CMFCColorButton _colorSelected;
	int _textSize;
};
