#pragma once
#include "CatHead.h"

class CShortcutDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CShortcutDlg)

public:
	CShortcutDlg();
	virtual ~CShortcutDlg();

#ifdef AFX_DESIGN_TIME
	enum
	{	IDD = IDD_SHORTCUT };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCreateShortcut();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );

private:
	CString _shortcutName;
	CatHead _catHead;
};
