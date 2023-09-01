// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThumbnailSettingsDlg.h"
#include "ResString.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(ThumbnailSettingsDlg, CDialog)

ThumbnailSettingsDlg::ThumbnailSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THUMBNAIL_SETTINGS, pParent)
	, createMode( 0 )
	, selectMode( 0 )
	, sizeMode( 0 )
{

}

ThumbnailSettingsDlg::~ThumbnailSettingsDlg()
{
}

void ThumbnailSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_GROUP_CREATE, _groupCreate );
	DDX_Control( pDX, IDC_GROUP_FRAME, _groupFrame );
	DDX_Control( pDX, IDC_GROUP_SIZE, _groupSize );
	DDX_Control( pDX, IDC_COMMENT_BOX, _commentBox );
	DDX_Radio( pDX, IDC_CREATE_NEW, createMode );
	DDX_Radio( pDX, IDC_AUTO_FRAMES, selectMode );
	DDX_Radio( pDX, IDC_FULLSIZE, sizeMode );
}


BEGIN_MESSAGE_MAP(ThumbnailSettingsDlg, CDialog)
	ON_BN_CLICKED( IDOK, &ThumbnailSettingsDlg::OnBnClickedOk )
END_MESSAGE_MAP()

void ThumbnailSettingsDlg::OnOK()
{
	;// CDialog::OnOK();
}

BOOL ThumbnailSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if( createMode == 1 )
	{
		GetDlgItem( IDC_CREATE_NEW )->EnableWindow( FALSE );
		GetDlgItem( IDC_CREATE_ALL )->EnableWindow( FALSE );
	}

	_commentBox.SetWindowText( ResString( IDS_TASK_THUMBS_INFO ) );

	return TRUE;
}

void ThumbnailSettingsDlg::OnBnClickedOk()
{
	UpdateData();
	CDialog::OnOK();
}


