// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "EditCard.h"
#include "VideoCatDoc.h"
#include "Entry.h"
#include "ErrorDlg.h"
#include "ResString.h"
#include "resource.h"

IMPLEMENT_DYNAMIC( CEditCard, CDialogEx )

BOOL CEditCard::_autoSave = FALSE;

CEditCard::CEditCard( CWnd* pParent /*=nullptr*/ )
	: CDialogEx( IDD_EDIT_CARD, pParent )
	, _doc( nullptr )
	, _entry( nullptr )
	, _defaultTab( CardTabName::FilmInfo )
	, _objectChanged( false )
{

}

CEditCard::~CEditCard()
{}

void CEditCard::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_AUTOSAVE_CHANGES, _autoSave );
}


BEGIN_MESSAGE_MAP( CEditCard, CDialogEx )
END_MESSAGE_MAP()



BOOL CEditCard::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rect;
	GetDlgItem( IDC_TAB_RECT )->GetWindowRect( rect );
	ScreenToClient( rect );
	_tab.Create( CMFCTabCtrl::STYLE_3D_ROUNDED, rect, this, IDC_TAB_RECT, CMFCTabCtrl::LOCATION_TOP );
	_tab.Init( _doc, &_editEntry, _defaultTab );

	return TRUE;
}


void CEditCard::OnOK()
{
	VC_TRY;

	CWaitCursor pleaseWait;
	UpdateData();

	CDialogEx::OnOK();

	_tab.SaveData();

	const bool hasChanges = AfterEditEqual( *_entry, _editEntry );

	if( hasChanges )
	{
		*_entry = _editEntry;
		_doc->InvalidateDatabase();
		_doc->InvalidateProgressInfo();

		if( _autoSave )
		{
			_doc->SaveCurrentCollection();
		}
	}

	VC_CATCH(...)
	{
		ShowError( ResString( IDS_IRREMOVABLE_EDIT_ERROR), false );
	}
}


void CEditCard::OnCancel()
{
	CDialogEx::OnCancel();
}
