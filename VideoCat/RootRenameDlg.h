#pragma once

#include "CommentBox.h"

class RootRenameDlg : public CDialog
{
	DECLARE_DYNAMIC(RootRenameDlg)

public:
	explicit RootRenameDlg(CWnd* pParent = nullptr);
	~RootRenameDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ROOT_TITLE };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

private:
	CCommentBox _commentBox;

public:
	CString title;
	CString comment;
};
