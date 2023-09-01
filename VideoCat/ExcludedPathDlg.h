#pragma once
#include <set>

class CExcludedPathDlg : public CDialog
{
	DECLARE_DYNAMIC(CExcludedPathDlg)

public:
	explicit CExcludedPathDlg( CWnd* pParent = nullptr );
	~CExcludedPathDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXCLUDED_FILES };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void RebuildList();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAddExluded();
	afx_msg void OnRemoveExcluded();

private:
	CString _excludeFolder;
	CListBox _excludedList;

public:
	std::set<CString> excludedPath;
	CString rootDir;
};
