#pragma once

#include <vector>
#include "Commands/CommandExecute.h"

class EditToolbarDlg : public CDialog
{
	DECLARE_DYNAMIC(EditToolbarDlg)

public:
	EditToolbarDlg(CWnd* pParent = nullptr);
	virtual ~EditToolbarDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_TOOLBAR };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	void UpdateToolbarList();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLbnDblclkToolbarList();
	afx_msg void OnLbnDblclkCommandList();
	afx_msg void OnBnClickedMoveUp();
	afx_msg void OnBnClickedMoveDown();

private:
	CListBox _toolbarList;
	CListBox _commandList;

	std::vector<CommandName> _allCommands;
	std::vector<CommandID> _selectedCommands;
};
