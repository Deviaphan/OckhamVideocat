// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CheckBoxList.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCheckBoxList, CDialog)

CCheckBoxList::CCheckBoxList(CWnd* pParent)
	: CDialog(IDD_CHECK_LIST, pParent)
{

}

CCheckBoxList::~CCheckBoxList()
{
}

void CCheckBoxList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_LIST, _listBox);
}


BEGIN_MESSAGE_MAP(CCheckBoxList, CDialog)
END_MESSAGE_MAP()



BOOL CCheckBoxList::OnInitDialog()
{
	CDialog::OnInitDialog();

	checks.resize( names.size(), BST_CHECKED );
	for( std::vector<CString>::const_iterator ii = names.begin(); ii != names.end(); ++ii )
	{
		int idx = _listBox.AddString( *ii );
		_listBox.SetCheck( idx, checks[idx] );
	}

	return TRUE;
}


void CCheckBoxList::OnOK()
{
	UpdateData();

	for( int i = 0; i < _listBox.GetCount(); ++i )
	{
		checks[i] = _listBox.GetCheck( i ) == BST_CHECKED;
	}

	CDialog::OnOK();
}
