// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "poster.h"
#include "Net/base64.h"
#include <fstream>
#include "resource.h"
#include <omp.h>
#include "BitmapTools.h"

#pragma warning( push )
#pragma warning( disable:4244 )
#pragma warning( disable:4245 )
#pragma warning( disable:4293 )
#include <boost/crc.hpp>

unsigned int PosterHash( const std::vector<unsigned char> & bytes )
{
	boost::crc_32_type crc32;
	crc32.process_bytes( std::data(bytes), bytes.size() );
	return crc32.checksum();
}

#pragma warning( pop )


PosterManager::PosterManager()
	: _nextID(1)
{
}

void PosterManager::Unload()
{
	_nextID = 1;
	_database.clear();
}


void PosterManager::LoadPoster( PosterID id, Gdiplus::Bitmap *& image )
{
	delete image;
	image = nullptr;

	auto item = _database.find( id );
	if( item != _database.end() )
	{
		image = LoadBitmap( item->second.bytes );
	}
}

PosterID PosterManager::SaveImage( Gdiplus::Bitmap * image )
{
	std::vector<unsigned char> buffer;
	SaveBitmap( image, 75, buffer );
	
	PosterID posterID = DefaultPosterID;

#pragma omp critical
	{
		posterID = _nextID++;

		_database[posterID].bytes.swap( buffer );
	}

	return posterID;
}

void PosterManager::RemovePoster( PosterID id )
{
	if( id != DefaultPosterID )
		_database.erase( id );
}

void PosterManager::EncodeBase64( PosterID id, std::string & base64 )
{
	auto item = _database.find( id );
	if( item != _database.end() )
	{
		const auto & bytes = item->second.bytes;
		Base64Encode( std::data(bytes), bytes.size(), base64 );
	}
}

bool PosterManager::SaveToFile( PosterID id, const CString & path, unsigned int & crc32 ) const
{
	auto item = _database.find( id );
	if( item == _database.end() )
		return false;

	const auto & bytes = item->second.bytes;

	const unsigned int posterCrc32 = PosterHash( bytes );

	if( (crc32 == posterCrc32) && ::PathFileExists( path ) )
	{
		return true;
	}

	std::ofstream out( path, std::ios::binary );
	out.write( (const char*)std::data(bytes), bytes.size() );

	crc32 = posterCrc32;
	return true;
}

int PosterManager::Maintenance( const std::set<PosterID> & usedPosters )
{
	std::vector<PosterID> unusedPosters;

	for( auto & ii : _database )
	{
		if( usedPosters.find( ii.first ) == usedPosters.end() )
		{
			unusedPosters.push_back( ii.first );
		}
	}

	for( const PosterID & id : unusedPosters )
	{
		_database.erase( id );
	}

	return (int)unusedPosters.size();
}
