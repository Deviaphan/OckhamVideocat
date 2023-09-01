// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "DeleteRootDlg.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(DeleteRootDlg, CDialog)

DeleteRootDlg::DeleteRootDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DELETE_ROOT, pParent)
{

}

DeleteRootDlg::~DeleteRootDlg()
{
}

void DeleteRootDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
	DDX_Text( pDX, IDC_COMMENT, comment );
}


BEGIN_MESSAGE_MAP(DeleteRootDlg, CDialog)
	ON_BN_CLICKED( IDC_BTN_DELETE, &DeleteRootDlg::OnBtnDelete )
END_MESSAGE_MAP()

void DeleteRootDlg::OnOK()
{
	//CDialog::OnOK();
}


void DeleteRootDlg::OnBtnDelete()
{
	CDialog::OnOK();
}




