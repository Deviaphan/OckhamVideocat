#pragma once

#include <string>

void ToLower( CString & name );
void ToUpper( CString & name );

std::string EncodeClean( const std::wstring & rus );
std::string Encode( const CString & rus );
std::string Encode( const std::wstring & rus );

bool HasRus( const wchar_t * str );

int CompareNoCase( const std::wstring & str1, const std::wstring & str2 );

std::wstring ToUnicode( const std::string & ss );
std::string ToUTF8( const std::wstring & ss );
