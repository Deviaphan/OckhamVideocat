#pragma once
#include "XPGroupBox.h"

class ViewStyle;

class ThemeBgPage : public CDialog
{
	DECLARE_DYNAMIC(ThemeBgPage)

public:
	explicit ThemeBgPage( CWnd* pParent = nullptr );
	~ThemeBgPage() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_BG };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;
	void OnCancel() override;

public:
	void WriteData( const ViewStyle & style );
	void ReadData( ViewStyle & style );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDeleteBg();

protected:
	CXPGroupBox _bgGroup;
	CMFCEditBrowseCtrl _imageFile;
	CMFCColorButton _topColor;
	CMFCColorButton _bottomColor;
	CMFCColorButton _borderColor;
	CMFCColorButton _glassColor;
	CSpinButtonCtrl _glassSpiner;
	CSpinButtonCtrl _borderSpiner;
	CSpinButtonCtrl _blurSpiner;
	int _borderOpacity;
	int _glassBlur;
	int _glassOpacity;
};
