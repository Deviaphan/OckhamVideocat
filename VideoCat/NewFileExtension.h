#pragma once

#include "resource.h"

// CNewFileExtension dialog

class CNewFileExtension : public CDialog
{
	DECLARE_DYNAMIC(CNewFileExtension)

public:
	CNewFileExtension(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CNewFileExtension();

// Dialog Data
	enum { IDD = IDD_EXTENSION_TYPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString newExtension;
};
