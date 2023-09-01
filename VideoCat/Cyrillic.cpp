// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "Cyrillic.h"
#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const char codes[] = "%D0%B0%D0%B1%D0%B2%D0%B3%D0%B4%D0%B5%D0%B6%D0%B7%D0%B8%D0%B9%D0%BA%D0%BB%D0%BC%D0%BD%D0%BE%D0%BF%D1%80%D1%81%D1%82%D1%83%D1%84%D1%85%D1%86%D1%87%D1%88%D1%89%D1%8A%D1%8B%D1%8C%D1%8D%D1%8E%D1%8F";

void ToLower( CString & name )
{
	if( !name.IsEmpty() )
	{
		::CharLowerBuff( name.GetBuffer(), name.GetLength() );
		name.ReleaseBuffer();
	}
}

void ToUpper( CString & name )
{
	if( !name.IsEmpty() )
	{
		::CharUpperBuff( name.GetBuffer(), name.GetLength() );
		name.ReleaseBuffer();
	}
}

std::string EncodeClean( const std::wstring & rus )
{
	if( rus.empty() )
		return std::string();

	// Русские буквы кодируем, английские и цифры оставляем как есть, остальные символы заменяем на пробелы

	std::string encoded;
	encoded.reserve( rus.size() * 6 );

	for(const wchar_t ru : rus)
	{
		wchar_t c = ru;
		if( c == L'ё' || c == L'Ё' )
		{
			c = L'е';
		}

		if( c >= L'а' && c <= L'я' )
		{
			const int num = c - L'а';
			const char * ptr = &codes[num * 6];

			encoded += *(ptr + 0);
			encoded += *(ptr + 1);
			encoded += *(ptr + 2);
			encoded += *(ptr + 3);
			encoded += *(ptr + 4);
			encoded += *(ptr + 5);
		}
		else if( c >= L'А' && c <= L'Я' )
		{
			const int num = c - L'А';

			const char * ptr = &codes[num * 6]; //-V557

			encoded += *(ptr + 0);
			encoded += *(ptr + 1);
			encoded += *(ptr + 2);
			encoded += *(ptr + 3);
			encoded += *(ptr + 4);
			encoded += *(ptr + 5);
		}
		else if( c >= L'A' && c <= L'Z' )
			encoded += char( ru - L'A' + 'a' );
		else if( c >= L'a' && c <= L'z' )
			encoded += char( ru - L'a' + 'a' );
		else if( c >= L'0' && c <= L'9' )
			encoded += char( ru - L'0' + '0' );
		else
		{
			switch( c )
			{
				case L'\n':
					encoded += "%0A"; break;
				case L'\r':
					break; // skip
				case L'(':
					encoded += '('; break;
				case L')':
					encoded += ')'; break;
				case L'[':
					encoded += '['; break;
				case L']':
					encoded += ']'; break;
				case L'.':
					encoded += '.'; break;
				case L',':
					encoded += "%2C"; break;
				case L'!':
					encoded += '!'; break;
				case L'?':
					encoded += "%3F"; break;
				case L'%':
					encoded += "%25"; break;

					// всё остальное заменяем пробелами
				default:
					encoded += "%20";
					break;
			}
		}
	}

	return encoded;
}

std::string Encode( const CString & rus )
{
	if( rus.IsEmpty() )
		return std::string();

	std::vector<char> utf8( rus.GetLength() * 6 );
	const int utf8Size = WideCharToMultiByte( CP_UTF8, 0, rus, rus.GetLength(), std::data(utf8), utf8.size(), nullptr, nullptr );
	utf8.resize(utf8Size);
	return std::data(utf8);
}

std::string Encode( const std::wstring & rus )
{
	if( rus.empty() )
		return std::string();

	std::vector<char> utf8( rus.size() * 6 );
	const int utf8Size = WideCharToMultiByte( CP_UTF8, 0, rus.c_str(), rus.length(), std::data(utf8), utf8.size(), nullptr, nullptr );
	utf8.resize(utf8Size);
	return std::data(utf8);
}

bool HasRus( const wchar_t * str )
{
	if( !str )
		return false;

	const wchar_t minRus = L'а';
	const wchar_t maxRus = L'я';

	while( *str )
	{
		const wchar_t c = *str;

		if( c >= minRus && c <= maxRus )
			return true;

		++str;
	}

	return false;
}


int CompareChar( wchar_t c1, wchar_t c2 )
{
	if( c1 == c2 )
		return 0;

	
	c1 = std::tolower( c1 );
	c2 = std::tolower( c2 );

	return c1 - c2;
}

void ToLower( std::wstring & str )
{
	for( auto & c : str )
	{
		c = std::tolower( c );
	}
}

int CompareNoCase( const std::wstring & str1, const std::wstring & str2 )
{
	std::wstring s1( str1 );
	ToLower( s1 );

	std::wstring s2( str2 );
	ToLower( s2 );

	int size1 = s1.size();
	int size2 = s2.size();

	return s1.compare( s2 );

	//const int size = (std::min)(str1.length(), str2.length());

	//auto p1 = str1.begin();
	//auto p2 = str2.begin();

	//for( int i = 0; i < size; ++i )
	//{
	//	int result = CompareChar( *p1, *p2 );

	//	if( result != 0 )
	//		return result;

	//	++p1;
	//	++p2;
	//}

	//return (int)str1.length()- (int)str2.length();
}

std::wstring ToUnicode( const std::string & ss )
{
	std::wstring unicode;
	unicode.resize( ss.size() * 4 );

	const int count = MultiByteToWideChar( CP_UTF8, 0, std::data( ss ), ss.size(), unicode.data(), unicode.size() );
	if( count >= 0 )
		unicode.resize(count);

	return unicode;
}

std::string ToUTF8( const std::wstring& ss )
{
	std::string utf8;
	utf8.resize( ss.size() * 6 );

	const int count = WideCharToMultiByte( CP_UTF8, 0, ss.data(), ss.size(), utf8.data(), utf8.size(), nullptr, nullptr );
	if( count >= 0 )
		utf8.resize(count);

	return utf8;
}
