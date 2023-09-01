#pragma once
#include "CommentButton.h"

class CreateRootDlg : public CDialog
{
	DECLARE_DYNAMIC(CreateRootDlg)

public:
	explicit CreateRootDlg( CWnd* pParent = nullptr );
	~CreateRootDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATE_ROOT_DIRECTORY };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCreateRealRoot();
	afx_msg void OnCreateVirtualRoot();

private:
	CommentButton _realBtn;
	CommentButton _virtBtn;

public:
	bool isRealRoot; // реальная или виртуальная директория
	CString title; // название корневой директории
	CString path; // путь до папки, только для реальных директорий
	bool useHiddenFolder; // искать в скрытых папках
	bool useFileInFolder; // каждый фильм лежит в папке
};
