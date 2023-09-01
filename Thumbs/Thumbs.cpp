// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "Thumbs.h"
#include "ThumbsDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CThumbsApp theApp;

BEGIN_MESSAGE_MAP( CThumbsApp, CWinApp )
END_MESSAGE_MAP()

CThumbsApp::CThumbsApp()
{}

BOOL CThumbsApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof( InitCtrls );
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx( &InitCtrls );

	CWinApp::InitInstance();

	SetRegistryKey( _T( "maaGames" ) );

	CThumbsDlg dlg;
	m_pMainWnd = &dlg;
	dlg._filePath = m_lpCmdLine;

	INT_PTR nResponse = dlg.DoModal();

	return TRUE;
}

