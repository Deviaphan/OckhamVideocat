// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CardTechInfo.h"

#include <filesystem>
#include <sstream>
#include "Entry.h"
#include "ErrorDlg.h"
#include "GlobalSettings.h"
#include "VideoCatDoc.h"
#include "MediaInfo.h"
#include "Traverse/TraverseCollection.h"
#include "ProgressDlg.h"
#include "ResString.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CCardTechInfo, CMFCPropertyPage )

CCardTechInfo::CCardTechInfo()
	: CMFCPropertyPage(IDD_TAB_TECH_INFO)
{

}

CCardTechInfo::~CCardTechInfo()
{
}

void CCardTechInfo::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST, _list );
}


BEGIN_MESSAGE_MAP(CCardTechInfo, CMFCPropertyPage )
	ON_BN_CLICKED( IDC_READ_FILE_INFO, &CCardTechInfo::OnBnReadFileInfo )
	ON_BN_CLICKED( IDC_CLEAR_FILE_INFO, &CCardTechInfo::OnBnClearFileInfo )
	ON_BN_CLICKED( IDC_COPY_INFO, &CCardTechInfo::OnBnClickedCopyInfo )
END_MESSAGE_MAP()




void AddData( CListCtrl &ctrl, int row, int col, const wchar_t * str )
{
	LVITEM lv;
	lv.iItem = row;
	lv.iSubItem = col;
	lv.pszText = (LPWSTR)str;
	lv.mask = LVIF_TEXT;

	if( col == 0 )
		ctrl.InsertItem( &lv );
	else
		ctrl.SetItem( &lv );
}

BOOL CCardTechInfo::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	_list.InsertColumn( 0, L"Название" );
	_list.SetColumnWidth( 0, 200 );
	_list.InsertColumn( 1, L"Значение" );
	_list.SetColumnWidth( 1, 600 );


	return TRUE;
}

void CCardTechInfo::ReinitTab()
{
	__super::ReinitTab();

	RebuildList();
}

bool CCardTechInfo::SaveData()
{
	if( !initialized )
		return false;

	return true;
}

void CCardTechInfo::RebuildList()
{
	if( entry )
	{
		_list.DeleteAllItems();
		int i = 0;
		for( auto ii : entry->techInfo )
		{
			if( i > 0 && ii.second.empty() )
			{
				_list.InsertItem( i, L"" );
				++i;
			}

			AddData( _list, i, 0, ii.first.c_str() );
			AddData( _list, i, 1, ii.second.c_str() );
			++i;
		}
	}
}

void CCardTechInfo::OnBnReadFileInfo()
{
	if( !entry || !doc->Ready() )
		return;

	CollectionDB * db = doc->GetCurrentCollection();
	if( !db )
		return;

	const Entry * root = db->FindEntry( entry->GetRootHandle() );
	if( !root )
		throw (int)1;

	media::SetMediaInfoPath( GetGlobal().GetMediaInfoDll() );

	const std::wstring fullPath = entry->GetMoviePath( root->filePath, *db );

	std::error_code errCode;

	if( !std::filesystem::exists( fullPath, errCode ) )
	{
		CTaskDialog taskDlg(
			fullPath.c_str()
			, L"Файл не найден:"
			, L"Ошибка!"
			, TDCBF_CLOSE_BUTTON
			, 0
		);

		taskDlg.SetDialogWidth( 220 );
		taskDlg.SetMainIcon( TD_ERROR_ICON );
		taskDlg.DoModal();

		return;
	}

	MediaInfoDLL::MediaInfo mi;

	mi.Open( fullPath );
	mi.Option( L"Complete" );
	std::wstring inform = mi.Inform();
	mi.Close();

	media::Info mediaInfo;
	mediaInfo.Parse( inform );

	entry->techInfo.swap( mediaInfo._data );

	RebuildList();
}

void CCardTechInfo::OnBnClearFileInfo()
{
	if( entry )
	{
		entry->techInfo.clear();
		doc->InvalidateDatabase();

		RebuildList();
	}
}


void CCardTechInfo::OnBnClickedCopyInfo()
{
	VC_TRY;

	if( !entry )
		return;

	if( entry->techInfo.empty() )
		return;

	if( !OpenClipboard() )
		return;

	EmptyClipboard();

	std::wstringstream ss;
	for( const auto & line : entry->techInfo )
	{
		if( !line.second.empty() )
			ss << line.first.c_str() << L":\t" << line.second.c_str() << L"\n";
		else
			ss << L"\n" << line.first.c_str() << L"\n";
	}

	const std::wstring data = ss.str();

	HGLOBAL hglbCopy = GlobalAlloc( GMEM_MOVEABLE, (data.size() + 1)* sizeof(wchar_t) );
	if( !hglbCopy )
	{
		CloseClipboard();
		return;
	}

	char * lptstrCopy = (char*)GlobalLock( hglbCopy );
	if( lptstrCopy )
	{
		memcpy( lptstrCopy, data.c_str(), data.size() * sizeof( wchar_t ) );
		lptstrCopy[data.size() * sizeof( wchar_t )] = 0;
		GlobalUnlock( hglbCopy );

		SetClipboardData( CF_UNICODETEXT, hglbCopy );
	}

	CloseClipboard();

	VC_CATCH( ... )
	{}
}

namespace Traverse
{
	struct NoTechInfo : public Base
	{
		std::vector<EntryHandle> films;

		virtual Result Run( Entry & entry ) override
		{
			if( entry.IsFile() && entry.techInfo.empty() )
				films.push_back( entry.thisEntry );

			return CONTINUE;
		}
	};
}

BOOL ReadFileInfo( CollectionDB & cdb, const EntryHandle & parent )
{
	VC_TRY;

	media::SetMediaInfoPath( GetGlobal().GetMediaInfoDll() );

	Traverse::NoTechInfo noInfo;
	cdb.TraverseAll( parent, &noInfo );
	
	MediaInfoDLL::MediaInfo mi;

	ProgressController progress;
	progress.Create( ResString( IDS_READ_FILE_INFO_PROGRESS ) );
	progress.SetProgressCount( (int)noInfo.films.size() );

	unsigned count = 0;

	for( const EntryHandle & h : noInfo.films )
	{
		Entry * entry = cdb.FindEntry( h );

		const Entry * root = cdb.FindEntry( entry->GetRootHandle() );
		if( !root )
			throw (int)1;

		const std::wstring fullPath = entry->GetMoviePath( root->filePath, cdb );

		std::error_code errCode;
		if( std::filesystem::exists( fullPath, errCode ) )
		{
			mi.Open( fullPath );
			mi.Option( L"Complete" );
			std::wstring inform = mi.Inform();
			mi.Close();

			media::Info mediaInfo;
			mediaInfo.Parse( inform );

			entry->techInfo.swap( mediaInfo._data );

			++count;
		}
		
		progress.NextStep();
	}

	return count > 0;

	VC_CATCH( ... )
	{
		static bool ignore = false;

		ignore = ShowError( L"Невозможно получить техническую информацию.", ignore );

		return FALSE;
	}
}
