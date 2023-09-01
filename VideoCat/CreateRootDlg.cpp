// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CreateRootDlg.h"
#include "ResString.h"
#include "RootRenameDlg.h"
#include "EditRealRootDlg.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CreateRootDlg, CDialog)

CreateRootDlg::CreateRootDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CREATE_ROOT_DIRECTORY, pParent)
	, isRealRoot( true )
	, title()
	, path()
	, useHiddenFolder( false )
	, useFileInFolder( false )
{

}

CreateRootDlg::~CreateRootDlg()
{
}

void CreateRootDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CREATE_REAL_ROOT, _realBtn );
	DDX_Control( pDX, IDC_CREATE_VIRTUAL_ROOT, _virtBtn );
}


BEGIN_MESSAGE_MAP(CreateRootDlg, CDialog)
	ON_BN_CLICKED( IDC_CREATE_REAL_ROOT, &CreateRootDlg::OnCreateRealRoot )
	ON_BN_CLICKED( IDC_CREATE_VIRTUAL_ROOT, &CreateRootDlg::OnCreateVirtualRoot )
END_MESSAGE_MAP()

void CreateRootDlg::OnOK()
{
	;// CDialog::OnOK();
}

BOOL CreateRootDlg::OnInitDialog()
{
	_realBtn.comment = ResString( IDS_COMMENT_ROOT_REAL );

	_virtBtn.comment = ResString( IDS_COMMENT_ROOT_VIRTUAL );

	CDialog::OnInitDialog();

	return TRUE;
}


void CreateRootDlg::OnCreateRealRoot()
{
	ShowWindow( SW_HIDE );

	EditRealRootDlg dlg;

	dlg.title = title;
	dlg.path = path;
	dlg.useFileInFolder = useFileInFolder;

	const INT_PTR result = dlg.DoModal();

	ShowWindow( SW_SHOW );

	if( result != IDOK )
		return;

	isRealRoot = true;
	title = dlg.title;
	path = dlg.path;
	useFileInFolder = dlg.useFileInFolder;

	CDialog::OnOK();
}


void CreateRootDlg::OnCreateVirtualRoot()
{
	ShowWindow( SW_HIDE );

	RootRenameDlg dlg;
	const INT_PTR result = dlg.DoModal();

	ShowWindow( SW_SHOW );

	if( result != IDOK )
		return;

	isRealRoot = false;
	title = dlg.title;

	CDialog::OnOK();
}

