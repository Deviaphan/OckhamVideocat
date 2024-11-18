// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CopyWebPageDlg.h"
#include "resource.h"
#include <string>
#include <filesystem>
#include "json/json.h"

using Microsoft::WRL::Callback;

void Test( HRESULT hr )
{
	if( !SUCCEEDED( hr ) )
	{
		if( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
		{
			AfxMessageBox( L"Не найдена совместимая версия Microsoft Edge" );
		}
		else
		{
			AfxMessageBox( L"Ошибка создания WebView окружения" );
		}
	}
}

IMPLEMENT_DYNAMIC(CopyWebPageDlg, CDialog)

CopyWebPageDlg::CopyWebPageDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_COPY_PAGE, pParent)
	, _minX( 0 )
	, _minY( 0 )
{

}

CopyWebPageDlg::~CopyWebPageDlg()
{
}

void CopyWebPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CopyWebPageDlg, CDialog)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED( IDC_COPY, &CopyWebPageDlg::OnBnClickedCopy )
	ON_WM_SIZE()
END_MESSAGE_MAP()



BOOL CopyWebPageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWaitCursor wait;

	HRESULT hresult = CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
	Test( hresult );

	InitializeWebView();

	CRect rect;
	GetWindowRect( &rect );
	_minX = rect.Width();
	_minY = rect.Height();

	//_browser.Navigate( startPageUrl, &var, nullptr, nullptr, nullptr );

	return TRUE;
}


void CopyWebPageDlg::OnOK()
{
	; // CDialog::OnOK();
}

void CopyWebPageDlg::OnBnClickedCopy()
{
	//UpdateData();
	
	//m_webView->ExecuteScript( L"alert('HTML Page Popup');", Callback<ICoreWebView2ExecuteScriptCompletedHandler>( this, &CopyWebPageDlg::ExecuteScriptComplete ).Get() );

	//m_webView->ExecuteScript( L"document.documentElement.outerHTML;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>( this, &CopyWebPageDlg::ExecuteScriptComplete ).Get() );

	wil::com_ptr<ICoreWebView2_21> webView = m_webView.try_query<ICoreWebView2_21>();
	if( webView )
	{
		HRESULT hr = webView->ExecuteScriptWithResult( L"document.body.outerHTML;", Callback<ICoreWebView2ExecuteScriptWithResultCompletedHandler>( this, &CopyWebPageDlg::ExecuteScriptComplete ).Get() );
		Test( hr );
	}
	else
	{
		//AfxMessageBox( L"try_query<ICoreWebView2_21> == null" );

		HRESULT hr = m_webView->ExecuteScript( L"document.body.outerHTML;", Callback < ICoreWebView2ExecuteScriptCompletedHandler>( this, &CopyWebPageDlg::ExScriptComplete ).Get() );
		Test( hr );
	}
	

	//std::wstring unicode;
	//unicode.resize( length * 4 );

	//const int count = MultiByteToWideChar( isWindows1251 ? 1251 : CP_UTF8, 0, htmlCode, length, &unicode[0], unicode.size() );

	//pageData = htmlCode;


	//CDialog::OnOK();
}


void CopyWebPageDlg::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	if( _minX > 0 )
	{
		lpMMI->ptMinTrackSize.x = _minX;
		lpMMI->ptMinTrackSize.y = _minY;
	}

	CDialog::OnGetMinMaxInfo( lpMMI );
}

void CopyWebPageDlg::CloseWebView()
{
	if( m_controller )
	{
		m_controller->Close();
		m_controller = nullptr;
		m_webView = nullptr;
	}

	m_webViewEnvironment = nullptr;
}

void CopyWebPageDlg::InitializeWebView()
{
	CloseWebView();

#ifdef USE_WEBVIEW2_WIN10
	m_wincompCompositor = nullptr;
#endif
	LPCWSTR subFolder = nullptr;
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	options->put_AllowSingleSignOnUsingOSPrimaryAccount( FALSE );

	auto tempPath = std::filesystem::temp_directory_path();
	tempPath.append( "VideoCat" );

	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions( subFolder, tempPath.c_str(), options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>( this, &CopyWebPageDlg::OnCreateEnvironmentCompleted ).Get() );
	Test( hr );
}

HRESULT CopyWebPageDlg::OnCreateEnvironmentCompleted( HRESULT result, ICoreWebView2Environment* environment )
{
	m_webViewEnvironment = environment;
	HRESULT hr = m_webViewEnvironment->CreateCoreWebView2Controller( this->GetSafeHwnd(), Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>( this, &CopyWebPageDlg::OnCreateCoreWebView2ControllerCompleted ).Get() );
	Test( hr );
	return S_OK;
}

HRESULT CopyWebPageDlg::OnCreateCoreWebView2ControllerCompleted( HRESULT result, ICoreWebView2Controller* controller )
{
	if( result == S_OK )
	{
		m_controller = controller;
		wil::com_ptr<ICoreWebView2> coreWebView2;
		m_controller->get_CoreWebView2( &coreWebView2 );
		coreWebView2.query_to( &m_webView );

		NewComponent<ViewComponent>(
			this, m_dcompDevice.get(),
#ifdef USE_WEBVIEW2_WIN10
			m_wincompCompositor,
#endif
			false );

		EventRegistrationToken token;
		HRESULT hr = m_webView->add_WebMessageReceived( Callback<ICoreWebView2WebMessageReceivedEventHandler>( this, &CopyWebPageDlg::WebMessageReceived ).Get(), &token );
		Test( hr );

		//EventRegistrationToken token2;
		//m_webView->add_FrameNavigationCompleted( Callback<ICoreWebView2NavigationCompletedEventHandler>( this, &CopyWebPageDlg::NavigationCompletedEventHandler ).Get(), &token2 );

		hr = m_webView->Navigate( startPageUrl );
		Test( hr );
		if( hr == S_OK )
		{
			ResizeEverything();
		}
	}
	else
	{
		Test( result );
	}
	return S_OK;
}

void CopyWebPageDlg::ResizeEverything()
{
	CWnd * placeholder = this->GetDlgItem( IDC_WEB_PLACEHOLDER );
	if( !placeholder )
		return;

	CRect availableBounds = { 0 };
	placeholder->GetWindowRect( &availableBounds );
	ScreenToClient( availableBounds );

	// ClientToScreen(&availableBounds);

	if( auto view = GetComponent<ViewComponent>() )
	{
		view->SetBounds( availableBounds );
	}
}

void CopyWebPageDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize( nType, cx, cy );

	ResizeEverything();
}

HRESULT CopyWebPageDlg::WebMessageReceived( ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args )
{
	LPWSTR pwStr;
	args->TryGetWebMessageAsString( &pwStr );
	CString receivedMessage = pwStr;
	if( !receivedMessage.IsEmpty() )
	{
		AfxMessageBox( L"This message came from Javascript : " + receivedMessage );
	}
	return S_OK;
}

//HRESULT CopyWebPageDlg::NavigationCompletedEventHandler( ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args )
//{
//	//OnBnClickedCopy();
//
//	//wil::com_ptr<ICoreWebView2_21> webView = m_webView.try_query<ICoreWebView2_21>();
//	//if( webView )
//	//{
//	//	return webView->ExecuteScriptWithResult( L"document.body.outerHTML;", Callback<ICoreWebView2ExecuteScriptWithResultCompletedHandler>( this, &CopyWebPageDlg::ExecuteScriptComplete ).Get() );
//	//}
//
//	return S_OK;
//}

HRESULT CopyWebPageDlg::ExecuteScriptComplete( HRESULT error, ICoreWebView2ExecuteScriptResult* result )
{
	LPWSTR str = nullptr;
	BOOL value = FALSE;
	HRESULT hr = result->TryGetResultAsString( &str, &value );
	Test( hr );

	pageData = str;

	CDialog::OnOK();
	return S_OK;
}

HRESULT CopyWebPageDlg::ExScriptComplete( HRESULT errorCode, LPCWSTR result )
{
	Test( errorCode );

	const int length = lstrlenW( result );
	std::vector<char> utf8;
	utf8.resize( length * 4 );
	const int utf8Size = WideCharToMultiByte( CP_UTF8, 0, result, length, std::data( utf8 ), utf8.size(), nullptr, nullptr );
	if( utf8Size < 0 )
		return -1;

	utf8[utf8Size] = '\0';

	Json::Value root;
	Json::Reader r;
	r.parse( utf8.data(), utf8.data() + utf8Size, root, false );
	const auto str = root.asString();

	std::vector<wchar_t> wstr( utf8Size + 1 );
	const int unicodeSize = MultiByteToWideChar( CP_UTF8, 0, str.data(), str.length(), std::data( wstr ), wstr.size() );
	if( unicodeSize < 0 )
		return -1;
	wstr[unicodeSize] = L'\0';

	pageData = wstr.data();

	CDialog::OnOK();

	return S_OK;
}
