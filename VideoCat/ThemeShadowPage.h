#pragma once

#include "XPGroupBox.h"

class ViewStyle;

class ThemeShadowPage : public CDialog
{
	DECLARE_DYNAMIC(ThemeShadowPage)

public:
	explicit ThemeShadowPage( CWnd* pParent = nullptr );
	~ThemeShadowPage() override;

	BOOL OnInitDialog() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_SHADOW };
#endif
public:
	void WriteData( const ViewStyle & style );
	void ReadData( ViewStyle & style );

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()

protected:
	CXPGroupBox _shadowGroup1;
	CMFCColorButton _shadowColor;
	CMFCColorButton _markerColor;
	CSpinButtonCtrl _shadowSpiner;
	CSpinButtonCtrl _markerSpiner;
	int _shadowOpacity;
	int _markerOpacity;
};
