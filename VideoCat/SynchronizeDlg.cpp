// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "SynchronizeDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CSynchronizeDlg, CDialog)

CSynchronizeDlg::CSynchronizeDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SYNCHRONIZE, pParent)
	, _showNewFilms(TRUE)
	, lockRecurse(false)
	, autofillInfo( FALSE )
	, fillTechinfo( FALSE )
	, generateThumbs( FALSE )
	, _minX(0)
	, _minY(0)
	, _pluginId( 0 )
{

}

CSynchronizeDlg::~CSynchronizeDlg()
{
}

void CSynchronizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_FILE_TREE, _fileTree );
	DDX_Check( pDX, IDC_CHK_AUTOFILL, autofillInfo );
	DDX_Check( pDX, IDC_CHK_TECHINFO, fillTechinfo );
	DDX_Check( pDX, IDC_CHK_THUMBS, generateThumbs );
	DDX_Control( pDX, IDC_FILL_PLUGIN, _pluginList );
	DDX_CBIndex( pDX, IDC_FILL_PLUGIN, _pluginId );
}


BEGIN_MESSAGE_MAP(CSynchronizeDlg, CDialog)
	ON_BN_CLICKED( IDC_NEW_FILMS, &CSynchronizeDlg::OnNewFilms )
	ON_BN_CLICKED( IDC_LOST_FILMS, &CSynchronizeDlg::OnLostFilms )
	ON_NOTIFY( TVN_ITEMCHANGED, IDC_FILE_TREE, &CSynchronizeDlg::OnTvnItemChangedFileTree )
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


BOOL CSynchronizeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_pluginList.AddString( L"Заполняется пользователем" );
	_pluginList.AddString( L"Кинопоиск" );
	_pluginList.SetCurSel( 1 );

	CRect rect;
	GetWindowRect( &rect );
	_minX = rect.Width();
	_minY = rect.Height();

	if( newFilms.empty() )
	{
		_showNewFilms = FALSE;
		GetDlgItem( IDC_NEW_FILMS )->EnableWindow( FALSE );
	}

	if( lostedFilms.empty() )
	{
		GetDlgItem( IDC_LOST_FILMS )->EnableWindow( FALSE );

		if( _showNewFilms == FALSE )
		{
			GetDlgItem( IDOK )->EnableWindow( FALSE );
		}
	}

	_fileTree.ModifyStyle( TVS_CHECKBOXES, 0 );
	_fileTree.ModifyStyle( 0, TVS_CHECKBOXES );

	RebuildTree();

	_tooltip.Create( this, TTS_ALWAYSTIP );

	CMFCToolTipInfo params;
	params.m_bBalloonTooltip = FALSE;
	params.m_bVislManagerTheme = TRUE;
	params.m_bDrawSeparator = TRUE;
	_tooltip.SetParams( &params );

	_tooltip.AddTooltip( GetDlgItem( IDC_NEW_FILMS ), L"Новые фильмы", L"Отметьте фильмы, которые следует добавить в каталог. Пропущенные фильмы можно будет добавить в следующий раз." );
	_tooltip.AddTooltip( GetDlgItem( IDC_LOST_FILMS ), L"Отсутствующие фильмы", L"Отметьте фильмы, которые следует удалить из каталога. Пропущенные фильмы можно будет удалить в следующий раз." );

	return TRUE;
}


void CSynchronizeDlg::OnNewFilms()
{
	_showNewFilms = TRUE;
	RebuildTree();
}


void CSynchronizeDlg::OnLostFilms()
{
	_showNewFilms = FALSE;
	RebuildTree();
}

void CSynchronizeDlg::RebuildTree()
{
	_fileTree.DeleteAllItems();
	_pathToItem.clear();

	lockRecurse = true;

	const std::vector<SyncData> * data = _showNewFilms ? &newFilms : &lostedFilms;

	const HTREEITEM rootItem = _fileTree.GetRootItem();

	for( unsigned i = 0; i < (unsigned)data->size(); ++i )
	{
		const SyncData & sd = (*data)[i];

		HTREEITEM parentItem = rootItem;
		HTREEITEM currentItem = nullptr;
		
		int pos = 0;
		CString token;
		
		CString sumPath;

		while( !(token = sd.path.Tokenize( L"\\", pos )).IsEmpty() )
		{
			sumPath += token;
			if( sumPath != sd.path )
				sumPath += L"\\";

			auto ii = _pathToItem.find( sumPath );
			if( ii == _pathToItem.end() )
			{
				currentItem = _fileTree.InsertItem( token, parentItem );
				_pathToItem[sumPath] = currentItem;

				if( sumPath == sd.path )
				{
					_fileTree.SetItemData( currentItem, (DWORD_PTR)(i+1) );
					_fileTree.SetCheck( currentItem, sd.allow );
				}

				parentItem = currentItem;
			}
			else
			{
				// родительские папки уже могли быть добавлены в процессе добавления фильмов
				parentItem = ii->second;

				if( sumPath == sd.path )
				{
					_fileTree.SetItemData( ii->second, (DWORD_PTR)(i+1) );
					_fileTree.SetCheck( ii->second, sd.allow );
				}
			}
		}
	}

	lockRecurse = false;

	SortItem( TVI_ROOT );
}

void CSynchronizeDlg::SortItem( HTREEITEM item )
{
	if( item )
	{
		if( item == TVI_ROOT || _fileTree.ItemHasChildren( item ) )
		{
			HTREEITEM child = _fileTree.GetChildItem( item );

			while( child )
			{
				SortItem( child );
				child = _fileTree.GetNextItem( child, TVGN_NEXT );
			}

			_fileTree.SortChildren( item );
		}
	}
}

void CSynchronizeDlg::OnTvnItemChangedFileTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	NMTVITEMCHANGE *pNMTVItemChange = reinterpret_cast<NMTVITEMCHANGE*>(pNMHDR);
	*pResult = 0;

	if( lockRecurse )
		return;

	const BOOL checkedOld = ((BOOL)(pNMTVItemChange->uStateOld >> 12) - 1);
	const BOOL checkedNew = ((BOOL)(pNMTVItemChange->uStateNew >> 12) - 1);
	if( checkedOld != checkedNew )
	{
		UpdateData();
		ChangeStates( pNMTVItemChange->hItem, pNMTVItemChange->hItem, checkedNew );

		if( checkedNew != FALSE )
		{
			lockRecurse = true;
			SetParentStates( pNMTVItemChange->hItem );
			lockRecurse = false;
		}
	}
}

void CSynchronizeDlg::ChangeStates( HTREEITEM startItem, HTREEITEM currentItem, BOOL state )
{
	if( lockRecurse )
		return;

	while( currentItem )
	{
		// Обрабатываем текущий узел
		_fileTree.SetCheck( currentItem, state );

		unsigned index = (unsigned)_fileTree.GetItemData( currentItem );
		if( index > 0 )
		{
			index -= 1;
			std::vector<SyncData> * data = _showNewFilms ? &newFilms : &lostedFilms;
			if( data->size() > index )
			{
				(*data)[index].allow = state;
			}
		}

		if( _fileTree.ItemHasChildren( currentItem ) )
		{
			// Выполняем для дочерних узлов
			HTREEITEM childItem = _fileTree.GetChildItem( currentItem );
			ChangeStates( startItem, childItem, state );
		}

		// Переходим к следующему узлу на этом же уровне
		if( startItem != currentItem )
			currentItem = _fileTree.GetNextSiblingItem( currentItem );
		else
			return;
	}
}

void CSynchronizeDlg::SetParentStates( HTREEITEM currentItem )
{
	HTREEITEM parentItem = _fileTree.GetParentItem( currentItem );
	if( !parentItem )
		return;

	unsigned index = (unsigned)_fileTree.GetItemData( parentItem );
	if( index > 0 )
	{
		index -= 1;
		std::vector<SyncData> * data = _showNewFilms ? &newFilms : &lostedFilms;
		if( data->size() > index )
		{
			_fileTree.SetCheck( currentItem, TRUE );
			(*data)[index].allow = TRUE;
		}
	}

	SetParentStates( parentItem );
}


BOOL CSynchronizeDlg::PreTranslateMessage( MSG* pMsg )
{
	_tooltip.RelayEvent( pMsg );

	return CDialog::PreTranslateMessage( pMsg );
}


void CSynchronizeDlg::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	if( _minX > 0 )
	{
		lpMMI->ptMinTrackSize.x = _minX;
		lpMMI->ptMinTrackSize.y = _minY;
	}

	CDialog::OnGetMinMaxInfo( lpMMI );
}

PluginID CSynchronizeDlg::GetAutofillPlugin()
{
	switch( _pluginId )
	{
		case 0:
			return (PluginID)PluginManual;

		default:
			return (PluginID)PluginKinopoisk;
	}
}


void CSynchronizeDlg::OnOK()
{
	UpdateData();

	CDialog::OnOK();
}
