#pragma once

#include "CollectionDB.h"

class EraseData
{
public:
	EraseData()
		: notEraseFolder( FALSE )
		, eraseKinopoisk( FALSE )
		, eraseCounters( FALSE )
		, eraseTags( FALSE )
		, eraseComments( FALSE )
		, eraseTechInfo( FALSE )
		, erasePosters( FALSE )
		, eraseThumbs( FALSE )
	{
	}

public:
	BOOL notEraseFolder;
	BOOL eraseKinopoisk;
	BOOL eraseCounters;
	BOOL eraseTags;
	BOOL eraseComments;
	BOOL eraseTechInfo;
	BOOL erasePosters;
	BOOL eraseThumbs;
};

class CEraseDlg: public CDialog
{
	DECLARE_DYNAMIC( CEraseDlg )

public:
	explicit CEraseDlg( CWnd* pParent = nullptr );
	~CEraseDlg() override;

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_ERASE
	};
#endif

protected:
	void DoDataExchange( CDataExchange* pDX ) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnErase();

public:
	EraseData data;
};

void EraseCards( CollectionDB& cdb, Entry& root, EraseData& data );

void ErasePosters( CollectionDB& cdb, Entry& root );
void EraseThumbs( CollectionDB& cdb, Entry& root );
