#pragma once

class CErrorDlg : public CDialog
{
	DECLARE_DYNAMIC(CErrorDlg)

public:
	explicit CErrorDlg( const CString & msg, CWnd* pParent = nullptr );
	~CErrorDlg() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRAM_ERROR };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()

public:
	CString message;
};

bool ShowError( const CString & msg, bool ignore );
