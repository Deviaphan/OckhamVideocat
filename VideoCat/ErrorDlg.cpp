// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ErrorDlg.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CErrorDlg, CDialog)

CErrorDlg::CErrorDlg( const CString & msg, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PROGRAM_ERROR, pParent)
	, message( msg )
{

}

CErrorDlg::~CErrorDlg()
{
}

void CErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_ERROR_TEXT, message );
}


BEGIN_MESSAGE_MAP(CErrorDlg, CDialog)
END_MESSAGE_MAP()

bool ShowError( const CString & msg, bool ignore )
{
	if( ignore )
		return ignore;

	CErrorDlg dlg( msg );

	return dlg.DoModal() != IDOK;
}