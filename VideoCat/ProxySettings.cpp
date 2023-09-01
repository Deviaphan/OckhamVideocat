// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ProxySettings.h"
#include "GlobalSettings.h"
#include "TestIP.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(CProxySettings, CPropertyPage)

CProxySettings::CProxySettings()
	: CPropertyPage(IDD_PROXY_SETTINGS)
	, proxyType( 0 )
	, proxyIP( 0 )
	, proxyPort( 0 )
	, timeout( 0 )
{

}

CProxySettings::~CProxySettings()
{
}

void CProxySettings::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange( pDX );
	DDX_Radio( pDX, IDC_NO_PROXY, proxyType );
	DDX_IPAddress( pDX, IDC_IP_ADDRESS, proxyIP );
	DDX_Text( pDX, IDC_IP_PORT, proxyPort );
	DDX_Text( pDX, IDC_TIMEOUT, timeout );
}


BEGIN_MESSAGE_MAP(CProxySettings, CPropertyPage)
	ON_BN_CLICKED( IDC_CHECK_IP, &CProxySettings::OnCheckIp )
END_MESSAGE_MAP()



BOOL CProxySettings::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;
}


void CProxySettings::OnCheckIp()
{
	CWaitCursor wait;

	UpdateData();

	if( (ProxyType)proxyType == ProxyType::UseProxy )
		GetGlobal().ReadProxyList();

	CStringA * torIP = nullptr;
	CStringA * pIP = nullptr;
	CStringA temp;

	switch( (ProxyType)proxyType )
	{
		case ProxyType::NoProxy:
			break;
		
		case ProxyType::UseTor:
		{
			temp = GetGlobal().GetProxy();
			torIP = &temp;
			break;
		}

		case ProxyType::UseProxy:
		{
			temp = GetGlobal().GetProxy();
			pIP = &temp;
			break;
		}
	}

	TestIP( timeout, torIP, pIP );
}
