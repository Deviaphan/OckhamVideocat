// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "HashItem.h"

#pragma warning( push )
#pragma warning( disable:4244 )
#pragma warning( disable:4245 )
#pragma warning( disable:4293 )
#include <boost/crc.hpp>

typedef boost::crc_optimal<64, 0x5FB7D03C81AE5243, 0, 0, true, true> crc_64_type;


PathHashType HashingPath( const wchar_t * path, unsigned length )
{
	crc_64_type crc64;
	crc64.process_bytes( path, length * sizeof( wchar_t ) );
	return crc64.checksum();
}

#pragma warning(pop)
