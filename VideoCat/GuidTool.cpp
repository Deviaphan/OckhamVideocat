// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "GuidTool.h"
#include "ErrorDlg.h"

const wchar_t guidFormat[] = L"%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X";

void GenerateGuid( GUID & guid )
{
	CoInitialize( nullptr );
	CoCreateGuid( &guid );
	CoUninitialize();
}

void GetFormattedGuid( const GUID & guid, CString & guidString )
{
	guidString.Format( guidFormat,
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
	);
}

CString GetGuidString()
{
	CString guidString;

	try
	{
		GUID guid;
		GenerateGuid( guid );

		GetFormattedGuid( guid, guidString );
	}
	catch( ... )
	{
		ShowError( L"Ошибка создания GUID", false );
	}

	return guidString;
}
