// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "KeySerialize.h"
#include <exception>

void KeySerialize::Serialize()
{
	for(unsigned char & i : buffer)
	{
		i = (unsigned char)(rand() / 128);
	}

	_ptr = buffer;
	_end = _ptr + 256;

	// мусор 4 байта
	_ptr += 4;

	// идентификатор файла ключа
	WriteHeader( "VideoCat" );

	*_ptr = (unsigned char)licenseType;
	++_ptr;

	WriteString( email.GetString() );
	WriteString( name.GetString() );
}

bool KeySerialize::Deserialize()
{
	email.Empty();
	name.Empty();
	licenseType = 0;

	_ptr = buffer;
	_end = _ptr + 256;

	_ptr += 4; // в первых 4 байтах сохранён мусор

	char fileType[16];
	ReadHeader( fileType );
	if( strcmp( (char*)fileType, "VideoCat" ) != 0 )
		return false;

	licenseType = *_ptr;
	++_ptr;

	wchar_t readBuffer[128];
	
	ReadString( readBuffer );
	email = readBuffer;

	ReadString( readBuffer );
	name = readBuffer;

	return true;
}

void KeySerialize::WriteHeader( const char * str )
{
	const int length = strlen( str );
	if( length >= 16 )
		throw std::exception();

	*_ptr = (unsigned char)length;
	++_ptr;

	unsigned char * end = _ptr + length;
	while( _ptr < end )
	{
		*_ptr = *str;
		++_ptr;
		++str;
	}
}

void KeySerialize::ReadHeader( char * str )
{
	const unsigned int length = *_ptr;
	if( length !=  strlen("VideoCat") )
		throw std::exception();

	++_ptr;

	unsigned char * end = _ptr + length;

	while( _ptr < end )
	{
		*str = *_ptr;
		++_ptr;
		++str;
	}
	*str = '\0';
}

void KeySerialize::WriteString( const wchar_t * str )
{
	const int numBytes = wcslen( str ) * 2;
	if( numBytes > 120 )
		throw std::exception();

	*_ptr = (unsigned char)numBytes;
	++_ptr;

	unsigned char * end = _ptr + numBytes;
	unsigned char * bs = (unsigned char*)str;
	while( _ptr < end )
	{
		*_ptr = *bs;
		++_ptr;
		++bs;
	}
}

void KeySerialize::ReadString( wchar_t * str )
{
	const int numBytes = *_ptr;
	if( numBytes > 120 )
		throw std::exception();

	++_ptr;

	unsigned char * end = _ptr + numBytes;
	unsigned char * bs = (unsigned char*)str;
	while( _ptr < end )
	{
		*bs = *_ptr;
		++_ptr;
		++bs;
	}
	
	str[numBytes / 2] = L'\0';
}

void ReadBuffer( const CImage & image, unsigned char buffer[256] )
{
	if( image.IsNull() )
		return;

	const unsigned char * bits = (const unsigned char *)image.GetBits();

	int pitch = image.GetPitch();

	int index = 0;

	bits += pitch * 8;

	for( int y = 0; y < 16; ++y )
	{
		for( int x = 0; x < 16; ++x )
		{
			buffer[index] = bits[x * 16+8];
			++index;
		}

		bits += pitch*16;
	}
}
