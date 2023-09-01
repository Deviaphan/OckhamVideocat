// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AndroidExportDlg.h"
#include <algorithm>
#include "../CheckBoxList.h"
#include "../CollectionDB.h"
#include "../resource.h"


IMPLEMENT_DYNAMIC(CAndroidExportDlg, CDialog)

CAndroidExportDlg::CAndroidExportDlg( CollectionDB & cdb)
	: CDialog(IDD_ANDROID, nullptr)
	, _cdb( cdb )
	, themeIndex( 0 )
	, exportDir( _T( "" ) )
	, exportType( 0 )
	, filterTree( FALSE )
	, filterRu( TRUE )
	, filterEn( TRUE )
	, filterYear( TRUE )
	, filterGenre( TRUE )
	, filterTag( TRUE )
	, markType( 0 )
	, exportDescr( TRUE )
{

}

CAndroidExportDlg::~CAndroidExportDlg()
{
}

void CAndroidExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EXPORT_PATH, exportDir );
	DDX_Radio( pDX, IDC_EXPORT_ALL, exportType );
	DDX_Check( pDX, IDC_FILTER_TREE, filterTree );
	DDX_Check( pDX, IDC_FILTER_RU, filterRu );
	DDX_Check( pDX, IDC_FILTER_EN, filterEn );
	DDX_Check( pDX, IDC_FILTER_YEAR, filterYear );
	DDX_Check( pDX, IDC_FILTER_GENRE, filterGenre );
	DDX_Check( pDX, IDC_FILTER_TAG, filterTag );
	DDX_Control( pDX, IDC_EXPORT_THEME, _themes );
	DDX_CBIndex( pDX, IDC_EXPORT_THEME, themeIndex );
	DDX_Radio( pDX, IDC_MARK_NOT_VIEWED, markType );
	DDX_Check( pDX, IDC_EXPORT_DESCRIPTION, exportDescr );
}


BEGIN_MESSAGE_MAP(CAndroidExportDlg, CDialog)
	ON_BN_CLICKED( IDC_SELECT_TAGS, &CAndroidExportDlg::OnBnClickedSelectTags )
	ON_BN_CLICKED( IDC_TUTORIAL, &CAndroidExportDlg::OnBnClickedTutorial )
END_MESSAGE_MAP()


BOOL CAndroidExportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for( const CString & theme : themeNames )
	{
		_themes.AddString( theme );
	}
	_themes.SetCurSel( themeIndex );

	return TRUE; 
}


void CAndroidExportDlg::OnOK()
{
	UpdateData();

	if( exportDir.IsEmpty() || !::PathFileExists( exportDir ) )
	{
		AfxMessageBox( L"Папка для сохранения не существует,\r\n либо к ней нет доступа.", MB_OK | MB_TOPMOST );
		return;
	}

	TagManager & tm = _cdb.GetTagManager();

	if( tags.empty() )
	{
		for( auto ii = tm.GetAllTags().begin(); ii != tm.GetAllTags().end(); ++ii )
		{
			tags.emplace_back( std::make_pair( ii->first, (BOOL)(exportType != ExportNotSelectedTag) ) );
		}
	}

	CDialog::OnOK();
}


void CAndroidExportDlg::OnBnClickedSelectTags()
{
	UpdateData();

	CCheckBoxList listDlg;

	TagManager & tm = _cdb.GetTagManager();

	if( tags.empty() )
	{
		for( auto ii = tm.GetAllTags().begin(); ii != tm.GetAllTags().end(); ++ii )
		{
			tags.emplace_back( std::make_pair( ii->first, (BOOL)(exportType == ExportSelectedTag) ) );
		}
	}

	//  В диалоге пункты не сортируются, поэтому нужно отсортировать метки и иметь возможность восстановить прежний порядок

	struct TagData
	{
		CString name;
		BOOL used;
		unsigned oldIndex;
	};
	std::vector<TagData> data( tags.size() );

	for( unsigned i = 0; i < (unsigned)tags.size(); ++i )
	{
		data[i].name = tm.GetTag( tags[i].first ).c_str();
		data[i].used = tags[i].second;
		data[i].oldIndex = i;
	}

	std::sort( data.begin(), data.end(), []( auto & l, auto & r )->bool{return l.name < r.name; } );

	for( const auto & tag : data )
	{
		listDlg.names.push_back( tag.name );
		listDlg.checks.push_back( tag.used );
	}

	if( listDlg.DoModal() != IDOK )
		return;

	for( unsigned i = 0; i < (unsigned)tags.size(); ++i )
	{
		tags[data[i].oldIndex].second = listDlg.checks[i];
	}
}


void CAndroidExportDlg::OnBnClickedTutorial()
{
	::ShellExecute( nullptr, L"open", L"https://youtu.be/m7oeGziZSn8", nullptr, nullptr, SW_SHOWNORMAL );
}
