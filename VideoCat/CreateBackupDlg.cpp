// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CreateBackupDlg.h"
#include "ResString.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CreateBackupDlg, CDialog)

CreateBackupDlg::CreateBackupDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CREATE_BACKUP, pParent)
	, filePath( _T( "" ) )
	, addCollection( TRUE )
	, addProgress( TRUE )
	, addFrames( TRUE )
{

}

CreateBackupDlg::~CreateBackupDlg()
{
}

void CreateBackupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_BACKUP_PATH, filePath );
	DDX_Check( pDX, IDC_BACKUP_COLLECTION, addCollection );
	DDX_Check( pDX, IDC_BACKUP_PROGRESS, addProgress );
	DDX_Check( pDX, IDC_BACKUP_FRAMES, addFrames );
	DDX_Control( pDX, IDC_COMMENT, _comment );
	DDX_Control( pDX, IDC_BACKUP_PATH, _path );
}


BEGIN_MESSAGE_MAP(CreateBackupDlg, CDialog)
END_MESSAGE_MAP()


// CreateBackupDlg message handlers


BOOL CreateBackupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText( collectionTitle );
	_comment.SetText( ResString( IDS_COMMENT_CREATE_BACKUP ) );

	_path.EnableFileBrowseButton( L"vcbackup", ResString( IDS_BACKUP_FILTER ) );

	return TRUE;
}
