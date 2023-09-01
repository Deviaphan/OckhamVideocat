// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CopyFolderDataDlg.h"
#include "ResString.h"
#include "resource.h"



IMPLEMENT_DYNAMIC(CopyFolderData, CDialog)

CopyFolderData::CopyFolderData(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_COPY_FOLDER_DATA, pParent)
{

}

CopyFolderData::~CopyFolderData()
{
}

void CopyFolderData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
	DDX_Check( pDX, IDC_COPY_DESCROPTION, data.description );
	DDX_Check( pDX, IDC_COPY_POSTER, data.poster );
	DDX_Check( pDX, IDC_COPY_TAGS, data.tags );
	DDX_Check( pDX, IDC_COPY_COMMENT, data.comments );
}


BEGIN_MESSAGE_MAP(CopyFolderData, CDialog)
END_MESSAGE_MAP()



BOOL CopyFolderData::OnInitDialog()
{
	CDialog::OnInitDialog();

	_commentBox.SetText( ResString( IDS_COMMENT_COPY_FOLDER ) );

	return TRUE;
}
