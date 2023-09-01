// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "UpdaterDlg.h"
#include "resource.h"
#include "Net/CurlBuffer.h"
#include "SharedBase/vc_defines.h"
#include <pathcch.h>
#include <fstream>
#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void __cdecl DownloadThread( void * ptr );
void __cdecl UnpackThread( void * ptr );
void __cdecl InstallThread( void * ptr );

void CUpdaterDlg::StartNewUpdater( CString newUpdater, CString updateDir )
{
	wchar_t cmdLine[2048];
	wsprintf( cmdLine, L"\"%s\" \"%s\\\"", newUpdater.GetString(), updateDir.GetString() );

	PROCESS_INFORMATION piProcInfo;
	ZeroMemory( &piProcInfo, sizeof( PROCESS_INFORMATION ) );
	STARTUPINFO siStartInfo;
	ZeroMemory( &siStartInfo, sizeof( STARTUPINFO ) );
	siStartInfo.cb = sizeof( STARTUPINFO );

	CreateProcess(
		newUpdater.GetString(),
		cmdLine,
		nullptr,          // process security attributes 
		nullptr,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		NORMAL_PRIORITY_CLASS,             // creation flags 
		nullptr,          // use parent's environment 
		nullptr,          // use parent's current directory 
		&siStartInfo,
		&piProcInfo
	);
}

void __cdecl DownloadThread( void * ptr )
{
	CUpdaterDlg * dlg = (CUpdaterDlg *)ptr;
	if( !dlg )
		return;

	const bool result = dlg->DownloadUpdate();
	dlg->CleanThread();

	// после загрузки выполняется распаковка
	if( result )
	{
		dlg->SetThread( (HANDLE)_beginthread( UnpackThread, 0, dlg ) );
	}
}

void __cdecl UnpackThread( void * ptr )
{
	CUpdaterDlg * dlg = (CUpdaterDlg *)ptr;
	if( !dlg )
		return;

	const bool result = dlg->UnpackUpdate();
	dlg->CleanThread();
	
	// После распаковки запускается новый апдейтер, сразу с этапа установки, а этот закрывается
	if( result )
	{
		CString newUpdater = dlg->unzipDir;
		newUpdater += L"videocat\\updater.exe";
		CUpdaterDlg::StartNewUpdater( newUpdater, dlg->installPath );
	}

	dlg->PostMessage( WM_CLOSE, 0, 0 );
}

void __cdecl InstallThread( void * ptr )
{
	CUpdaterDlg * dlg = (CUpdaterDlg *)ptr;
	if( !dlg )
		return;

	const bool result = dlg->InstallUpdate();
	dlg->CleanThread();

	// закрываем по завершению
	dlg->PostMessage( WM_CLOSE, 0, 0 );
}

void CUpdaterDlg::SetThread( HANDLE h )
{
	_workingThread = h;
}

void CUpdaterDlg::CleanThread()
{
	_workingThread = INVALID_HANDLE_VALUE;
}

CUpdaterDlg::CUpdaterDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_UPDATER_DIALOG, pParent)
	, _workingThread( INVALID_HANDLE_VALUE )
	, currentStep(1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESS_1, _progress1 );
	DDX_Control( pDX, IDC_PROGRESS_2, _progress2 );
	DDX_Control( pDX, IDC_PROGRESS_3, _progress3 );
}

BEGIN_MESSAGE_MAP(CUpdaterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


BOOL CUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);	
	SetIcon(m_hIcon, FALSE);
	
	if( currentStep == 1 )
	{
		Get7zPath();

		wchar_t tempPath[MAX_PATH + 1];
		GetTempPath( MAX_PATH, tempPath );

		_updateZipPath = tempPath; 
		_updateZipPath += L"update.7z";

		unzipDir = tempPath;
		unzipDir += L"vc_upd\\";

		SetThread( (HANDLE)_beginthread( DownloadThread, 0, this ) );
	}
	else
	{
		GetParentDir();

		SetThread( (HANDLE)_beginthread( InstallThread, 0, this ) );
	}

	if( _workingThread == INVALID_HANDLE_VALUE )
		return FALSE;

	return TRUE;
}

void CUpdaterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUpdaterDlg::OnOK()
{
	// запрещаю закрытие по Интеру
	//CDialog::OnOK();
}

void CUpdaterDlg::OnCancel()
{
	// просто убиваю поток на полуслове. Т.к. Апдейтер это отдельная утилита, то утечек памяти не будет (сразу всё подчистится при закрытии апдейтера)
	if( _workingThread != INVALID_HANDLE_VALUE )
		CloseHandle( _workingThread );

	CDialog::OnCancel();
}

void CUpdaterDlg::SetInstallationStep( int curStep )
{
	currentStep = curStep;

	GetDlgItem( IDC_TITLE_2 )->EnableWindow( currentStep == 2 );
	_progress2.EnableWindow( currentStep == 2 );
	if( currentStep == 2 )
		_progress2.SetMarquee( TRUE, 50 );

	GetDlgItem( IDC_TITLE_3 )->EnableWindow( currentStep >= 3 );
	_progress3.EnableWindow( currentStep >= 3 );
	if( currentStep == 3 )
	{
		_progress3.SetMarquee( TRUE, 50 );

		_progress1.SetRange( 0, 100 );
		_progress1.EnableWindow( FALSE );
		_progress1.SetPos( 100 );

		_progress2.SetRange( 0, 100 );
		_progress2.EnableWindow( FALSE );
		_progress2.SetPos( 100 );
	}
}

void CUpdaterDlg::Get7zPath()
{
	wchar_t path[MAX_PATH] = L"";

	HMODULE thisModule = nullptr;
	VERIFY( GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR) & __FUNCTION__, &thisModule ) );
	VERIFY( GetModuleFileName( thisModule, path, MAX_PATH ) );

	::PathCchRemoveFileSpec( path, MAX_PATH );
	_workDir = path;
	_workDir += L"\\";
	_unzipPath = _workDir + L"7z\\7za.exe";
}

void CUpdaterDlg::GetParentDir()
{
	wchar_t path[MAX_PATH] = L"";

	HMODULE thisModule = nullptr;
	VERIFY( GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR) & __FUNCTION__, &thisModule ) );
	VERIFY( GetModuleFileName( thisModule, path, MAX_PATH ) );

	::PathCchRemoveFileSpec( path, MAX_PATH );
	::PathCchRemoveFileSpec( path, MAX_PATH );

	_workDir = path;
	_workDir += L"\\";
}

int DownloadProgress( void * ptr, curl_off_t TotalToDownload, curl_off_t NowDownloaded, curl_off_t /*TotalToUpload*/, curl_off_t /*NowUploaded*/ )
{
	if( TotalToDownload <= 0.0 )
	{
		return 0;
	}

	const int totalSteps = 100;

	double fractiondownloaded = (double)NowDownloaded / (double)TotalToDownload;

	int currentStep = (int)(fractiondownloaded * totalSteps + 0.5);

	CUpdaterDlg * dlg = (CUpdaterDlg *)ptr;
	if( dlg )
	{
		dlg->SetDownloadStep( currentStep );
	}

	// if you don't return 0, the transfer will be aborted - see the documentation
	return 0;
}

void CUpdaterDlg::SetDownloadStep( int percent )
{
	_progress1.SetPos( percent );
}

void CUpdaterDlg::SetUnpackStep()
{
	_progress2.StepIt();
}

bool CUpdaterDlg::DownloadUpdate()
{
	VC_TRY;

	CWaitCursor waiting;

	_progress1.SetRange( 0, 100 );

	SetInstallationStep( 1 );

	CStringA curlQuery( downloadUrl );

	Curl curl( 0, false, "VideoCat Updater" ); // no timeout
	CurlBuffer chunk;

	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, curlQuery.GetString() ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );	
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_NOPROGRESS, FALSE ) ); // Internal CURL progressmeter must be disabled if we provide our own callback
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_XFERINFOFUNCTION, DownloadProgress ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_XFERINFODATA, this ) );

	//curl.InitHeaders();

	CURLcode code = curl_easy_perform( curl );
	if( CURLE_OK != code )
	{
		CStringA errMsg;
		errMsg.Format( "CURL error:  %s", curl_easy_strerror( code ) );
		AfxMessageBox( CString( errMsg ), MB_OK );
		return false;
	}

	chunk.TerminateBuffer();

	const char * ptr = (char *)chunk.GetData();
	if( !ptr )
		return false;

	std::ofstream fout( _updateZipPath.GetString(), std::ios_base::binary );
	fout.write( (const char*) chunk.GetData(), chunk.GetSize() );

	return true;

	VC_CATCH( ... )
	{
		AfxMessageBox( L"Curl error", MB_OK | MB_TOPMOST );
		return false;
	}
}

bool CUpdaterDlg::UnpackUpdate()
{
	VC_TRY;
	CWaitCursor waiting;

	SetInstallationStep( 2 );

	wchar_t cmdLine[2048];
	wsprintf( cmdLine, L"%s x -aoa -y -bb0 -bd -o\"%s\" \"%s\"", _unzipPath.GetString(), unzipDir.GetString(), _updateZipPath.GetString() );

	PROCESS_INFORMATION piProcInfo;
	ZeroMemory( &piProcInfo, sizeof( PROCESS_INFORMATION ) );
	STARTUPINFO siStartInfo;
	ZeroMemory( &siStartInfo, sizeof( STARTUPINFO ) );
	siStartInfo.cb = sizeof( STARTUPINFO );

	BOOL bSuccess = CreateProcess(
		_unzipPath.GetString(),
		cmdLine,
		nullptr,          // process security attributes 
		nullptr,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,             // creation flags 
		nullptr,          // use parent's environment 
		_workDir.GetString(),          // use parent's current directory 
		&siStartInfo,
		&piProcInfo
	);

	if( !bSuccess )
		return false;

	::WaitForSingleObject( piProcInfo.hProcess, INFINITE );

	::DeleteFile( _updateZipPath.GetString() );

	return true;

	VC_CATCH( ... )
	{
		return false;
	}
}

bool CUpdaterDlg::InstallUpdate()
{
	VC_TRY;

	CWaitCursor waiting;

	SetInstallationStep( 3 );

	Sleep( 2000 );// на всякий случай пару секунд ждём, чтобы старый апдейтер точно успел выключиться и перезаписаться новым

	std::error_code errCode;

	std::filesystem::path copyFrom( _workDir.GetString() );
	std::filesystem::path copyTo( installPath.GetString() );

	if( !std::filesystem::exists( copyTo, errCode ) )
	{
		bool result = std::filesystem::create_directory( copyTo, errCode );
		if( !result && errCode )
		{
			CString m( CStringA( errCode.message().c_str() ) );
			CString msg;
			msg.Format( L"Install path: %s \r\n Create directory error: %s", installPath.GetString() , m.GetString() );
			AfxMessageBox( msg, MB_OK | MB_TOPMOST );
			return false;
		}
	}

	std::filesystem::copy( copyFrom, copyTo, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, errCode );
	if( errCode )
	{
		CString m( CStringA( errCode.message().c_str() ) );
		CString msg;
		msg.Format( L"Copy error: %s", m.GetString() );
		AfxMessageBox( msg, MB_OK | MB_TOPMOST );
		return false;
	}

	CString whatsNew = installPath + L"what's new.txt";

	::ShellExecute( nullptr, L"open", whatsNew, nullptr, nullptr, SW_SHOWNORMAL );

	return true;

	VC_CATCH( ... )
	{
		AfxMessageBox( L"Copy exception", MB_OK );
		return false;
	}
}
