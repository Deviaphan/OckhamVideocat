#pragma once
#include "CatHead.h"
#include "resource.h"

class CWaitDlg : public CDialog
{
	DECLARE_DYNAMIC(CWaitDlg)

public:
	CWaitDlg(CWnd* pParent = nullptr);
	virtual ~CWaitDlg();

	enum { IDD = IDD_WAIT };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );

private:
	CatHead _catHead;
};

class NoMadalWait
{
public:
	NoMadalWait()
		: dlg( 0 )
	{
		dlg = new CWaitDlg;
		dlg->Create( CWaitDlg::IDD );
		dlg->ShowWindow( SW_SHOW );
	}

	~NoMadalWait()
	{
		dlg->DestroyWindow();
		delete dlg;
	}

public:
	CWaitDlg * Get()
	{
		return dlg;
	}

private:
	CWaitDlg * dlg;
};
