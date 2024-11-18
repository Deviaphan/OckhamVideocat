// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include "Updater.h"
#include "UpdaterDlg.h"
#include <shellapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CUpdaterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CUpdaterApp::CUpdaterApp()
{
}

CUpdaterApp theApp;


BOOL CUpdaterApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof( InitCtrls );
	InitCtrls.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControlsEx( &InitCtrls );

	CWinApp::InitInstance();

	CString url;
	CString updateDir;

	int numArgs = 0;

	wchar_t ** args = ::CommandLineToArgvW( ::GetCommandLineW(), &numArgs );
	
	switch( numArgs  )
	{
		case 1:
		{
			AfxMessageBox( L"Скачайте, распакуйте и замените файлы самостоятельно.", MB_OK | MB_ICONINFORMATION | MB_TOPMOST );
			return FALSE;
		}
		case 2:
		{
			updateDir = args[1];
			break;
		}
		case 3:
		{
			url = args[1];// L"http://videocat.maagames.ru/files/videocat_380.7z";
			updateDir = args[2];
			break;
		}
	}

	LocalFree( args );

	CUpdaterDlg dlg;
	dlg.downloadUrl = url;
	dlg.installPath = updateDir;
	if( !dlg.installPath.IsEmpty() )
	{
		if( dlg.installPath.GetAt( dlg.installPath.GetLength() - 1 ) != '\\' )
			dlg.installPath += L"\\";
	}
	dlg.currentStep = numArgs == 3 ? 1 : 3;

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	//if (nResponse == IDOK)
	//{
	//}
	//else if (nResponse == IDCANCEL)
	//{
	//}
	//else if (nResponse == -1)
	//{
	//}


#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

