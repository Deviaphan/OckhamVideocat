#pragma once

#include "resource.h"

class SerialFillDlg : public CDialog
{
	DECLARE_DYNAMIC(SerialFillDlg)

public:
	SerialFillDlg(CWnd* pParent = nullptr);
	virtual ~SerialFillDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERIAL_FILL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	int fillType;
};
