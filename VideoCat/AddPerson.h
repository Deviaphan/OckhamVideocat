#pragma once
#include <vector>
#include "SharedBase/EntryBase.h"

class CAddPerson : public CDialog
{
	DECLARE_DYNAMIC(CAddPerson)

public:
	CAddPerson(CWnd* pParent = nullptr);
	virtual ~CAddPerson();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_PERSON };
#endif

	virtual BOOL OnInitDialog();

	void UpdateList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnOK2();
	afx_msg void OnAddPerson();
	afx_msg void OnRemoveSelected();

private:
	CListBox _filmListCtrl;

public:
	int _personId;
	int _filmId;
	std::vector<FilmId> _filmList;
};
