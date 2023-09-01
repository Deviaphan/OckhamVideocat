#pragma once
#include <cstdarg>

CString ResString( UINT id );

CString ResFormat( UINT nFormatID, ... );

CString ResFormat( const wchar_t * pszFormat, ... );
