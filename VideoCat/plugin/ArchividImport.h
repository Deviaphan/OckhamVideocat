#pragma once

class CArchividImport : public CDialog
{
	DECLARE_DYNAMIC(CArchividImport)

public:
	CArchividImport(CWnd* pParent = nullptr);
	virtual ~CArchividImport();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ARCHIVID };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	CString archividDir;
};
