#pragma once

class CThumbsApp : public CWinApp
{
public:
	CThumbsApp();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CThumbsApp theApp;
