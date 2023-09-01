// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "HexToUnicode.h"

wchar_t Hex( char c )
{
	if( c >= '0' && c <= '9' )
		return (c - '0');

	return c - 'a' + 10;
}

void HexToUnicode( const char * ptr, const char * endPtr, std::wstring & output )
{
	output.clear();

	while( ptr < endPtr )
	{
		if( *ptr == '\\' )
		{
			if( *(ptr + 1) == 'u' )
			{
				//считываем hex представление кода символа
				wchar_t u = 0;

				u = Hex( *(ptr + 2) );
				u <<= 4;

				u |= Hex( *(ptr + 3) );
				u <<= 4;

				u |= Hex( *(ptr + 4) );
				u <<= 4;

				u |= Hex( *(ptr + 5) );

				output += u;

				ptr += 6;
			}
			else if( *(ptr + 1) == 'n' || *(ptr + 1) == 'r' )
			{
				// \n\r | \r\n
				output += L'\n';
				ptr += 4;
			}
			else
			{
				output += L'\\';
				++ptr;
			}
		}
		else
		{
			output += (wchar_t)*ptr;
			++ptr;
		}
	}
}
