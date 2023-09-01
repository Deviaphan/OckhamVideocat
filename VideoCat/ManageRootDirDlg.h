#pragma once

#include "CommentBox.h"
#include "AdvancedMenu.h"

class CVideoCatDoc;

class ManageRootDirDlg : public CDialog
{
	DECLARE_DYNAMIC(ManageRootDirDlg)

public:
	explicit ManageRootDirDlg( CVideoCatDoc * doc, CWnd* pParent = nullptr );
	~ManageRootDirDlg() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MANAGE_ROOT_DIRECTORY };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;
	void OnCancel() override;
	BOOL OnInitDialog() override;

	inline CVideoCatDoc * GetDocument()
	{
		return _doc;
	}

	void UpdateRootList();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCreateRootBtn();
	afx_msg void OnBnClickedDeleteRootBtn();
	afx_msg void OnBnClickedExcludeFolderBtn();
	afx_msg void OnBnClickedEditRootBtn();
	afx_msg void OnImportExport();
	afx_msg void OnLbnSelchangeRootList();

private:
	CListBox _rootList;
	CMFCButton _btnCreateRoot;
	CMFCButton _btnRenameRoot;
	CMFCButton _btnExcludeDir;
	CMFCButton _btnDeleteRoot;
	CCommentBox _commentBox;
	CMFCMenuButton _btnImportExport;
	adv_mfc::AdvancedMenu _menuImportExport;

private:
	CVideoCatDoc * _doc;
	CString _comment;
};
