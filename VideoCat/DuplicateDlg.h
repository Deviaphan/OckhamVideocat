#pragma once
#include <map>
#include <vector>
#include "CollectionDB.h"

class Entry;

class DuplicatesGrid : public CMFCPropertyGridCtrl
{
	DECLARE_DYNAMIC( DuplicatesGrid )
	DECLARE_MESSAGE_MAP()
public:
	void AdjustLayout()
	{
		m_nLeftColumnWidth = 80;
		CMFCPropertyGridCtrl::AdjustLayout();
	}
};

class CDuplicateDlg : public CDialog
{
	DECLARE_DYNAMIC( CDuplicateDlg )

public:
	CDuplicateDlg( CWnd* pParent = nullptr );
	virtual ~CDuplicateDlg();

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_DUBLICATES
	};
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );

	DECLARE_MESSAGE_MAP()

private:
	DuplicatesGrid _grid;

public:
	std::map<unsigned int, std::vector<Entry*> > data;
	CollectionDB * cdb;
};
