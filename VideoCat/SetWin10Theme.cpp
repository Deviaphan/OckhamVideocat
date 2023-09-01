// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "SetWin10Theme.h"

void SetWin10Theme( HWND wnd, bool dark )
{
	SetWindowTheme( wnd, dark ? L"DarkMode_Explorer" : L"Explorer", nullptr );

	//static DWORD AppsUseLightTheme = -1;
	//if( AppsUseLightTheme == -1 )
	//{
	//	AppsUseLightTheme = 1;

	//	CRegKey key;
	//	if( ERROR_SUCCESS != key.Open( HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", KEY_READ ) )
	//		return;

	//	if( ERROR_SUCCESS != key.QueryDWORDValue( L"AppsUseLightTheme", AppsUseLightTheme ) )
	//		return;
	//}

	//SetWindowTheme( wnd, AppsUseLightTheme ? L"Explorer" : L"DarkMode_Explorer", nullptr );
}
