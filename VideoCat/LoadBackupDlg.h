#pragma once
#include "CommentBox.h"

class LoadBackupDlg : public CDialog
{
	DECLARE_DYNAMIC(LoadBackupDlg)

public:
	explicit LoadBackupDlg( CWnd* pParent = nullptr );
	~LoadBackupDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOAD_BACKUP };
#endif

	BOOL OnInitDialog() override;

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()
private:
	CMFCEditBrowseCtrl _backupFile;
	CMFCEditBrowseCtrl _extractFolder;
	CCommentBox _comment;

public:
	CString backupPath;
	CString extractFolder;
};
