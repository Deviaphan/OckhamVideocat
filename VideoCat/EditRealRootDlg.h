#pragma once
#include "CommentBox.h"

class EditRealRootDlg : public CDialog
{
	DECLARE_DYNAMIC(EditRealRootDlg)

public:
	explicit EditRealRootDlg( CWnd* pParent = nullptr );
	~EditRealRootDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_REAL_ROOT };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

protected:
	CMFCEditBrowseCtrl _browsePath;
	CCommentBox _commentBox;
	CString _comment;
	CString _oldPath;

public:
	CString title;
	CString path;
	BOOL useFileInFolder;
};
