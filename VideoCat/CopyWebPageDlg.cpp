// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CopyWebPageDlg.h"
#include "mshtml.h"
#include "resource.h"
#include <string>

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
	DDX_Control( pDX, IDC_EXPLORER, _browser );
}


BEGIN_MESSAGE_MAP(CopyWebPageDlg, CDialog)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED( IDC_COPY, &CopyWebPageDlg::OnBnClickedCopy )
END_MESSAGE_MAP()



BOOL CopyWebPageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWaitCursor wait;

	CRect rect;
	GetWindowRect( &rect );
	_minX = rect.Width();
	_minY = rect.Height();

	VARIANT var;
	var.vt = VT_I4;
	var.intVal = navNoHistory | navNoReadFromCache;

	//std::string ua = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)";

	//UrlMkSetSessionOption( URLMON_OPTION_USERAGENT_REFRESH, nullptr, 0, 0 );
	//UrlMkSetSessionOption( URLMON_OPTION_USERAGENT, (LPVOID)ua.c_str(), ua.size(), 0 );

	//_browser.Navigate2( startPageUrl, &var, nullptr, nullptr, &vheaders );
	_browser.Navigate( startPageUrl, &var, nullptr, nullptr, nullptr );

	return TRUE;
}


void CopyWebPageDlg::OnOK()
{
	; // CDialog::OnOK();
}

void CopyWebPageDlg::OnBnClickedCopy()
{
	UpdateData();

	//const CString locationURL = _browser.get_LocationURL();

	CComPtr<IDispatch> idoc( _browser.get_Document() );

	CComPtr<IHTMLDocument2> htmlDoc;
	HRESULT hresult = idoc->QueryInterface( IID_IHTMLDocument2, (void **)&htmlDoc );
	if( FAILED( hresult ) )
		return;
	
	BSTR htmlCharset = nullptr;
	hresult = htmlDoc->get_charset( &htmlCharset );
	if( FAILED( hresult ) )
		return;

	pageCharset = htmlCharset;
	SysFreeString( htmlCharset );

	CComPtr<IHTMLElement> htmlBody;
	hresult = htmlDoc->get_body( &htmlBody );
	if( FAILED( hresult ) )
		return;

	CComPtr<IHTMLElement> bodyParent; // чтобы получить весь код страницы, а не только тэг body
	hresult = htmlBody->get_parentElement( &bodyParent );
	if( FAILED( hresult ) )
		return;

	BSTR htmlCode = nullptr;
	bodyParent->get_outerHTML( &htmlCode );

	//htmlBody->get_outerHTML( &htmlCode );

	//const bool isWindows1251 = pageCharset.CompareNoCase( "windows-1251" ) == 0;

	//const int length = wcslen( htmlCode );

	//std::wstring unicode;
	//unicode.resize( length * 4 );

	//const int count = MultiByteToWideChar( isWindows1251 ? 1251 : CP_UTF8, 0, htmlCode, length, &unicode[0], unicode.size() );

	pageData = htmlCode;
	SysFreeString( htmlCode );


	CDialog::OnOK();
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

