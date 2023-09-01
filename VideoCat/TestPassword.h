#pragma once

class CTestPassword : public CDialog
{
	DECLARE_DYNAMIC(CTestPassword)

public:
	explicit CTestPassword(CWnd* pParent);
	~CTestPassword() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_PASSWORD };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()

public:
	CString password;
};
