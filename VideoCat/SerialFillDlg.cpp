// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "VideoCat.h"
#include "SerialFillDlg.h"


IMPLEMENT_DYNAMIC(SerialFillDlg, CDialog)

SerialFillDlg::SerialFillDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SERIAL_FILL, pParent)
	, fillType( 0 )
{

}

SerialFillDlg::~SerialFillDlg()
{
}

void SerialFillDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio( pDX, IDC_TYPE_LOCAL, fillType );
}


BEGIN_MESSAGE_MAP(SerialFillDlg, CDialog)
END_MESSAGE_MAP()


