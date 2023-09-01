// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "DuplicateDlg.h"
#include "Entry.h"
#include "ResString.h"
#include "resource.h"

IMPLEMENT_DYNAMIC( DuplicatesGrid, CMFCPropertyGridCtrl )
BEGIN_MESSAGE_MAP( DuplicatesGrid, CMFCPropertyGridCtrl )
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC( CDuplicateDlg, CDialog )

CDuplicateDlg::CDuplicateDlg( CWnd* pParent /*=nullptr*/ )
	: CDialog( IDD_DUBLICATES, pParent )
	, cdb( nullptr )
{}

CDuplicateDlg::~CDuplicateDlg()
{}

void CDuplicateDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_MFCPROPERTYGRID, _grid );
}

BEGIN_MESSAGE_MAP( CDuplicateDlg, CDialog )
END_MESSAGE_MAP()


BOOL CDuplicateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for( auto & item : data )
	{
		CMFCPropertyGridProperty * entry = new CMFCPropertyGridProperty( ResFormat( L"[%u] %s", (unsigned)item.second.size(), item.second[0]->title.c_str() ) );

		for( const auto & file : item.second )
		{
			Entry * rootEntry = cdb->FindEntry( file->GetRootHandle() );
			const std::wstring path = rootEntry->filePath + file->filePath;

			CMFCPropertyGridFileProperty * subitem = new CMFCPropertyGridFileProperty( ResFormat( L"id: %u", file->filmId ), TRUE, path.c_str() );
			subitem->AllowEdit( FALSE );
			entry->AddSubItem( subitem );
		}

		_grid.AddProperty( entry );
	}

	return TRUE;
}
