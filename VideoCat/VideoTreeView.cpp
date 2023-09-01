// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "VideoTreeView.h"

#include <algorithm>

#include "ContextMenu.h"
#include "EraseDlg.h"
#include "ErrorDlg.h"
#include "FileFormatSettings.h"
#include "GenreFilter.h"
#include "GlobalSettings.h"
#include "IconManager.h"
#include "Kinopoisk/Kinopoisk.h"
#include "Kinopoisk/LoadFilmInfo.h"
#include "PosterDownloader.h"
#include "ResString.h"
#include "Traverse/AllGenresTags.h"
#include "TestPassword.h"
#include "ProgressDlg.h"
#include "IconManager.h"
#include "CommandManager.h"
#include "Commands/CommandExecute.h"
#include "OnscreenKeyboardDlg.h"
#include "SetWin10Theme.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const DWORD FIND_BY_NAME_ID = 6543;

template< class T_Functor >
bool ForEach( CTreeCtrl & tree, HTREEITEM root, T_Functor action )
{
	while( nullptr != root )
	{
		if( (*action)(tree, root) )
			return true;

		if( tree.ItemHasChildren( root ) )
		{
			// Выполняем для дочерних узлов
			if( ForEach( tree, tree.GetNextItem( root, TVGN_CHILD ), action ) )
				return true;
		}

		// Переходим к следующему узлу на этом же уровне
		root = tree.GetNextItem( root, TVGN_NEXT );
	}

	return false;
}

IMPLEMENT_DYNCREATE( CVideoTreeView, CFormView )

CVideoTreeView::CVideoTreeView()
	: CFormView( CVideoTreeView::IDD )
	, _nameFilter( _T( "" ) )
	, _activeMenuItem( nullptr )
	, _isRoot( false )
	, _info( _T( "" ) )
	, _lastUsedDB( -1 )
	, _bgColor( RGB( 255, 255, 255 ) )
	, _textColor( RGB( 0, 0, 0 ) )
{

}

CVideoTreeView::~CVideoTreeView()
{}

void CVideoTreeView::UpdateView()
{
	_tree.Invalidate(FALSE);
	//_tree.UpdateWindow();
}

void CVideoTreeView::SetDarkTheme( bool dark )
{
	SetWin10Theme( GetSafeHwnd(), dark );
	SetWin10Theme( _collectionList.GetSafeHwnd(), dark );
	SetWin10Theme( _rootList.GetSafeHwnd(), dark );
	SetWin10Theme( _tree.GetSafeHwnd(), dark );
}

void CVideoTreeView::DoDataExchange( CDataExchange* pDX )
{
	__super::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BTN_HELP, _btnHelp );
	DDX_Control( pDX, IDC_BTN_KEYBOARD, _btnKeyboard );
	DDX_Control( pDX, IDC_BTN_MAIN, _btnMainMenu );
	DDX_Control( pDX, IDC_BTN_OPEN_FILTER, _btnOpenFilter );
	DDX_Control( pDX, IDC_BTN_VIEW_SETTINGS, _btnViewSettings );
	DDX_Control( pDX, IDC_DATABASE_LIST, _collectionList );
	DDX_Control( pDX, IDC_FIND_BY_NAME, _findByName );
	DDX_Control( pDX, IDC_INFO, _infoBox );
	DDX_Control( pDX, IDC_INFO_VALUES, _infoBoxValues );
	DDX_Control( pDX, IDC_ROOT_LIST, _rootList );
	DDX_Control( pDX, IDC_ROOT_TREE, _tree );
	DDX_Text( pDX, IDC_FIND_BY_NAME, _nameFilter );
}

BEGIN_MESSAGE_MAP( CVideoTreeView, CFormView )
	ON_BN_CLICKED( IDC_BTN_HELP, &CVideoTreeView::OnBnClickedHelp )
	ON_BN_CLICKED( IDC_BTN_KEYBOARD, &CVideoTreeView::OnBtnKeyboard )
	ON_BN_CLICKED( IDC_BTN_MAIN, &CVideoTreeView::OnBnClickedMainMenu )
	ON_BN_CLICKED( IDC_BTN_OPEN_FILTER, &CVideoTreeView::OnBnClickedOpenFilter )
	ON_BN_CLICKED( IDC_BTN_VIEW_SETTINGS, &CVideoTreeView::OnBnClickedViewSettings )
	ON_EN_CHANGE( IDC_FIND_BY_NAME, &CVideoTreeView::OnChangeFindByName )
	ON_LBN_SELCHANGE( IDC_DATABASE_LIST, &CVideoTreeView::OnLbnSelchangeDatabaseList )
	ON_LBN_SELCHANGE( IDC_ROOT_LIST, &CVideoTreeView::OnLbnSelchangeRootList )
	ON_NOTIFY( TVN_SELCHANGED, IDC_ROOT_TREE, &CVideoTreeView::OnChangedTreeItem )
	ON_WM_CONTEXTMENU()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

#ifdef _DEBUG
void CVideoTreeView::AssertValid() const
{
	__super::AssertValid();
}

#ifndef _WIN32_WCE
void CVideoTreeView::Dump( CDumpContext& dc ) const
{
	__super::Dump( dc );
}
#endif
#endif

void CVideoTreeView::UpdateCollectionList()
{
	VC_TRY;

	_lastUsedDB = -1;

	_collectionList.ResetContent();

	int currentIndex = -1;
	CollectionDB::DBID collectionId;

	CVideoCatDoc * doc = GetDocument();

	CollectionDB * collection = doc->GetCurrentCollection();
	if( collection )
		collectionId = collection->GetId();

	for( unsigned i = 0; i < doc->GetNumCollections(); ++i )
	{
		const CVideoCatDoc::CollectionInfo * info = doc->GetCollectionInfo( i );
		if( info )
		{
			_collectionList.AddString( info->name );

			if( collectionId == info->id )
				currentIndex = i;
		}
		else
		{
			_collectionList.AddString( L"?WTF?" );
		}
	}

	_collectionList.SetCurSel( currentIndex );
	_lastUsedDB = currentIndex;

	RebuildTree();

	VC_CATCH_ALL;
}

void CVideoTreeView::OnChangedTreeItem( NMHDR *pNMHDR, LRESULT *pResult )
{
	VC_TRY;
	CWaitCursor waiting;

	*pResult = 0;

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	Entry * fi = nullptr;
	{
		auto ii = _treeMap.find( pNMTreeView->itemNew.hItem );
		if( ii != _treeMap.end() )
			fi = ii->second;		
	}

	Entry * oldFi = _activeMenuItem;
	_activeMenuItem = fi;

	if( _tree.GetParentItem( pNMTreeView->itemNew.hItem ) == nullptr )
		_isRoot = true;
	else
		_isRoot = false;
	
	//_nameFilter.Empty();
	//UpdateData( FALSE );

	CVideoCatDoc * doc = GetDocument();
	
	doc->RebuildFilteredFiles( fi, oldFi );

	VC_CATCH_ALL;
}


void CVideoTreeView::OnChangeFindByName()
{
	VC_TRY;
	KillTimer( FIND_BY_NAME_ID );
	UpdateData();

	if( _nameFilter.GetLength() >= 3 )
	{
		SetTimer( FIND_BY_NAME_ID, 500, nullptr );
	}
	VC_CATCH_ALL;
}

void CVideoTreeView::OnBnClickedOpenFilter()
{
	VC_TRY;
	CWaitCursor waiting;

	CollectionDB * cdb = GetDocument()->GetCurrentCollection();
	if( !cdb )
		return;

	Traverse::AllGenresTags allGenres;
	cdb->ForEach( &allGenres );

	CGenreFilter filter;
	filter.tagManager = &cdb->GetTagManager();
	filter.inputGenres.swap( allGenres.numGenres );
	filter.inputTags.swap( allGenres.numTags );

	CRect btnRect;
	_btnOpenFilter.GetWindowRect( btnRect );

	filter.position.SetPoint( btnRect.left, btnRect.bottom );

	if( IDOK != filter.DoModal() )
		return;

	if( filter.filterType == FilterType::ByGenre )
		GetDocument()->RebuildFilteredFiles( filter.outputGenres, filter.strictGenre );
	else
		GetDocument()->RebuildFilteredFiles( filter.selectedTags, filter.strictGenre );

	if( filter.extraFiltering )
		GetDocument()->RebuildFilteredFiles( _nameFilter, TRUE );

	_nameFilter.Empty();
	UpdateData( FALSE );

	VC_CATCH_ALL;
}

void CVideoTreeView::OnBnClickedMainMenu()
{
	VC_TRY;

	CRect btnRect;
	_btnMainMenu.GetWindowRect( btnRect );

	adv_mfc::AdvancedMenu menu;
	menu.CreatePopupMenu();
	
	CreateMainMenu( menu, GetDocument() );

	menu.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, btnRect.left, btnRect.top, this, nullptr );

	VC_CATCH_ALL;
}

#ifdef _DEBUG
CVideoCatDoc* CVideoTreeView::GetDocument() const
{
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CVideoCatDoc ) ) );
	return (CVideoCatDoc*)m_pDocument;
}
#endif //_DEBUG


void CVideoTreeView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	_btnOpenFilter.m_bTransparent = TRUE;
	_btnMainMenu.m_bTransparent = TRUE;
	_btnHelp.m_bTransparent = TRUE;
	_btnViewSettings.m_bTransparent = TRUE;
	_btnKeyboard.m_bTransparent = TRUE;

	_collectionList.SetIcon( AfxGetApp()->LoadIcon( IDI_DATABASE ) );
	_rootList.SetIcon( AfxGetApp()->LoadIcon( IDI_FOLDER_ICON ) );
}

void CVideoTreeView::InitializeTree()
{
	VC_TRY;

	_tree.Initialize();

	UpdateCollectionList();

	Edit_SetCueBannerText( _findByName.GetSafeHwnd(), ResString( IDS_SEARCH_TAG ).GetString() );

	CRect r;
	GetClientRect( r );
	OnSize( 0, r.Width(), r.Height() );

	_infoBox.SetWindowText( L"Фильмы:\r\nПерсоны:\r\nВсего:" );

	VC_CATCH_ALL;
}

void RebuildTree( CTreeCtrl & tree, std::map<HTREEITEM, Entry* > & treeMap, HTREEITEM parent, const Entry & parentFile, CollectionDB & cdb )
{
	for( const auto & child : parentFile.childs )
	{
		Entry * item = cdb.FindEntry( child );
		if( !item )
			continue;

		if( !item->IsFolder() )
			continue;

		if( item->IsPrivate() && cdb.PrivateLocked() )
			continue;

		HTREEITEM newItem = tree.InsertItem( item->title.c_str(), parent );
		treeMap[newItem] = item;


		RebuildTree( tree, treeMap, newItem, *item, cdb );
	}
}

void CVideoTreeView::ClearTree()
{
	_tree.DeleteAllItems();
}

class RedrawGuard
{
public:
	explicit RedrawGuard( CTreeCtrl & tree )
		: _tree( &tree )
	{
		_tree->SetRedraw( FALSE );
	}

	~RedrawGuard()
	{
		if( _tree )
			_tree->SetRedraw( TRUE );
	}

	void Unlock()
	{
		_tree->SetRedraw( TRUE );
		_tree = nullptr;
	}

private:
	CTreeCtrl * _tree;
};

Entry* CVideoTreeView::GetTreeEntry( HTREEITEM hitem )
{
	auto ii = _treeMap.find( hitem );
	if( ii == _treeMap.end() )
		return nullptr;

	return ii->second;
}

void CVideoTreeView::RebuildTree()
{
	if( GetGlobal().theme )
	{
		_tree.SetItemHeight( GetGlobal().theme->tree.fontSize + 4 );

		COLORREF bgColor = GetGlobal().theme->bgTopColor;
		COLORREF textColor = GetGlobal().theme->tree.textColor;

		if( _ctrlBrush.m_hObject )
		{
			_ctrlBrush.DeleteObject();
		}

		_ctrlBrush.CreateSolidBrush( bgColor );
		_bgColor = bgColor;
		_textColor = textColor;
	}

	_activeMenuItem = nullptr;
	_isRoot = false;

	CVideoCatDoc * doc = GetDocument();

	RedrawGuard redrawGuard( _tree );

	_treeMap.clear();
	_tree.DeleteAllItems();
	_rootList.ResetContent();

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	for( unsigned ri = 0; ri < cdb->GetNumRoots(); ++ri )
	{
		EntryHandle rootHandle = cdb->GetRoot( ri );
		Entry * rootEntry = cdb->FindEntry( rootHandle );
		if( !rootEntry )
			continue;

		CString displayName;
		if( rootEntry->thisEntry.IsVirtual() )
			displayName = rootEntry->title.c_str(); // для виртуальных нет пути, только имя
		else
			displayName = ResFormat( L"%s   <%s>", rootEntry->title.c_str(), rootEntry->filePath.c_str() );

		int rootIndex = _rootList.AddString( displayName );
		_rootList.SetItemDataPtr( rootIndex, rootEntry );

		HTREEITEM parent = _tree.InsertItem( displayName );
		_treeMap[parent] = rootEntry;

		::RebuildTree( _tree, _treeMap, parent, *rootEntry, *cdb );
	}

	_rootList.SetCurSel( 0 );

	struct ExpandAll
	{
		CVideoTreeView * tv;

		bool operator()( CTreeCtrl & tree, HTREEITEM item )
		{
			Entry * entry = tv->GetTreeEntry( item );
			if( entry && entry->thisEntry.IsPerson() )
			{
				return false;
			}

			tree.Expand( item, TVE_EXPAND );
			return false;
		}
	};

	ExpandAll expandAll;
	expandAll.tv = this;
	ForEach( _tree, _tree.GetRootItem(), &expandAll );

	redrawGuard.Unlock();

	_tree.SelectItem( _tree.GetRootItem() );
	if( cdb->GetNumRoots() == 0 )
	{
		doc->RebuildFilteredFiles( (Entry *)nullptr );
	}

	const unsigned numFilms = cdb->GetNumFilms();
	const unsigned numPersons = cdb->GetNumPersones();
	const unsigned numTotal = cdb->GetNumEntries() + numPersons;

	_info.Format( L"%u\r\n%u\r\n%u", numFilms, numPersons, numTotal );
	_infoBoxValues.SetWindowText( _info );
}

void CVideoTreeView::SelectItem( Entry * item )
{
	if( !item )
	{
		_tree.SelectItem( (HTREEITEM)nullptr );
		return;
	}
	
	struct FindNodeByData
	{
		const std::map<HTREEITEM, Entry* > & _treeMap;

		FindNodeByData( const std::map<HTREEITEM, Entry* > & tm, void * ptr )
			: _treeMap( tm )
			, searchThis( ptr )
			, result( nullptr )
		{}

		bool operator()( CTreeCtrl &, HTREEITEM item )
		{
			auto ii = _treeMap.find( item );

			if( ii != _treeMap.end() && ii->second == searchThis )
			{
				result = item;
				return true;
			}
			return false;
		}

		void * searchThis;
		HTREEITEM result;
	};

	CWaitCursor wait;

	FindNodeByData func( _treeMap, item );
	ForEach( _tree, _tree.GetRootItem(), &func );

	if( func.result )
	{
		_tree.SelectItem( func.result );
		_tree.Expand( func.result, TVE_EXPAND );
		UpdateView();
	}
}

Entry * CVideoTreeView::GetCurrentItem()
{
	return _activeMenuItem;
}

Entry * CVideoTreeView::GetParentItem( Entry * item )
{
	if( item->parentEntry == EntryHandle::EMPTY )
		return nullptr;

	CVideoCatDoc * doc = GetDocument();
	if( !doc )
		return nullptr;

	return doc->GetCurrentCollection()->FindEntry( item->parentEntry );
}

void CVideoTreeView::GoBack()
{
	HTREEITEM selectedItem = _tree.GetSelectedItem();
	HTREEITEM parentItem = _tree.GetParentItem( selectedItem );

	if( !selectedItem )
	{
		parentItem = _tree.GetRootItem();
	}

	if( parentItem )
	{
		auto ii = _treeMap.find( parentItem );

		Entry * info = (ii != _treeMap.end()) ? ii->second : nullptr;
		if( info )
			SelectItem( info );
	}
}


void CVideoTreeView::OnTimer( UINT_PTR nIDEvent )
{
	if( nIDEvent == FIND_BY_NAME_ID )
	{
		VC_TRY;

		KillTimer( FIND_BY_NAME_ID );

		CVideoCatDoc * doc = GetDocument();
		if( doc )
			doc->RebuildFilteredFiles( _nameFilter, FALSE );

		VC_CATCH_ALL;
	}

	CFormView::OnTimer( nIDEvent );
}

void CVideoTreeView::OnContextMenu( CWnd* pWnd, CPoint point )
{
	if( pWnd == &_collectionList )
	{
		OnCollectionMenu( pWnd, point );
		return;
	}
	else if( pWnd == &_rootList )
	{
		OnRootMenu( pWnd, point );
		return;
	}

	POINT mPos;
	GetCursorPos( &mPos );
	_tree.ScreenToClient( &mPos );
	HTREEITEM item = _tree.HitTest( mPos );

	if( item )
	{
		auto ii = _treeMap.find( item );
		_activeMenuItem = (ii != _treeMap.end()) ? ii->second : nullptr;
		if( !_activeMenuItem )
			return;

		if( _tree.GetParentItem( item ) == nullptr )
		{
			_isRoot = true;
		}

		adv_mfc::AdvancedMenu menu;
		menu.CreatePopupMenu();

		CreateFolderMenu( menu, *_activeMenuItem, true );

		menu.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, nullptr );
	}
	else
	{
		_activeMenuItem = nullptr;
		_isRoot = false;
	}
}

void CVideoTreeView::OnCollectionMenu( CWnd * pWnd, CPoint point )
{
	adv_mfc::AdvancedMenu menu;
	menu.CreatePopupMenu();

	menu.AddItem( (UINT_PTR)CommandID::ManageCollections, new adv_mfc::AdvancedTextItem( CommandExecute::Instance().GetCommand( CommandID::ManageCollections ).commandTitle, CommandExecute::Instance().GetCommand( CommandID::ManageCollections ).iconID ) );

	menu.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, nullptr );
}

void CVideoTreeView::OnRootMenu( CWnd * pWnd, CPoint point )
{
	adv_mfc::AdvancedMenu menu;
	menu.CreatePopupMenu();

	menu.AddItem( (UINT_PTR)CommandID::ManageRoots, new adv_mfc::AdvancedTextItem( CommandExecute::Instance().GetCommand( CommandID::ManageRoots ).commandTitle, CommandExecute::Instance().GetCommand( CommandID::ManageRoots ).iconID ) );

	menu.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, nullptr );
}


BOOL CVideoTreeView::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if( HIWORD( wParam ) == 0 && (lParam == 0) )
	{
		if( AllCommandsDisabled() )
			return TRUE;

		const DWORD menuId = LOWORD( wParam );
		
		CVideoCatDoc* doc = GetDocument();
		if( !doc )
			return TRUE;

		CommandInfo info;
		info.doc = doc;
		info.entry = _activeMenuItem;

		CommandExecute::Instance().Process( (CommandID)menuId, &info );

		return TRUE;
	}

	return CFormView::OnCommand( wParam, lParam );
}

void CVideoTreeView::OnSize( UINT nType, int cx, int cy )
{
	CFormView::OnSize( nType, cx, cy );
}


BOOL CVideoTreeView::OnEraseBkgnd( CDC* pDC )
{
	if( GetGlobal().theme )
	{
		CRect r;
		GetClientRect( r );
		pDC->FillSolidRect( r, GetGlobal().theme->bgTopColor );
	}
	return TRUE;
}

void CVideoTreeView::OnBnClickedHelp()
{
	CommandInfo info; // передаю пустышку
	CommandExecute::Instance().Process( CommandID::OpenForum, &info );
}


void CVideoTreeView::OnLbnSelchangeDatabaseList()
{
	CWaitCursor wait;


	const int index = _collectionList.GetCurSel();
	if( index < 0 )
		return;

	if( _lastUsedDB == index )
		return;

	_lastUsedDB = index;

	CVideoCatDoc * doc = GetDocument();
	const CVideoCatDoc::CollectionInfo * info = doc->GetCollectionInfo( index );
	if( info )
	{
		doc->ReloadCollection( info->id );
	}

	RebuildTree();	// TODO: Add your control notification handler code here
}

void CVideoTreeView::OnLbnSelchangeRootList()
{
	int index = _rootList.GetCurSel();
	if( index < 0 )
		return;

	Entry * rootEntry = (Entry*)_rootList.GetItemDataPtr( index );
	if( rootEntry )
		SelectItem( rootEntry );	
}


void CVideoTreeView::OnBnClickedViewSettings()
{
	VC_TRY;

	CVideoCatDoc* doc = GetDocument();
	if( !doc )
		return;

	CRect btnRect;
	_btnViewSettings.GetWindowRect( btnRect );

	adv_mfc::AdvancedMenu menu;
	menu.CreatePopupMenu();

	{
		adv_mfc::AdvancedMenu * sortMenu = new adv_mfc::AdvancedMenu;
		sortMenu->CreatePopupMenu();

#define ADD_ITEM( ID ) AddItem( (UINT_PTR)(ID), new adv_mfc::AdvancedTextItem( CommandExecute::Instance().GetCommand( ID ).commandTitle, CommandExecute::Instance().GetCommand( ID ).iconID ) );

		sortMenu->ADD_ITEM( CommandID::SortGlobalByYearDown );
		sortMenu->ADD_ITEM( CommandID::SortGlobalByYearUp );
		sortMenu->ADD_ITEM( CommandID::SortGlobalByRussianName );
		sortMenu->ADD_ITEM( CommandID::SortGlobalByOriginalName );
		sortMenu->ADD_ITEM( CommandID::SortGlobalByAddDate );
		sortMenu->ADD_ITEM( CommandID::SortGlobalByFilenameUp );
		sortMenu->ADD_ITEM( CommandID::SortGlobalByFilenameDown );

		static const CommandID convertSort[] = {
			CommandID::SortGlobalByYearDown, // SortYearDown = 0
			CommandID::SortGlobalByYearUp, // SortYearUp
			CommandID::SortGlobalByRussianName, // SortRu
			CommandID::SortGlobalByOriginalName, // SortEn
			CommandID::SortGlobalByAddDate, // SortDate
			CommandID::SortGlobalByFilenameUp, // SortFileUp
			CommandID::SortGlobalByFilenameDown // SortFileDown
		};

		const sort_predicates::SortType predicate = sort_predicates::GetPredicateId( doc->GetCurrentPredicate() );
		sortMenu->SetDefaultItem( (UINT)convertSort[predicate] );

		menu.AddItem( sortMenu, new adv_mfc::AdvancedTextItem( ResString( IDS_SORT ), IconManager::Sorting ) );
	}

	if( doc->HasRecentlyMovies() )
	{
		menu.ADD_ITEM( CommandID::ShowLastViewed );
	}

	menu.ADD_ITEM( CommandID::ShowNewFilms );

	menu.ADD_ITEM( CommandID::ShowRandomFilms );

	menu.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, btnRect.left, btnRect.top, this, nullptr );

	VC_CATCH_ALL;
}





HBRUSH CVideoTreeView::OnCtlColor( CDC * pDC, CWnd * pWnd, UINT nCtlColor )
{
	if( !_ctrlBrush.m_hObject )
	{
		return CFormView::OnCtlColor( pDC, pWnd, nCtlColor );
	}

	pDC->SetBkColor( _bgColor );
	pDC->SetTextColor( _textColor );

	return _ctrlBrush;
}



void CVideoTreeView::OnBtnKeyboard()
{
	CVideoCatDoc * doc = GetDocument();
	if( !doc )
		return;

	CommandInfo info;
	info.doc = doc;

	CommandExecute::Instance().Process( CommandID::OskFindByName, &info );
}
