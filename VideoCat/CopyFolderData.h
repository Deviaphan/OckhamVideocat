#pragma once

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


class CopyFolderData : public CDialog
{
	DECLARE_DYNAMIC(CopyFolderData)

public:
	CopyFolderData(CWnd* pParent = nullptr);
	virtual ~CopyFolderData();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COPY_FOLDER_DATA };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange * pDX );

	DECLARE_MESSAGE_MAP()
	CCommentBox _commentBox;

public:
	CopyData data;
};
