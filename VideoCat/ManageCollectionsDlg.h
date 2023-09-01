#pragma once

#include "AdvancedButton.h"

class CVideoCatDoc;

class ManageCollectionsDlg : public CDialog
{
	DECLARE_DYNAMIC(ManageCollectionsDlg)

public:
	explicit ManageCollectionsDlg( CVideoCatDoc * doc, CWnd* pParent = nullptr );   // standard constructor
	~ManageCollectionsDlg() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MANAGE_COLLECTION };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnOK() override;
	void OnCancel() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAttachCollection();
	afx_msg void OnCreateCollection();
	afx_msg void OnDetachCollection();
	afx_msg void OnEditCollection();
	afx_msg void OnLbnSelchangeCollectionList();
	afx_msg void OnMaintenance();
	afx_msg void OnBackupCollection();

	void RebuildCollectionList();

	inline CVideoCatDoc * GetDocument()
	{
		return _doc;
	}

	void LoadBackup();
	void CreateBackup();

private:
	CVideoCatDoc * _doc;
	CListBox _collectionList;

	AdvancedButton _btnAddCollection;
	AdvancedButton _btnCreateCollection;
	AdvancedButton _btnEditCollection;
	AdvancedButton _btnMaintenance;
	AdvancedButton _btnRemoveCollection;
	AdvancedMenuButton _btnBackupCollection;
	CMenu _menuBackup;

};
