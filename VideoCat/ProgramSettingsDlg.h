#pragma once

#include "ShortcutDlg.h"
#include "FileFormatSettings.h"
#include "BehaviorDlg.h"
#include "ViewStyleDlg.h"
//#include "ProxySettings.h"
#include "VideoCatDoc.h"
#include "resource.h"

class CProgramSettings : public CDialog
{
	DECLARE_DYNAMIC(CProgramSettings)

public:
	explicit CProgramSettings( CVideoCatDoc * doc );
	~CProgramSettings() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;
	void OnCancel() override;
	BOOL DestroyWindow() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTcnSelchangeSettingsTab(NMHDR *pNMHDR, LRESULT *pResult);

private:
	CTabCtrl _tabCtrl;
	CWnd * _currentPage;
	CVideoCatDoc * _doc;

public:
	CShortcutDlg shortcutDlg;
	CBehaviorDlg behaviorDlg;
	CViewStyleDlg viewStyleDlg;
	CFileFormatSettings fileFormatSettings;
	//CProxySettings proxySettings;

	static int refCount;
};
