#pragma once
#include <vector>

class CCheckBoxList : public CDialog
{
	DECLARE_DYNAMIC(CCheckBoxList)

public:
	CCheckBoxList(CWnd* pParent = nullptr);
	virtual ~CCheckBoxList();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECK_LIST };
#endif

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()

private:
	CCheckListBox _listBox;

public:
	std::vector<CString> names;
	std::vector<BOOL> checks;
};
