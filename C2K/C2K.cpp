#include "stdafx.h"
#include "C2K.h"
#include "C2KDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CC2KApp, CWinApp)
END_MESSAGE_MAP()


CC2KApp::CC2KApp()
{
}

CC2KApp theApp;


BOOL CC2KApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	SetRegistryKey( _T( "maaGames" ) );

	CC2KDlg dlg;
	m_pMainWnd = &dlg;
	dlg.userLibrary = m_lpCmdLine;

	if( dlg.userLibrary.IsEmpty() )
		return FALSE;

	// т.к. параметры передаются в кавычках, то кавычки теперьн адо удалить
	dlg.userLibrary = dlg.userLibrary.Mid( 1, dlg.userLibrary.GetLength() - 2 );

	dlg.DoModal();

	return TRUE;
}

