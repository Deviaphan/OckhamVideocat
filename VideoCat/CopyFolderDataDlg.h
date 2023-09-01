#pragma once
#include "CollectionDB.h"
#include "CommentBox.h"

class CopyData
{
public:
	CopyData()
		: description( FALSE )
		, poster( FALSE )
		, tags( FALSE )
		, comments( FALSE )
	{
	}

public:
	BOOL description;
	BOOL poster;
	BOOL tags;
	BOOL comments;
};


class CopyFolderDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CopyFolderDataDlg)

public:
	CopyFolderDataDlg(CWnd* pParent = nullptr);
	virtual ~CopyFolderDataDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COPY_FOLDER_DATA };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange( CDataExchange * pDX ) override;

	DECLARE_MESSAGE_MAP()
	CCommentBox _commentBox;

public:
	CopyData data;
};

void CopyCards( CollectionDB & cdb, Entry & root, CopyData & data );

void CopyPosters( CollectionDB & cdb, Entry & root );
