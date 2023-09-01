// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "Translite.h"

const wchar_t tokenTransliteDelim[] = L" ,-_()[].:;~&!`+\"«»";


bool PossibleTranslite( const CString & title )
{
	// Если в названии нет ни одной русской буквы (гласной), то оно вполне может быть написано на транслите

	const wchar_t vowels[] = L"аеиоуыэюя";
	const int count = _countof( vowels ) - 1; //нуль терминант нам не нужен

	for( int i = 0; i < count; ++i )
	{
		if( title.Find( vowels[i] ) >= 0 )
			return false;
	}

	return true;
}

CString FromTranslite( const CString & title, bool ignoreUnexpected )
{
	// а б в г д е ё ж  з и й к л м н о п р с т у ф х  ц  ч  ш  щ    ъ ы ь э ю  я
	// a b v g d e e zh z i y k l m n o p r s t u f kh ts ch sh shch ' y ' e yu ya

	CString rus;

	const wchar_t * ptr = title.GetString();
	const wchar_t * last = ptr + title.GetLength();

	// конечный автомат
	while( ptr < last )
	{
		switch( *ptr )
		{
		case L'a':	rus += L'а';	break;
		case L'b':	rus += L'б';	break;
		case L'v':	rus += L'в';	break;
		case L'g':	rus += L'г';	break;
		case L'd':	rus += L'д';	break;
		case L'e':	rus += L'е';	break;
		case L'z':
			if( *(ptr + 1) == L'h' )
			{
				++ptr;
				rus += L'ж';
			}
			else
				rus += L'з';
			break;
		case L'i':	rus += L'и';	break;
		case L'j':
			switch( *(ptr + 1) )
			{
			case L'o':
				++ptr;
				rus += L'ё';
				break;
			case L'e':
				++ptr;
				rus += L'э';
				break;
			case L'u':
				++ptr;
				rus += L'ю';
				break;
			case L'a':
				++ptr;
				rus += L'я';
				break;
			default:
				rus += L'й';
			}
			break;
		case L'k':
			if( *(ptr + 1) == L'h' )
			{
				++ptr;
				rus += L'х';
			}
			else
				rus += L'к';
			break;
		case L'l':	rus += L'л';	break;
		case L'm':	rus += L'м';	break;
		case L'n':	rus += L'н';	break;
		case L'o':	rus += L'о';	break;
		case L'p':	rus += L'п';	break;
		case L'r':	rus += L'р';	break;
		case L's':
			if( *(ptr+1) == L'h' )
			{
				if( *(ptr + 2) == L'c' && *(ptr + 3) == L'h' )
				{
					ptr += 3;
					rus += L'щ';
				}
				else
				{
					++ptr;
					rus += L'ш';
				}
			}
			else
				rus += L'с';
			break;
		case L't':
			if( *(ptr + 1) == L's' )
			{
				++ptr;
				rus += L'ц';
			}
			else
				rus += L'т';
			break;
		case L'u':	rus += L'у';	break;
		case L'f':	rus += L'ф';	break;
		case L'h':	rus += L'х';	break;
		case L'x':	rus += L'х';	break;
		case L'c':
			if( *(ptr + 1) == L'h' )
			{
				++ptr;
				rus += L'ч';
			}
			else
			{
				rus += L'ц';
				return rus;
			}
			break;
		case L'w':	rus += L'щ';	break;
		case L'y':
			if( *(ptr + 1) == L'u' )
			{
				++ptr;
				rus += L'ю';
			}
			else if( *(ptr + 1) == L'a' )
			{
				++ptr;
				rus += L'я';
			}
			else if( *(ptr + 1) == L'o' )
			{
				++ptr;
				rus += L'ё';
			}
			else
				rus += L'ы';
			break;
		case L'#':	rus += L'ъ';	break;
		case L'\'':	rus += L'ь';	break;
		default:
			if( ignoreUnexpected )
			{
				rus += L' ';
			}
			else
			{
				rus.Empty();
				return rus;
			}
		}

		++ptr;
	}

	return rus;
}
