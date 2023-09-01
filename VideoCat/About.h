#pragma once
#include "CatHead.h"
#include "License/LicenseBox.h"
#include "AdvancedButton.h"

class CAboutDlg: public CDialog
{
public:
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_ABOUTBOX
	};
#endif

	CAboutDlg();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );

	void AllowUpdateButton();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT TestMessageCall( WPARAM, LPARAM );
	afx_msg void OnBuy();
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnPaint();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnVkSubscribe();
	afx_msg void OnInstallUpdate();

private:
	AdvancedButton _btnSubscribe;
	AdvancedButton _btnShopping;
	CListBox _licenseInfo;

	CatHead _catHead;
	LicenseBox _licenseBox;
	bool _hasLicense;

	CString _versionText;
	CString _programVersion;
	CString _downloadUrl;

	friend void CheckUpdateData( CAboutDlg * dlg );
};
