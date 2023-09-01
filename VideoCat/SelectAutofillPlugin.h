#pragma once
#include "PluginManager.h"

class CSelectAutofillPlugin : public CDialog
{
	DECLARE_DYNAMIC(CSelectAutofillPlugin)

public:
	CSelectAutofillPlugin(CWnd* pParent = nullptr);
	virtual ~CSelectAutofillPlugin();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SELECT_AUTOFILL_PLUGIN };
#endif

public:
	PluginID GetPluginID();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
private:
	int _choise;
	CComboBox _pluginList;
};
