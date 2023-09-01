// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeName.h"
#include "afxdialogex.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(ThemeName, CDialogEx)

ThemeName::ThemeName(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_THEME_NAME, pParent)
	, themeName()
{

}

ThemeName::~ThemeName()
{
}

void ThemeName::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_THEME_NAME, themeName );
}


BEGIN_MESSAGE_MAP(ThemeName, CDialogEx)
END_MESSAGE_MAP()



void ThemeName::OnOK()
{
	UpdateData();

	if( !themeName.IsEmpty() )
		__super::OnOK();
}
