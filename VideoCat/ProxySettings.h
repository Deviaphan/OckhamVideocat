#pragma once

class CProxySettings : public CPropertyPage
{
	DECLARE_DYNAMIC(CProxySettings)

public:
	CProxySettings();
	virtual ~CProxySettings();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROXY_SETTINGS };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	void OnOK() {}
	void OnCancel() {}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckIp();

public:
	int proxyType;
	DWORD proxyIP;
	short proxyPort;
	UINT timeout;
};
