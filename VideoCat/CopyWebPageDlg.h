#pragma once

#include "explorer.h"

class CopyWebPageDlg : public CDialog
{
	DECLARE_DYNAMIC(CopyWebPageDlg)

public:
	CopyWebPageDlg(CWnd* pParent = nullptr);
	virtual ~CopyWebPageDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COPY_PAGE };
#endif

protected:
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );
	afx_msg void OnBnClickedCopy();

protected:
	CExplorer _browser;
	int _minX;
	int _minY;

public:
	CString startPageUrl;
	CString pageData; // utf-8
	CStringA pageCharset;
};
