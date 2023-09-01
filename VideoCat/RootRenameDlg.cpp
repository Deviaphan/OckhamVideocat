// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "RootRenameDlg.h"
#include "ResString.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(RootRenameDlg, CDialog)

RootRenameDlg::RootRenameDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_ROOT_TITLE, pParent)
	, comment( ResString(IDS_COMMENT_RENAME_ROOT) )
{

}

RootRenameDlg::~RootRenameDlg()
{
}

void RootRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_TITLE, title );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
	DDX_Text( pDX, IDC_COMMENT, comment );
}


BEGIN_MESSAGE_MAP(RootRenameDlg, CDialog)
END_MESSAGE_MAP()



void RootRenameDlg::OnOK()
{
	UpdateData();

	if( title.IsEmpty() )
	{
		AfxMessageBox( L"Не задано название корневой директории", MB_OK | MB_ICONERROR );
		return;
	}

	CDialog::OnOK();
}
