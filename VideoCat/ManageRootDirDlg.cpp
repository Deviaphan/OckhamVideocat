// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ManageRootDirDlg.h"
#include "ResString.h"
#include "CreateRootDlg.h"
#include "VideoCatDoc.h"
#include "ExcludedPathDlg.h"
#include "DeleteRootDlg.h"
#include "RootRenameDlg.h"
#include "ContextMenu.h"
#include "EditRealRootDlg.h"
#include "VideoTreeView.h"
#include "Commands/CommandExecute.h"
#include "Commands/CommandInfo.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(ManageRootDirDlg, CDialog)

ManageRootDirDlg::ManageRootDirDlg( CVideoCatDoc * doc, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MANAGE_ROOT_DIRECTORY, pParent)
	, _doc( doc )
	, _comment( L"" )
{

}

ManageRootDirDlg::~ManageRootDirDlg()
{
}

void ManageRootDirDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_COMMENT_BOX, _comment );
	DDX_Control( pDX, IDC_COMMENT_BOX, _commentBox );
	DDX_Control( pDX, IDC_CREATE_ROOT_BTN, _btnCreateRoot );
	DDX_Control( pDX, IDC_DELETE_ROOT_BTN, _btnDeleteRoot );
	DDX_Control( pDX, IDC_EXCLUDE_FOLDER_BTN, _btnExcludeDir );
	DDX_Control( pDX, IDC_IMPORT_EXPORT, _btnImportExport );
	DDX_Control( pDX, IDC_RENAME_ROOT_BTN, _btnRenameRoot );
	DDX_Control( pDX, IDC_ROOT_LIST, _rootList );
}


BEGIN_MESSAGE_MAP(ManageRootDirDlg, CDialog)
	ON_BN_CLICKED( IDC_CREATE_ROOT_BTN, &ManageRootDirDlg::OnBnClickedCreateRootBtn )
	ON_BN_CLICKED( IDC_DELETE_ROOT_BTN, &ManageRootDirDlg::OnBnClickedDeleteRootBtn )
	ON_BN_CLICKED( IDC_EDIT_ROOT_BTN, &ManageRootDirDlg::OnBnClickedEditRootBtn )
	ON_BN_CLICKED( IDC_EXCLUDE_FOLDER_BTN, &ManageRootDirDlg::OnBnClickedExcludeFolderBtn )
	ON_BN_CLICKED( IDC_IMPORT_EXPORT, &ManageRootDirDlg::OnImportExport )
	ON_LBN_SELCHANGE( IDC_ROOT_LIST, &ManageRootDirDlg::OnLbnSelchangeRootList )
END_MESSAGE_MAP()

void ManageRootDirDlg::OnOK()
{
	// CDialog::OnOK();
}


BOOL ManageRootDirDlg::OnInitDialog()
{
	_comment = ResString( IDS_COMMENT_ROOT );

	CDialog::OnInitDialog();

	const CString & collectionName =  GetDocument()->GetCurrentCollection()->GetName();

	CString title;
	GetWindowText( title );
	title += L" • ";
	title += collectionName;
	SetWindowText( title );

	UpdateRootList();

	if( _menuImportExport.CreatePopupMenu() )
	{
		CreateImportExportMenu( _menuImportExport );

		_btnImportExport.m_hMenu = _menuImportExport.GetSafeHmenu();
	}

	return TRUE;
}

void ManageRootDirDlg::UpdateRootList()
{
	VC_TRY;

	_rootList.ResetContent();

	CollectionDB * collection = GetDocument()->GetCurrentCollection();
	if( !collection )
		return;

	const int numRoots = collection->GetNumRoots();
	for( int index = 0; index < numRoots; ++index )
	{
		EntryHandle rootHandle = collection->GetRoot( index );
		Entry * root = collection->FindEntry( rootHandle );
		if( root )
		{
			std::wstring title( root->title);
			if( rootHandle.IsVirtual() )
				title += L"  [виртуальная папка]";

			_rootList.AddString( title.c_str() );
		}
		else
			_rootList.AddString( L"" ); // просто пустую строку вставляю, чтобы индексацию не сбивать
	}

	VC_CATCH_ALL;
}


void ManageRootDirDlg::OnLbnSelchangeRootList()
{
	const int index = _rootList.GetCurSel();

	_btnRenameRoot.EnableWindow( index >= 0 );
	_btnExcludeDir.EnableWindow( index >= 0 );
	_btnDeleteRoot.EnableWindow( index >= 0 );

	if( index < 0 )
		return;
}


void ManageRootDirDlg::OnBnClickedCreateRootBtn()
{
	VC_TRY;

	CollectionDB * cdb = GetDocument()->GetCurrentCollection();
	if( !cdb )
		return;

	CreateRootDlg rootDlg;
	if( IDOK != rootDlg.DoModal() )
		return;
	
	EntryHandle rootHandle;

	if( rootDlg.isRealRoot )
		rootHandle = cdb->InsertRoot( rootDlg.title, rootDlg.path );
	else
		rootHandle = cdb->InsertVirtualRoot( rootDlg.title );

	if( rootHandle == EntryHandle::EMPTY )
		return;

	CollectionDB::RootInfo & rootInfo = cdb->CreateRootInfo( rootHandle );
	if( rootDlg.isRealRoot )
	{
		rootInfo.searchHidden = rootDlg.useHiddenFolder;
		rootInfo.innerFilesMode = rootDlg.useFileInFolder;
	}

	GetDocument()->InvalidateDatabase();

	UpdateRootList();

	if( !rootDlg.isRealRoot )
		return;
	
	if( IDYES != AfxMessageBox( L"Выполнить поиск фильмов для добавленной корневой директории?", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST ) )
		return;

	CommandInfo info;
	info.doc = GetDocument();
	info.entry = cdb->FindEntry( rootHandle );

	CommandExecute::Instance().Process( CommandID::SynchronizeFolder, &info );

	VC_CATCH_ALL;
}


void ManageRootDirDlg::OnBnClickedEditRootBtn()
{
	VC_TRY;

	const int index = _rootList.GetCurSel();
	if( index < 0 )
		return;

	CollectionDB * collection = GetDocument()->GetCurrentCollection();
	if( !collection )
		return;

	EntryHandle rootHandle = collection->GetRoot( index );
	Entry * root = collection->FindEntry( rootHandle );
	if( !root )
		return;

	if( rootHandle.IsVirtual() )
	{
		// виртуальную можно только переименовать
		RootRenameDlg dlg;
		dlg.title = root->title.c_str();

		if( IDOK != dlg.DoModal() )
			return;

		root->title = dlg.title.GetString();
	}
	else
	{
		auto & rootInfo = collection->GetRootInfo( rootHandle );

		EditRealRootDlg dlg;

		dlg.title = root->title.c_str();
		dlg.path = root->filePath.c_str();
		dlg.useFileInFolder = rootInfo.innerFilesMode;

		const INT_PTR result = dlg.DoModal();

		if( result != IDOK )
			return;

		root->title = dlg.title.GetString();
		root->filePath = dlg.path.GetString();
		rootInfo.innerFilesMode = dlg.useFileInFolder;

		if( IDYES == AfxMessageBox( L"Выполнить поиск фильмов для отредактированной корневой директории?", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST ) )
		{
			CommandInfo info;
			info.doc = GetDocument();
			info.entry = collection->FindEntry( rootHandle );

			CommandExecute::Instance().Process( CommandID::SynchronizeFolder, &info );
		}
	}

	GetDocument()->InvalidateDatabase();

	UpdateRootList();

	GetDocument()->GetVideoTreeView()->RebuildTree();

	VC_CATCH_ALL;
}


void ManageRootDirDlg::OnBnClickedExcludeFolderBtn()
{
	VC_TRY;

	const int index = _rootList.GetCurSel();
	if( index < 0 )
		return;

	CollectionDB * collection = GetDocument()->GetCurrentCollection();
	if( !collection )
		return;

	EntryHandle rootHandle = collection->GetRoot( index );
	Entry * root = collection->FindEntry( rootHandle );
	if( !root )
		return;

	CollectionDB::RootInfo & rootInfo = collection->GetRootInfo( rootHandle );

	CExcludedPathDlg dlg;
	dlg.excludedPath = rootInfo.excludedPath;
	dlg.rootDir = root->filePath.c_str();

	if( dlg.DoModal() != IDOK )
		return;

	rootInfo.excludedPath.swap( dlg.excludedPath );

	GetDocument()->InvalidateDatabase();

	VC_CATCH_ALL;
}


void ManageRootDirDlg::OnBnClickedDeleteRootBtn()
{
	VC_TRY;

	const int index = _rootList.GetCurSel();
	if( index < 0 )
		return;

	CollectionDB * collection = GetDocument()->GetCurrentCollection();
	if( !collection )
		return;

	EntryHandle rootHandle = collection->GetRoot( index );
	Entry * root = collection->FindEntry( rootHandle );

	DeleteRootDlg dlg;
	dlg.comment = ResFormat( IDS_COMMENT_DELETE_ROOT, root->title.c_str() );

	if( IDOK != dlg.DoModal() )
	{
		return;
	}

	collection->DeleteEntry( rootHandle );

	GetDocument()->InvalidateDatabase();

	UpdateRootList();

	VC_CATCH_ALL;
}

void ManageRootDirDlg::OnImportExport()
{
	VC_TRY;

	CommandInfo info;
	info.doc = GetDocument();

	CommandExecute::Instance().Process( (CommandID)_btnImportExport.m_nMenuResult, &info );

	GetDocument()->InvalidateDatabase();

	UpdateRootList();

	VC_CATCH_ALL;
}


void ManageRootDirDlg::OnCancel()
{
	GetDocument()->GetVideoTreeView()->RebuildTree();

	CDialog::OnCancel();
}
