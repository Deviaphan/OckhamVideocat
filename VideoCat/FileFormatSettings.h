#pragma once
#include "Extensions.h"
#include "resource.h"


class CFileFormatSettings : public CPropertyPage
{
	DECLARE_DYNAMIC(CFileFormatSettings)

public:
	CFileFormatSettings();
	virtual ~CFileFormatSettings();

	enum { IDD = IDD_FILE_FORMATS };

public:
	void SaveLastState();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnAddFormat();
	afx_msg void OnBnClickedBtnRemoveFormat();
	afx_msg void OnLbnSelchangeListFormats();

protected:
	void UpdateList();

private:
	CListBox _extList;
	CMFCEditBrowseCtrl _browsePlayer;

	CString _commandLine;
	int _playerType;
	int _lastSel;
	BOOL _special3d;

public:
	ExtensionsList extensions;
};
