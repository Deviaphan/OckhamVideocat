// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "TestPassword.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CTestPassword, CDialog)

CTestPassword::CTestPassword(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_TEST_PASSWORD, pParent)
	, password( _T( "" ) )
{

}

CTestPassword::~CTestPassword()
{
}

void CTestPassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_PASSWORD, password );
}


BEGIN_MESSAGE_MAP(CTestPassword, CDialog)
END_MESSAGE_MAP()


