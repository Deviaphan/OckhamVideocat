// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "FirstRunDlg.h"
#include "ResString.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(FirstRunDlg, CDialog)

FirstRunDlg::FirstRunDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_FIRST_RUN, pParent)
{

}

FirstRunDlg::~FirstRunDlg()
{
}

void FirstRunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMMENT, _comment );
}


BEGIN_MESSAGE_MAP(FirstRunDlg, CDialog)
END_MESSAGE_MAP()



BOOL FirstRunDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_comment.SetText( ResString( IDS_FIRST_RUN ) );

	return TRUE;
}
