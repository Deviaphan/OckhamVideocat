// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "RenameDialog.h"
#include "ResString.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CRenameDialog, CDialog)

CRenameDialog::CRenameDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_RENAME, pParent)
	, numFiles(0)
{

}

CRenameDialog::~CRenameDialog()
{
}

void CRenameDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_USE_YEAR, nameFormat.useYear );
	DDX_Check( pDX, IDC_USE_RUS, nameFormat.useRu );
	DDX_Check( pDX, IDC_USE_EN, nameFormat.useEn );
	DDX_Text( pDX, IDC_PRE_EPISODE, nameFormat.preEpisode );
	DDX_Text( pDX, IDC_POST_EPISODE, nameFormat.postEpisode );
	DDX_Text( pDX, IDC_PRE_YEAR, nameFormat.preYear );
	DDX_Text( pDX, IDC_POST_YEAR, nameFormat.postYear );
	DDX_Text( pDX, IDC_PRE_RU, nameFormat.preRu );
	DDX_Text( pDX, IDC_POST_RU, nameFormat.postRu );
	DDX_Text( pDX, IDC_PRE_EN, nameFormat.preEn );
	DDX_Text( pDX, IDC_POST_EN, nameFormat.postEn );
	DDX_Control( pDX, IDC_ORDER, _orderList );
	DDX_Control( pDX, IDC_EXAMPLE, _exampleList );
}


BEGIN_MESSAGE_MAP(CRenameDialog, CDialog)
	ON_BN_CLICKED( IDC_BTN_UP, &CRenameDialog::OnBtnUp )
	ON_BN_CLICKED( IDC_BTN_DOWN, &CRenameDialog::OnBtnDown )
	ON_BN_CLICKED( IDC_USE_YEAR, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_PRE_EPISODE, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_POST_EPISODE, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_PRE_YEAR, &CRenameDialog::RefreshExampleList  )
	ON_EN_CHANGE( IDC_POST_YEAR, &CRenameDialog::RefreshExampleList )
	ON_BN_CLICKED( IDC_USE_RUS, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_PRE_RU, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_POST_RU, &CRenameDialog::RefreshExampleList )
	ON_BN_CLICKED( IDC_USE_EN, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_PRE_EN, &CRenameDialog::RefreshExampleList )
	ON_EN_CHANGE( IDC_POST_EN, &CRenameDialog::RefreshExampleList )
END_MESSAGE_MAP()



BOOL CRenameDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	_orderList.AddString( L"Год" );
	_orderList.AddString( L"Основное название" );
	_orderList.AddString( L"Дополнительное название" );

	_orderList.SetItemData( 0, 0 );
	_orderList.SetItemData( 1, 1 );
	_orderList.SetItemData( 2, 2 );

	CString okText;
	GetDlgItemText( IDOK, okText );

	CString newOk;
	newOk.Format( L"%s %u файл(ов)", okText.GetString(), numFiles );
	SetDlgItemText( IDOK, newOk );

	RefreshExampleList();
	return TRUE;
}


void CRenameDialog::OnBtnUp()
{
	const int curSel = _orderList.GetCurSel();
	if( curSel < 1 )
		return;

	int id = (int)_orderList.GetItemData( curSel );
	CString title;
	_orderList.GetText( curSel, title );

	_orderList.DeleteString( curSel );
	_orderList.InsertString( curSel - 1, title );
	_orderList.SetItemData( curSel - 1, id );
	_orderList.SetCurSel( curSel - 1 );
	std::swap( nameFormat.order[curSel], nameFormat.order[curSel - 1] );

	RefreshExampleList();
}


void CRenameDialog::OnBtnDown()
{
	const int curSel = _orderList.GetCurSel();
	if( curSel < 0 || curSel == 2 )
		return;

	int id = (int)_orderList.GetItemData( curSel );
	CString title;
	_orderList.GetText( curSel, title );

	_orderList.DeleteString( curSel );
	_orderList.InsertString( curSel+1, title );
	_orderList.SetItemData( curSel+1, id );
	_orderList.SetCurSel( curSel + 1 );
	std::swap( nameFormat.order[curSel], nameFormat.order[curSel + 1] );

	RefreshExampleList();
}

void CRenameDialog::RefreshExampleList()
{
	UpdateData();

	_exampleList.ResetContent();

	for( const Film & f : exampleNames )
	{
		_exampleList.AddString( BuildNewName( nameFormat, f.rus, f.en, f.year, f.episode ) );
	}
}

CString BuildNewName( const NameFormat & nameFormat, const std::wstring & ru, const std::wstring & en, unsigned short y, const std::wstring & episode )
{
	CString newName;
	
	if( !episode.empty() )
	{
		// перед идентификатором сезона могут быть только НЕ буквенно-цифровые символы, тчобы потом можно было сделать распознавание. Все остальные символы пропускаем
		if( !nameFormat.preEpisode.IsEmpty() )
		{
			for( int i = 0; i < nameFormat.preEpisode.GetLength(); ++i )
			{
				wchar_t c = nameFormat.preEpisode[i];
				if( !iswalnum( c ) )
					newName += c;
			}
		}

		newName += episode.c_str();
		newName += nameFormat.postEpisode;
	}

	CString partYear;
	partYear.Format( L"%s%i%s", nameFormat.preYear.GetString(), (int)y, nameFormat.postYear.GetString() );

	CString partRu = ResFormat( L"%s%s%s", nameFormat.preRu.GetString(), ru.c_str(), nameFormat.postRu.GetString() );

	CString partEn = ResFormat( L"%s%s%s", nameFormat.preEn.GetString(), en.c_str(), nameFormat.postEn.GetString() );

	for(int order : nameFormat.order)
	{
		switch( order )
		{
			case 0:
				if( nameFormat.useYear && (y > 0) )
					newName += partYear;
				break;
			case 1:
				if( nameFormat.useRu && !ru.empty() )
					newName += partRu;
				break;
			case 2:
				if( nameFormat.useEn && !en.empty() )
					newName += partEn;
				break;
		}
	}

	newName.Replace( L':', L' ' );
	newName.Replace( L'*', L' ' );
	newName.Replace( L'?', L' ' );
	newName.Replace( L'\\', L' ' );
	newName.Replace( L'/', L' ' );
	newName.Replace( L'|', L' ' );
	newName.Replace( L'%', L' ' );
	newName.Replace( L'#', L' ' );
	newName.Replace( L'"', L' ' );
	newName.Replace( L'<', L' ' );
	newName.Replace( L'>', L' ' );
	newName.Replace( L'&', L' ' );


	newName.Trim();

	return newName;
}


