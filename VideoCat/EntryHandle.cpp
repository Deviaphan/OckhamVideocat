// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "EntryHandle.h"
#include "Net/base64.h"

const EntryHandle EntryHandle::EMPTY;

inline const wchar_t * GetPrefix( EntityType type )
{
	switch( type )
	{
		case EntryHandle::IS_LINK:
			return L"L";
		case EntryHandle::IS_VIRTUAL:
			return L"V";
		case EntryHandle::IS_PERSON:
			return L"P";
		default://IS_FILE
			return L"";
	}
}

CString EntryHandle::ToString( PathHashType h, EntityType type )
{
	static const wchar_t hex[17] = L"0123456789ABCDEF";

	CString result = GetPrefix( type );

	// мне без разницы, что строка получается перевёрнутой, важно лишь идентифицировать
	for( unsigned i = 0; i < 16; ++i )
	{
		result += hex[h & 0x0F];
		h >>= 4;
	}

	return result;
}

CString EntryHandle::ToBase64( PathHashType h, EntityType type )
{
	std::string base64;

	Base64Fake( &h, sizeof( h ), base64 );

	CString result;
	result.Format( L"%s%S", GetPrefix( type ), base64.c_str() );

	return result;
}
