// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AddCollectionDlg.h"
#include <filesystem>
#include <system_error>
#include "ResString.h"
#include "resource.h"


// AddCollectionDlg dialog

IMPLEMENT_DYNAMIC( AddCollectionDlg, CDialog )

AddCollectionDlg::AddCollectionDlg( CWnd * pParent /*=nullptr*/ )
	: CDialog( IDD_ADD_COLLECTION, pParent )
	, title( L"" )
	, path( L"" )
	, renameTitle( FALSE )
{

}

AddCollectionDlg::~AddCollectionDlg()
{
}

void AddCollectionDlg::DoDataExchange( CDataExchange * pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_RENAME_COLLECTION, renameTitle );
	DDX_Control( pDX, IDC_COLLECTION_TITLE, _titleBox );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
	DDX_Control( pDX, IDC_LOAD_COLLECTION, _pathBrowse );
	DDX_Text( pDX, IDC_COLLECTION_TITLE, title );
	DDX_Text( pDX, IDC_LOAD_COLLECTION, path );
}


BEGIN_MESSAGE_MAP( AddCollectionDlg, CDialog )
	ON_BN_CLICKED( IDC_RENAME_COLLECTION, &AddCollectionDlg::OnBnClickedRenameCollection )
END_MESSAGE_MAP()


BOOL AddCollectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_pathBrowse.EnableFileBrowseButton( L"Ockham:VideoCat", L"ockham|*.ockham||" );

	_commentBox.SetText( ResString( IDS_COMMENT_ADD_COLLECTION ) );

	return TRUE;
}

void AddCollectionDlg::OnOK()
{
	UpdateData();

	if( renameTitle && title.IsEmpty() )
	{
		AfxMessageBox( ResString( IDS_ERROR_NO_COLLECTION_TITLE ), MB_OK | MB_ICONERROR );
		return;
	}

	bool pathError = false;

	if( path.IsEmpty() )
	{
		pathError = true;
	}
	else if( path.GetLength() == 1 && path[0] != L'.' )
	{
		pathError = true;
	}
	else
	{
		std::error_code errCode;
		if( false == std::filesystem::exists( std::filesystem::path( path.GetString() ), errCode ) )
		{
			pathError = true;
		}
	}

	if( pathError )
	{
		AfxMessageBox( ResString( IDS_ERROR_NO_COLLECTION_FILENAME ), MB_OK | MB_ICONERROR );
		return;
	}

	CDialog::OnOK();
}



void AddCollectionDlg::OnBnClickedRenameCollection()
{
	UpdateData();

	_titleBox.EnableWindow( renameTitle );
}
