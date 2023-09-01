#pragma once
#include "CommentBox.h"

class CreateCollectionDlg : public CDialog
{
	DECLARE_DYNAMIC(CreateCollectionDlg)

public:
	CreateCollectionDlg(CWnd* pParent = nullptr);
	~CreateCollectionDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATE_COLLECTION };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

private:
	CMFCEditBrowseCtrl _pathBrowse;
	CCommentBox _commentBox;

public:
	CString title;
	CString path;
	bool isEditMode;
};
