// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "base64.h"

// на конце нуль-терминант, поэтому 65 символов, а не 64
const char base64_chars[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

void Base64Encode( const void * binaryData, unsigned int dataLength, std::string & resultData )
{
	// В векторе добавление в конец реализовано эффективнее, чем в std::string (речь только про MSVC)
	// Оказалолсь эффективнее создавать в векторе, даже несмотря на последующее копирование в строку (на 30-50% быстрее на больших объёмах)

	std::vector<char> base64Data;
	base64Data.reserve( (dataLength * 3) / 2 ); // примерно на 30% больше получается обычно

	const unsigned char * data = reinterpret_cast<const unsigned char *>(binaryData);

	unsigned int val = 0;
	int valb = -6;

	const unsigned char * const dataEnd = data + dataLength;
	for( ; data < dataEnd; ++data )
	{
		val = (val << 8) + *data;
		valb += 8;

		for( ; valb >= 0; valb -= 6 )
		{
			const char c = base64_chars[(val >> valb) & 0x3F];
			base64Data.push_back( c );			
		}		
	}

	// добавляем оставшиеся биты
	if( valb > -6 )
	{
		base64Data.push_back( base64_chars[((val << 8) >> (valb + 8)) & 0x3F] );
	}

	// размер должен быть кратен 4
	while( base64Data.size() % 4 )
	{
		base64Data.push_back( '=' );
	}

	resultData.clear();
	resultData.assign( base64Data.data(), base64Data.size() );
}


/*
void Base64Encode_old( const void * binaryData, unsigned int dataLength, std::string & base64Data )
{
	const unsigned char * data = reinterpret_cast<const unsigned char *>(binaryData);

	int i = 0;
	int j = 0;

	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while( dataLength-- )
	{
		char_array_3[i++] = *(data++);
		if( i == 3 )
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for( i = 0; i < 4; ++i )
				base64Data += base64_chars[char_array_4[i]];

			i = 0;
		}
	}

	if( i )
	{
		for( j = i; j < 3; j++ )
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for( j = 0; (j < i + 1); j++ )
			base64Data += base64_chars[char_array_4[j]];

		while( (i++ < 3) )
			base64Data += '=';

	}
}
*/

void Base64Decode( unsigned char const * base64Data, unsigned int dataLength, std::vector<unsigned char> & binaryData )
{

	/*

	  std::string out;

	std::vector<int> T(256,-1);
	for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	int val=0, valb=-8;
	for (uchar c : in) {
		if (T[c] == -1) break;
		val = (val<<6) + T[c];
		valb += 6;
		if (valb>=0) {
			out.push_back(char((val>>valb)&0xFF));
			valb-=8;
		}
	}
	return out;
	*/
}


void Base64Fake( const void * binaryData, unsigned int dataLength, std::string & resultData )
{
	const char base64_fake[65] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+-";

	// В векторе добавление в конец реализовано эффективнее, чем в std::string (речь только про MSVC)
	// Оказалолсь эффективнее создавать в векторе, даже несмотря на последующее копирование в строку (на 30-50% быстрее на больших объёмах)

	std::vector<char> base64Data;
	base64Data.reserve( (dataLength * 3) / 2 ); // примерно на 30% больше получается обычно

	const unsigned char * data = reinterpret_cast<const unsigned char *>(binaryData);

	unsigned int val = 0;
	int valb = -6;

	const unsigned char * const dataEnd = data + dataLength;
	for( ; data < dataEnd; ++data )
	{
		val = (val << 8) + *data;
		valb += 8;

		for( ; valb >= 0; valb -= 6 )
		{
			const char c = base64_fake[(val >> valb) & 0x3F];
			base64Data.push_back( c );
		}
	}

	// добавляем оставшиеся биты
	if( valb > -6 )
	{
		base64Data.push_back( base64_fake[((val << 8) >> (valb + 8)) & 0x3F] );
	}

	// размер должен быть кратен 4
	while( base64Data.size() % 4 )
	{
		base64Data.push_back( '=' );
	}

	resultData.clear();
	resultData.assign( base64Data.data(), base64Data.size() );
}
