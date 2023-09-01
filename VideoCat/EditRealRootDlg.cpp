// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "EditRealRootDlg.h"
#include "resource.h"



IMPLEMENT_DYNAMIC(EditRealRootDlg, CDialog)

EditRealRootDlg::EditRealRootDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EDIT_REAL_ROOT, pParent)
	, title( _T( "" ) )
	, path( _T( "" ) )
	, useFileInFolder( FALSE )
	, _comment( _T( "" ) )
{

}

EditRealRootDlg::~EditRealRootDlg()
{
}

void EditRealRootDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_TITLE, title );
	DDX_Control( pDX, IDC_ROOT_PATH, _browsePath );
	DDX_Text( pDX, IDC_ROOT_PATH, path );
	DDX_Check( pDX, IDC_CHK_INNER_FILMS, useFileInFolder );
	DDX_Text( pDX, IDC_COMMENT, _comment );
	DDX_Control( pDX, IDC_COMMENT, _commentBox );
}


BEGIN_MESSAGE_MAP(EditRealRootDlg, CDialog)
END_MESSAGE_MAP()



void EditRealRootDlg::OnOK()
{
	UpdateData();

	if( title.IsEmpty() )
	{
		AfxMessageBox( L"Не задано название корневой директории", MB_OK | MB_ICONERROR | MB_TOPMOST );
		return;
	}

	if( path.IsEmpty() )
	{
		AfxMessageBox( L"Не выбрана папка с фильмами", MB_OK | MB_ICONERROR | MB_TOPMOST );
		return;
	}

	CDialog::OnOK();

	// путь должен заканчиваться слэшем
	if( path.GetAt( path.GetLength() - 1 ) != L'\\' )
	{
		path += L"\\";
	}
}


BOOL EditRealRootDlg::OnInitDialog()
{
	_oldPath = path;

	_comment = L"Будьте внимательны при выборе папки, задающей корневую директорию. При выборе неправильной директории и выполнении синхронизации, старые записи могут быть утеряны.";

	CDialog::OnInitDialog();

	return TRUE;
}
