// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CollectionMaintenanceDlg.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CollectionMaintenanceDlg, CDialog)

CollectionMaintenanceDlg::CollectionMaintenanceDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAINTENANCE, pParent)
{

}

CollectionMaintenanceDlg::~CollectionMaintenanceDlg()
{
}

void CollectionMaintenanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STAGE, _stageList );
}


BEGIN_MESSAGE_MAP(CollectionMaintenanceDlg, CDialog)
END_MESSAGE_MAP()


void CollectionMaintenanceDlg::OnOK()
{
	;// CDialog::OnOK();
}

void CollectionMaintenanceDlg::OnCancel()
{
	;// CDialog::OnCancel();
}

void CollectionMaintenanceDlg::AddString( const CString & line )
{
	_stageList.AddString( line );
}

