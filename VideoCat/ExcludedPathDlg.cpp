// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ExcludedPathDlg.h"
#include "Cyrillic.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CExcludedPathDlg, CDialog)

CExcludedPathDlg::CExcludedPathDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EXCLUDED_FILES, pParent)
{

}

CExcludedPathDlg::~CExcludedPathDlg()
{
}

void CExcludedPathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EXCLUDED_FOLDER, _excludeFolder );
	DDX_Control( pDX, IDC_ALL_EXCLUDED_FOLDERS, _excludedList );
}


BEGIN_MESSAGE_MAP(CExcludedPathDlg, CDialog)
	ON_BN_CLICKED( IDC_ADD_EXLUDED, &CExcludedPathDlg::OnAddExluded )
	ON_BN_CLICKED( IDC_REMOVE_EXCLUDED, &CExcludedPathDlg::OnRemoveExcluded )
END_MESSAGE_MAP()



BOOL CExcludedPathDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RebuildList();

	return TRUE;
}

void CExcludedPathDlg::RebuildList()
{
	_excludedList.ResetContent();

	for( const CString & path : excludedPath )
	{
		_excludedList.AddString( path );
	}
}

void CExcludedPathDlg::OnAddExluded()
{
	UpdateData();

	ToLower( _excludeFolder );

	int index = _excludeFolder.Find( rootDir );
	if( index != 0 )
	{
		AfxMessageBox( L"Исключаемая папка находится вне корневой директории.", MB_OK | MB_ICONEXCLAMATION );
		return;
	}
	CString exPath = _excludeFolder.GetString() + rootDir.GetLength();
	if( exPath.GetAt( exPath.GetLength() - 1 ) != L'\\' )
		exPath += L"\\";

	excludedPath.insert( exPath );

	RebuildList();
}


void CExcludedPathDlg::OnRemoveExcluded()
{
	UpdateData();

	const int curSel = _excludedList.GetCurSel();
	if( curSel < 0 )
		return;

	CString text;
	_excludedList.GetText( curSel, text );
	excludedPath.erase( text );

	RebuildList();
}
