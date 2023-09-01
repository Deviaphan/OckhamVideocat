#pragma once

class ThemeName : public CDialogEx
{
	DECLARE_DYNAMIC(ThemeName)

public:
	ThemeName(CWnd* pParent = nullptr); 
	virtual ~ThemeName();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEME_NAME };
#endif

protected:
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:
	CString themeName;
};
