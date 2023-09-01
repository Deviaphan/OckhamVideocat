#pragma once
#include "CommentBox.h"


class AddCollectionDlg : public CDialog
{
	DECLARE_DYNAMIC(AddCollectionDlg)

public:
	explicit AddCollectionDlg( CWnd* pParent = nullptr );
	~AddCollectionDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADD_COLLECTION };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedRenameCollection();

private:
	CEdit _titleBox;
	CMFCEditBrowseCtrl _pathBrowse;
	CCommentBox _commentBox;

public:
	CString title;
	CString path;
	BOOL renameTitle;
};
