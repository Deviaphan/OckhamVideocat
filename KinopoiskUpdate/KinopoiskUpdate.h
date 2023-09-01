
#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"


class CKinopoiskUpdateApp : public CWinApp
{
public:
	CKinopoiskUpdateApp();

public:
	virtual BOOL InitInstance();


	DECLARE_MESSAGE_MAP()
};

extern CKinopoiskUpdateApp theApp;