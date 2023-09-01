#pragma once

#include <vector>
#include <map>
#include "FileStore/femstore.h"
#include "FileStore/std_map.h"
#include "FileStore/std_vector.h"
#include <set>

using PosterID = unsigned int;

#define DefaultPosterID PosterID(0)
#define IgnoredPosterID PosterID(0xFFffFFff)

class PosterManager
{
public:
	struct ImageData
	{
		std::vector<unsigned char> bytes;

	private:
		friend class FileStore::Access;
		template<class Store>
		void Storage( Store & ar, unsigned /*version*/ )
		{
			ar & bytes;
		}
	};

public:
	PosterManager();

	void Unload();

	void LoadPoster( PosterID id, Gdiplus::Bitmap * & image );
	PosterID SaveImage( Gdiplus::Bitmap * image );
	void RemovePoster( PosterID id );

	bool SaveToFile( PosterID id, const CString & path, unsigned int & crc32 ) const;

	void EncodeBase64( PosterID id, std::string & base64 );

	int Maintenance( const std::set<PosterID> & usedPosters );

private:

	std::map< PosterID, ImageData > _database;

	PosterID _nextID;


private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & _nextID;
		ar & _database;
	}

};
