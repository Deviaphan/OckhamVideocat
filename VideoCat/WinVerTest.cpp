// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "WinVerTest.h"

bool IsWin7Plus()
{
	OSVERSIONINFOEXW osvi = { sizeof( osvi ), 0, 0, 0, 0, { 0 }, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL ),
			VER_MINORVERSION, VER_GREATER_EQUAL ),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

	osvi.dwMajorVersion = HIBYTE( _WIN32_WINNT_WIN7 );
	osvi.dwMinorVersion = LOBYTE( _WIN32_WINNT_WIN7 );
	osvi.wServicePackMajor = 0;

	return VerifyVersionInfoW( &osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask ) != FALSE;
}
