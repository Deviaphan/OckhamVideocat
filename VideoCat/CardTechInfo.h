#pragma once
#include "EntryHandle.h"
#include "CardPageBase.h"
class CollectionDB;


class CCardTechInfo : public CMFCPropertyPage, public ICardPageBase
{
	DECLARE_DYNAMIC(CCardTechInfo)

public:
	CCardTechInfo();
	virtual ~CCardTechInfo();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_TECH_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void RebuildList();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnReadFileInfo();
	afx_msg void OnBnClearFileInfo();
	afx_msg void OnBnClickedCopyInfo();

public:
	virtual void ReinitTab() override;
	virtual bool SaveData() override;

private:
	CListCtrl _list;
};

BOOL ReadFileInfo( CollectionDB & cdb, const EntryHandle & root );
