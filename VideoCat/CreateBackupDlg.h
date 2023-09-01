#pragma once

#include "CommentBox.h"

class CreateBackupDlg : public CDialog
{
	DECLARE_DYNAMIC(CreateBackupDlg)

public:
	explicit CreateBackupDlg( CWnd* pParent = nullptr );
	~CreateBackupDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATE_BACKUP };
#endif

	BOOL OnInitDialog() override;
protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()

public:
	CString filePath;
	CString collectionTitle;
	BOOL addCollection;
	BOOL addProgress;
	BOOL addFrames;

private:
	CCommentBox _comment;
	CMFCEditBrowseCtrl _path;
};
