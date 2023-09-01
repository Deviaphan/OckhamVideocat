// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "VideoCat.h"

#include <thread>
#include "MainFrm.h"

#include "VideoCatDoc.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "Version.h"
#include "GlobalSettings.h"
#include "IconManager.h"
#include "Commands/RegisterAllCommands.h"
#include "Commands/CommandExecute.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CVideoCatApp theApp;

void CheckUpdate()
{
	try
	{
		int lastUpdate = theApp.GetProfileInt( L"", L"lastupdate", 0 );
		SYSTEMTIME st;
		GetSystemTime( &st );

		if( lastUpdate > st.wDay )
			lastUpdate -= 30;

		// Проверяю обновление три раза в месяц.
		if( (st.wDay - lastUpdate) >= 10 )
		{
			if( GetGlobal().versionInfo.HasNewVersion() )
			{
				CommandExecute::Instance().Process( CommandID::AboutVideocat, nullptr );
			}
			else
			{
				// Ставлю флаг проверки обновления, только если обновления не нашлось.
				// Чтобы надоедать обновлением при каждом запуске, пока не обновят.
				theApp.WriteProfileInt( L"", L"lastupdate", st.wDay );
			}
		}
	}
	catch( ... )
	{
	}
}

BEGIN_MESSAGE_MAP(CVideoCatApp, CWinAppEx)
END_MESSAGE_MAP()


CVideoCatApp::CVideoCatApp()
	:gdiplusStartupInput( 0 )
	, gdiplusToken( 0 )
{
	// формат для строки: ИмяКомпании.ИмяПродукта.СубПродукт.СведенияОВерсии
	SetAppID(_T("maaGames.VideoCat.AppID.1"));
}


BOOL CVideoCatApp::InitInstance()
{
	srand( ::GetTickCount() );

	if( _si.IsSecondInstance() )
	{
		HWND hWnd = FindWindow( nullptr, L"Ockham:VideoCat - домашняя видеоколлекция" );
		if( hWnd )
		{
			::ShowWindow( hWnd, SW_SHOW );
			::SetForegroundWindow( hWnd );
		}

		return FALSE;
	}

	//CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_LunaBlue);
	CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows7 ) );

	// InitCommonControlsEx() требуются для Windows XP, если манифест
	// приложения использует ComCtl32.dll версии 6 или более поздней версии для включения
	// стилей отображения.  В противном случае будет возникать сбой при создании любого окна.
	INITCOMMONCONTROLSEX initCtrls;
	initCtrls.dwSize = sizeof( INITCOMMONCONTROLSEX );
	// Выберите этот параметр для включения всех общих классов управления, которые необходимо использовать
	// в вашем приложении.
	initCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&initCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// Initialize GDI+.
	Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, nullptr );


	// Стандартная инициализация
	// Если эти возможности не используются и необходимо уменьшить размер
	// конечного исполняемого файла, необходимо удалить из следующего
	// конкретные процедуры инициализации, которые не требуются
	// Измените раздел реестра, в котором хранятся параметры
	SetRegistryKey(_T("maaGames"));

	// Зарегистрируйте шаблоны документов приложения.  Шаблоны документов
	//  выступают в роли посредника между документами, окнами рамок и представлениями
	CSingleDocTemplate* pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CVideoCatDoc),
		RUNTIME_CLASS(CMainFrame),       // основное окно рамки SDI
		RUNTIME_CLASS(CVideoCatView));

	AddDocTemplate(pDocTemplate);

	// Синтаксический разбор командной строки на стандартные команды оболочки, DDE, открытие файлов
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Команды диспетчеризации, указанные в командной строке.  Значение FALSE будет возвращено, если
	// приложение было запущено с параметром /RegServer, /Register, /Unregserver или /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	CWaitCursor waiting;

	try
	{
		RegisterAllCommands();

		GetGlobal().Init();
		GetGlobal().versionInfo.ReadVersion();

		SetDllDirectory( GetGlobal().GetProgramDirectory() + "\\videocat\\" );

		std::thread checkUpdate( CheckUpdate );
		checkUpdate.detach();

	}
	catch( ... )
	{}

	POSITION pos = pDocTemplate->GetFirstDocPosition();
	CVideoCatDoc * doc = dynamic_cast<CVideoCatDoc *>(pDocTemplate->GetNextDoc( pos ));
	if( doc )
	{
		//fwd
		void SetWin10ThemeByColor( CVideoCatDoc * doc, COLORREF color );

		SetWin10ThemeByColor( doc, GetGlobal().theme->bgBottomColor );

		doc->GetVideoTreeView()->InitializeTree();
		doc->GetVideoCatView()->SetFocus();
	}

	this->m_pMainWnd->ShowWindow( SW_SHOWMAXIMIZED );

	return TRUE;
}

int CVideoCatApp::ExitInstance()
{
	GetIconManager().Destroy();
	GetGlobal().Destroy();

	if( gdiplusToken )
		Gdiplus::GdiplusShutdown( gdiplusToken );

	return __super::ExitInstance();
}
