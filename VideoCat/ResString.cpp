// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ResString.h"

CString ResString( UINT id )
{
	CString string;
	string.LoadString( id );

	return string;
}

CString ResFormat( UINT nFormatID, ... )
{
	CString strFormat;
	strFormat.LoadString( nFormatID );

	CString formated;

	va_list argList;
	va_start( argList, nFormatID );
	formated.FormatV( strFormat, argList );
	va_end( argList );

	return formated;
}

CString ResFormat( const wchar_t * pszFormat, ... )
{
	CString s;

	ATLASSERT( ATL::AtlIsValidString( pszFormat ) );

	va_list argList;
	va_start( argList, pszFormat );
	s.FormatV( pszFormat, argList );
	va_end( argList );

	return s;
}
