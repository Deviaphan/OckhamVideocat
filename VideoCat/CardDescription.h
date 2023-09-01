#pragma once
#include <set>
#include "TagManager.h"
#include "CardPageBase.h"

class CCardDescription : public CMFCPropertyPage, public ICardPageBase
{
	DECLARE_DYNAMIC(CCardDescription)

public:
	CCardDescription();
	virtual ~CCardDescription();

	virtual BOOL OnInitDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_DESCRIPTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	void UpdateFilmTags();
	void UpdateAllTags();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNewTag();
	afx_msg void OnDeleteTag();
	afx_msg void OnClearTags();
	afx_msg void OnRemoveUsedTags();
	afx_msg void OnAddFromAllTags();
	afx_msg void OnRenameTag();
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );

public:
	virtual bool SaveData() override;
	virtual void ReinitTab() override;

private:
	TagManager * _tagManager;

	CString _tagline;
	CString _description;
	CString _comment;

	CListBox _allTags;
	CListBox _usedTags;
	CString _newTagName;

	std::set<TagId> _currentTags;
	CString _text;

};
