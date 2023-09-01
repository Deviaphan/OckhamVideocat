// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CreateCollectionDlg.h"
#include <filesystem>
#include "ResString.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CreateCollectionDlg, CDialog)

CreateCollectionDlg::CreateCollectionDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CREATE_COLLECTION, pParent)
	, title( L"" )
	, path( L"." )
	, isEditMode( false )
{

}

CreateCollectionDlg::~CreateCollectionDlg()
{
}

void CreateCollectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
	DDX_Text( pDX, IDC_COLLECTION_TITLE, title );
	DDX_Control( pDX, IDC_COLLECTION_FOLDER, _pathBrowse );
	DDX_Text( pDX, IDC_COLLECTION_FOLDER, path );
}


BEGIN_MESSAGE_MAP(CreateCollectionDlg, CDialog)
END_MESSAGE_MAP()




BOOL CreateCollectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	 
	if( isEditMode )
	{
		_commentBox.SetText( ResString( IDS_COMMENT_SET_NEW_COLLECTION_TITLE ) );
		_pathBrowse.SetReadOnly( TRUE );
	}
	else
	{
		_commentBox.SetText( ResString( IDS_COMMENT_SET_COLLECTION_DIRECTORY ) );
	}

	return TRUE;
}


void CreateCollectionDlg::OnOK()
{
	UpdateData();

	if( title.IsEmpty() )
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
		AfxMessageBox( ResString( IDS_ERROR_EMPTY_COLLECTION_PATH ), MB_OK | MB_ICONERROR );
		return;
	}

	CDialog::OnOK();
}
