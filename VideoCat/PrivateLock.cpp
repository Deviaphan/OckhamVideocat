// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "PrivateLock.h"
#include "CollectionDB.h"
#include "Traverse/TraverseCollection.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CPrivateLock, CDialog)

CPrivateLock::CPrivateLock(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PRIVATE_LOCK, pParent)
	, collectionPassword( _T( "" ) )
	, privatePassword( _T( "" ) )
	, deleteLock( FALSE )
	, hideCollectionPass( false )
	, hideDeleteLock( false )
	, hideNewPassword( true )
	, setNewPrivatePassword( FALSE )
	, newPassword( _T( "" ) )
{

}

CPrivateLock::~CPrivateLock()
{
}

void CPrivateLock::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT1, collectionPassword );
	DDX_Text( pDX, IDC_EDIT2, privatePassword );
	DDX_Check( pDX, IDC_DELETE_LOCK, deleteLock );
	DDX_Check( pDX, IDC_CHANGE_PRIVATE, setNewPrivatePassword );
	DDX_Text( pDX, IDC_EDIT3, newPassword );
}


BEGIN_MESSAGE_MAP(CPrivateLock, CDialog)
END_MESSAGE_MAP()


BOOL CPrivateLock::OnInitDialog()
{
	CDialog::OnInitDialog();

	if( hideCollectionPass )
		GetDlgItem( IDC_EDIT1 )->EnableWindow( FALSE );
	
	if( hideDeleteLock )
	{
		GetDlgItem( IDC_DELETE_LOCK )->EnableWindow( FALSE );
	}

	if( hideNewPassword )
	{
		GetDlgItem( IDC_CHANGE_PRIVATE )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT3 )->EnableWindow( FALSE );
	}

	return TRUE;
}

namespace Traverse
{
	struct ChangeLock : public Base
	{
		bool makePrivate = false;

		virtual Result Run( Entry & entry ) override
		{
			if( makePrivate )
				Set( entry.flags, EntryTypes::Private );
			else
				Clear( entry.flags, EntryTypes::Private );

			return CONTINUE;
		}
	};
}

bool LockFolder( CollectionDB & cdb, const EntryHandle & folderHandle )
{
	const CString & syncPassword = cdb.GetPassword();

	CPrivateLock lock;
	if( syncPassword.IsEmpty() )
		lock.hideCollectionPass = true;
	lock.hideDeleteLock = true;
	lock.hideNewPassword = false;

	if( IDOK != lock.DoModal() )
		return false;

	if( lock.collectionPassword != syncPassword )
		return false;

	if( lock.privatePassword.IsEmpty() )
		return false;

	const CString & privatePassword = cdb.GetPrivatePassword();
	if( privatePassword.IsEmpty() )
	{
		cdb.SetPrivatePassword( lock.privatePassword );
	}
	else
	{
		if( privatePassword != lock.privatePassword )
		{
			AfxMessageBox( L"Пароль не совпадает с уже используемым паролем для защищённых папок.", MB_OK | MB_ICONERROR );
			return false;
		}
	}

	if( lock.setNewPrivatePassword )
	{
		cdb.SetPrivatePassword( lock.newPassword );
	}

	Traverse::ChangeLock changeLock;
	changeLock.makePrivate = true;

	cdb.TraverseAll( folderHandle, &changeLock );

	return true;
}

bool UnlockFolder( CollectionDB & cdb, const EntryHandle & folderHandle )
{
	const CString & syncPassword = cdb.GetPassword();

	CPrivateLock lock;
	if( syncPassword.IsEmpty() )
		lock.hideCollectionPass = true;
	lock.hideDeleteLock = false;

	if( IDOK != lock.DoModal() )
		return false;

	if( lock.privatePassword.IsEmpty() )
		return false;

	if( syncPassword != lock.collectionPassword )
		return false;

	if( cdb.GetPrivatePassword() != lock.privatePassword )
	{
		AfxMessageBox( L"Пароль не совпадает с уже используемым паролем для защищённых папок.", MB_OK | MB_ICONERROR );
		return false;
	}

	if( !lock.deleteLock )
	{
		cdb.UnlockPrivate( lock.privatePassword );
		return true;
	}

	cdb.UnlockPrivate( lock.privatePassword );

	Traverse::ChangeLock changeLock;
	changeLock.makePrivate = false;

	cdb.TraverseAll( folderHandle, &changeLock );

	return true;
}

void GrantAccess( CollectionDB & cdb )
{
	CPrivateLock lock;
	lock.hideCollectionPass = true;
	lock.hideDeleteLock = true;

	if( IDOK != lock.DoModal() )
		return;

	cdb.UnlockPrivate( lock.privatePassword );
}

