#pragma once
#include "EntryHandle.h"

class CollectionDB;

class CPrivateLock : public CDialog
{
	DECLARE_DYNAMIC(CPrivateLock)

public:
	CPrivateLock(CWnd* pParent = nullptr); 
	virtual ~CPrivateLock();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PRIVATE_LOCK };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	CString collectionPassword;
	CString privatePassword;
	BOOL deleteLock;
	BOOL setNewPrivatePassword;
	CString newPassword;

	bool hideCollectionPass;
	bool hideDeleteLock;
	bool hideNewPassword;
};

bool LockFolder( CollectionDB & cdb, const EntryHandle & folderHandle );

bool UnlockFolder( CollectionDB & cdb, const EntryHandle & folderHandle );

void GrantAccess( CollectionDB & cdb );

