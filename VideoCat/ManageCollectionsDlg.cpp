// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ManageCollectionsDlg.h"
#include "AddCollectionDlg.h"
#include "Commands/CommandExecute.h"
#include "CreateBackupDlg.h"
#include "CreateCollectionDlg.h"
#include "GlobalSettings.h"
#include "IconManager.h"
#include "LoadBackupDlg.h"
#include "VideoCatDoc.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "ResString.h"
#include <filesystem>
#include "resource.h"

enum
{
	MENU_ITEM_LOAD_BACKUP = 1050,
	MENU_ITEM_CREATE_BACKUP
};

IMPLEMENT_DYNAMIC(ManageCollectionsDlg, CDialog)

ManageCollectionsDlg::ManageCollectionsDlg( CVideoCatDoc * doc, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MANAGE_COLLECTION, pParent)
	, _doc(doc)
{

}

ManageCollectionsDlg::~ManageCollectionsDlg()
{
}

void ManageCollectionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_ADD_COLLECTION, _btnAddCollection );
	DDX_Control( pDX, IDC_COLLECTION_LIST, _collectionList );
	DDX_Control( pDX, IDC_CREATE_COLLECTION, _btnCreateCollection );
	DDX_Control( pDX, IDC_EDIT_COLLECTION, _btnEditCollection );
	DDX_Control( pDX, IDC_MAINTENANCE, _btnMaintenance );
	DDX_Control( pDX, IDC_REMOVE_COLLECTION, _btnRemoveCollection );
	DDX_Control( pDX, IDC_BACKUP, _btnBackupCollection );
}


BEGIN_MESSAGE_MAP(ManageCollectionsDlg, CDialog)
	ON_BN_CLICKED( IDC_ADD_COLLECTION, &ManageCollectionsDlg::OnAttachCollection )
	ON_BN_CLICKED( IDC_CREATE_COLLECTION, &ManageCollectionsDlg::OnCreateCollection )
	ON_BN_CLICKED( IDC_EDIT_COLLECTION, &ManageCollectionsDlg::OnEditCollection )
	ON_BN_CLICKED( IDC_MAINTENANCE, &ManageCollectionsDlg::OnMaintenance )
	ON_BN_CLICKED( IDC_REMOVE_COLLECTION, &ManageCollectionsDlg::OnDetachCollection )
	ON_LBN_SELCHANGE( IDC_COLLECTION_LIST, &ManageCollectionsDlg::OnLbnSelchangeCollectionList )
	ON_BN_CLICKED( IDC_BACKUP, &ManageCollectionsDlg::OnBackupCollection )
END_MESSAGE_MAP()

BOOL ManageCollectionsDlg::OnInitDialog()
{
	_btnCreateCollection._iconID = IconManager::CollectionAdd;
	_btnEditCollection._iconID = IconManager::CollectionEdit;
	_btnRemoveCollection._iconID = IconManager::CollectionRemove;
	_btnAddCollection._iconID = IconManager::CollectionAdd;
	_btnMaintenance._iconID = IconManager::CollectionCheck;
	_btnBackupCollection._iconID = IconManager::Collections;

	CDialog::OnInitDialog();

	RebuildCollectionList();

	_menuBackup.CreatePopupMenu();
	_menuBackup.AppendMenuW( MF_STRING, MENU_ITEM_LOAD_BACKUP, ResString(IDS_LOAD_BACKUP) );
	_menuBackup.AppendMenuW( MF_STRING, MENU_ITEM_CREATE_BACKUP, ResString(IDS_CREATE_BACKUP) );

	_btnBackupCollection.m_hMenu = _menuBackup.GetSafeHmenu();

	return TRUE;
}

void ManageCollectionsDlg::RebuildCollectionList()
{
	_collectionList.ResetContent();

	const unsigned numCollections = GetDocument()->GetNumCollections();
	for( unsigned index = 0; index < numCollections; ++index )
	{
		const CVideoCatDoc::CollectionInfo * info = GetDocument()->GetCollectionInfo( index );
		_collectionList.AddString( info->name );
	}
}


void ManageCollectionsDlg::OnOK()
{
	// запрещаю закрытие по случайном унажатию Enter
	//CDialog::OnOK();
}

void ManageCollectionsDlg::OnCancel()
{
	const CollectionDB * cdb = GetDocument()->GetCurrentCollection();
	if( cdb )
	{
		const int numRoots = cdb->GetNumRoots();
		if( numRoots <= 0 )
		{
			CommandInfo info;
			info.doc = GetDocument();

			CommandExecute::Instance().Process( CommandID::ManageRoots, &info );
		}
	}

	CDialog::OnCancel();
}

void ManageCollectionsDlg::OnLbnSelchangeCollectionList()
{
	const int curSel = _collectionList.GetCurSel();
	_btnEditCollection.EnableWindow( curSel >= 0 );
	_btnMaintenance.EnableWindow( curSel >= 0 );
	_btnRemoveCollection.EnableWindow( curSel >= 0 );
}


void ManageCollectionsDlg::OnCreateCollection()
{
	VC_TRY;

	this->ShowWindow( SW_HIDE );

	CreateCollectionDlg dlg;
	INT_PTR result = dlg.DoModal();

	this->ShowWindow( SW_SHOW );

	if( result != IDOK )
		return;

	GetDocument()->CreateCollection( dlg.title, dlg.path );

	RebuildCollectionList();
	
	VC_CATCH_ALL;
}


void ManageCollectionsDlg::OnEditCollection()
{
	VC_TRY;

	int index = _collectionList.GetCurSel();
	if( index < 0 )
		return;

	const CVideoCatDoc::CollectionInfo * info = GetDocument()->GetCollectionInfo( index );
	if( !info )
		return;

	this->ShowWindow( SW_HIDE );
	CreateCollectionDlg dlg;
	dlg.title = info->name;
	dlg.path = info->fullPath;
	dlg.isEditMode = true;

	INT_PTR result = dlg.DoModal();

	this->ShowWindow( SW_SHOW );

	if( result != IDOK )
		return;

	CVideoCatDoc::CollectionInfo newInfo( *info );
	newInfo.name = dlg.title;

	GetDocument()->EditCollection( newInfo );

	RebuildCollectionList();

	VC_CATCH_ALL;
}


void ManageCollectionsDlg::OnAttachCollection()
{
	VC_TRY;

	this->ShowWindow( SW_HIDE );

	AddCollectionDlg dlg;

	dlg.path = GetGlobal().GetDefaultDirectory();

	INT_PTR result = dlg.DoModal();

	this->ShowWindow( SW_SHOW );

	if( result != IDOK )
		return;

	GetDocument()->AttachCollection( dlg.renameTitle ? dlg.title : CString(), dlg.path );

	RebuildCollectionList();

	VC_CATCH_ALL;
}


void ManageCollectionsDlg::OnDetachCollection()
{
	VC_TRY;

	int index = _collectionList.GetCurSel();
	if( index < 0 )
		return;

	const CVideoCatDoc::CollectionInfo * info = GetDocument()->GetCollectionInfo( index );
	if( !info )
		return;

	if( info->id == GetDocument()->GetTutorialID() )
	{
		AfxMessageBox( L"Руководство пользователя отключить нельзя", MB_OK | MB_ICONINFORMATION );
		return;
	}

	if( IDYES != AfxMessageBox( L"Вы действительно хотите отключить файл колелкции?\r\nПри отключении файлы удалены не будут и коллекцию можно будет подключить обратно.", MB_YESNO | MB_ICONQUESTION ) )
		return;

	GetDocument()->DetachCollection( info->id );

	RebuildCollectionList();

	VC_CATCH_ALL;
}



void ManageCollectionsDlg::OnMaintenance()
{
	VC_TRY;

	int index = _collectionList.GetCurSel();
	if( index < 0 )
		return;

	const CVideoCatDoc::CollectionInfo * info = GetDocument()->GetCollectionInfo( index );
	if( !info )
		return;

	GetDocument()->ReloadCollection( info->id );
	GetDocument()->GetVideoTreeView()->UpdateCollectionList();
	GetDocument()->GetVideoTreeView()->RebuildTree();

	if( GetDocument()->GetCurrentCollection()->Maintenance( GetDocument()->GetThumbDir() ) )
	{
		GetDocument()->InvalidateDatabase();
	}

	VC_CATCH_ALL;
}


void ManageCollectionsDlg::OnBackupCollection()
{
	VC_TRY;

	const int menuId = _btnBackupCollection.m_nMenuResult;
	if( menuId == MENU_ITEM_LOAD_BACKUP )
	{
		LoadBackup();
	}
	else
	{
		CreateBackup();
	}

	VC_CATCH_ALL;
}

void ManageCollectionsDlg::LoadBackup()
{
	LoadBackupDlg dlg;

	dlg.extractFolder = GetGlobal().GetDefaultDirectory();

	if( dlg.DoModal() != IDOK )
		return;

	CWaitCursor waiting;

	std::error_code err;
	if( !std::filesystem::exists( dlg.backupPath.GetString(), err ) )
		return;

	CollectionDB::DBID collectionID;
	{
		auto curCollection = GetDocument()->GetCurrentCollection();
		if( curCollection )
			collectionID = curCollection->GetId();
	}

	GetDocument()->UnloadCurrentCollection();

	wchar_t cmdLine[2048];
	wsprintf( cmdLine, L"\"%s\" x \"%s\" -o\"%s\"", GetGlobal().Get7zFileName().GetString(), dlg.backupPath.GetString(), dlg.extractFolder.GetString() );

	PROCESS_INFORMATION piProcInfo;
	ZeroMemory( &piProcInfo, sizeof( PROCESS_INFORMATION ) );
	STARTUPINFO siStartInfo;
	ZeroMemory( &siStartInfo, sizeof( STARTUPINFO ) );
	siStartInfo.cb = sizeof( STARTUPINFO );

	BOOL bSuccess = CreateProcess(
		GetGlobal().Get7zFileName().GetString(),
		cmdLine,
		nullptr,          // process security attributes 
		nullptr,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		NORMAL_PRIORITY_CLASS,             // creation flags 
		nullptr,          // use parent's environment 
		nullptr,          // use parent's current directory 
		&siStartInfo,
		&piProcInfo
	);

	if( !bSuccess )
		return;

	::WaitForSingleObject( piProcInfo.hProcess, INFINITE );

	GetDocument()->ReloadCollection( collectionID );
	GetDocument()->GetVideoTreeView()->RebuildTree();
}

void ManageCollectionsDlg::CreateBackup()
{
	const int index = _collectionList.GetCurSel();
	if( index < 0 )
	{
		AfxMessageBox( ResString( IDS_SELECT_COLLECTION ), MB_OK | MB_TOPMOST );
		return;
	}

	const CVideoCatDoc::CollectionInfo * info = GetDocument()->GetCollectionInfo( index );
	if( !info )
		return;

	{
		CollectionDB * curCollection = GetDocument()->GetCurrentCollection();
		if( curCollection && curCollection->GetId() == info->id )
		{
			// перед экспортированием убеждаемся, что все изменения сохранены
			GetDocument()->SaveCurrentCollection();
		}
	}


	CreateBackupDlg dlg;
	dlg.collectionTitle = info->name;
	dlg.filePath = info->fullPath + L".vcbackup";

	if( dlg.DoModal() != IDOK )
		return;

	if( dlg.filePath.IsEmpty() )
		return;

	CWaitCursor waiting;

	CString collectionPath = info->fullPath;
	CString progressPath = dlg.addProgress ? GetGlobal().GetDefaultDirectory() + info->id + L".progress" : CString(L"");
	CString framesPath = L"";
	if( dlg.addFrames )
	{
		std::filesystem::path path( info->fullPath.GetString() );
		path = path.remove_filename();
		path += info->id.GetString();
		framesPath = path.c_str();
	}

	CString commandLine;
	commandLine += L"a ";
	commandLine += L"\"" + dlg.filePath + L"\"";
	commandLine += L" \"" + collectionPath + L"\"";
	if( !progressPath.IsEmpty() )
		commandLine += L" \"" + progressPath + L"\"";
	if( !framesPath.IsEmpty() )
		commandLine += L" \"" + framesPath + L"\"";
	commandLine += L" -mx=0";

	wchar_t cmdLine[2048];
	wsprintf( cmdLine, L"%s %s", GetGlobal().Get7zFileName().GetString(), commandLine.GetString() );

	PROCESS_INFORMATION piProcInfo;
	ZeroMemory( &piProcInfo, sizeof( PROCESS_INFORMATION ) );
	STARTUPINFO siStartInfo;
	ZeroMemory( &siStartInfo, sizeof( STARTUPINFO ) );
	siStartInfo.cb = sizeof( STARTUPINFO );

	BOOL bSuccess = CreateProcess(
		GetGlobal().Get7zFileName().GetString(),
		cmdLine,
		nullptr,          // process security attributes 
		nullptr,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		NORMAL_PRIORITY_CLASS,             // creation flags 
		nullptr,          // use parent's environment 
		nullptr,          // use parent's current directory 
		&siStartInfo,
		&piProcInfo
	);

	if( !bSuccess )
		return;

	::WaitForSingleObject( piProcInfo.hProcess, INFINITE );
}
