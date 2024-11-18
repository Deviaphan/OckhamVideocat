// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AddPerson.h"
#include <string>
#include <algorithm>
#include "resource.h"


IMPLEMENT_DYNAMIC(CAddPerson, CDialog)

CAddPerson::CAddPerson(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EDIT_PERSON, pParent)
	, _personId( 0 )
	, _filmId( 0 )
{

}

CAddPerson::~CAddPerson()
{
}

void CAddPerson::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_URL, _personId );
	DDX_Control( pDX, IDC_FILM_LIST, _filmListCtrl );
	DDX_Text( pDX, IDC_FILM_ID, _filmId );
}


BEGIN_MESSAGE_MAP(CAddPerson, CDialog)
	ON_BN_CLICKED( IDC_ADD, &CAddPerson::OnAddPerson )
	ON_BN_CLICKED( IDC_REMOVE, &CAddPerson::OnRemoveSelected )
END_MESSAGE_MAP()


BOOL CAddPerson::OnInitDialog()
{
	CDialog::OnInitDialog();
	UpdateList();

	return TRUE;  
}

void CAddPerson::UpdateList()
{
	_filmListCtrl.ResetContent();

	std::sort( _filmList.begin(), _filmList.end() );
	for( auto i : _filmList )
	{
		_filmListCtrl.AddString( std::to_wstring( i ).c_str() );
	}
}

void CAddPerson::OnAddPerson()
{
	UpdateData();

	if( std::find( _filmList.begin(), _filmList.end(), _filmId ) != _filmList.end() )
		return;

	_filmList.push_back( _filmId );

	UpdateList();
}

void CAddPerson::OnRemoveSelected()
{
	const int curSel = _filmListCtrl.GetCurSel();
	if( curSel < 0 )
		return;

	_filmList.erase( _filmList.begin() + curSel );
	UpdateList();
}
