#pragma once

#include "CommentBox.h"

class DeleteRootDlg : public CDialog
{
	DECLARE_DYNAMIC(DeleteRootDlg)

public:
	explicit DeleteRootDlg(CWnd* pParent = nullptr);
	~DeleteRootDlg() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_ROOT };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnDelete();

private:
	CCommentBox _commentBox;

public:
	CString comment;
};
