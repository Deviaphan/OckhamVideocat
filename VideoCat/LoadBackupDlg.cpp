// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "LoadBackupDlg.h"
#include "ResString.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(LoadBackupDlg, CDialog)

LoadBackupDlg::LoadBackupDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LOAD_BACKUP, pParent)
	, backupPath( _T( "" ) )
	, extractFolder( _T( "" ) )
{

}

LoadBackupDlg::~LoadBackupDlg()
{
}

void LoadBackupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BACKUP_FILE, _backupFile );
	DDX_Text( pDX, IDC_BACKUP_FILE, backupPath );
	DDX_Text( pDX, IDC_EXTRACT_PATH, extractFolder );
	DDX_Control( pDX, IDC_COMMENT, _comment );
	DDX_Control( pDX, IDC_EXTRACT_PATH, _extractFolder );
}


BEGIN_MESSAGE_MAP(LoadBackupDlg, CDialog)
END_MESSAGE_MAP()




BOOL LoadBackupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_comment.SetWindowText( ResString( IDS_COMMENT_LOAD_BACKUP ) );

	_backupFile.EnableFileBrowseButton( L"vcbackup", ResString(IDS_BACKUP_FILTER) );

	_extractFolder.EnableFolderBrowseButton( 0, BIF_NEWDIALOGSTYLE );
	return TRUE; 
}
