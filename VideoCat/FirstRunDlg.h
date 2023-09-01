#pragma once

#include "CommentBox.h"

class FirstRunDlg : public CDialog
{
	DECLARE_DYNAMIC(FirstRunDlg)

public:
	FirstRunDlg(CWnd* pParent = nullptr);
	virtual ~FirstRunDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FIRST_RUN };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

protected:
	CCommentBox _comment;
};
