#pragma once

#include <set>
#include <map>
#include "TraverseCollection.h"
#include "../genres.h"
#include "../TagManager.h"

namespace Traverse
{
	struct AllGenresTags : public Base
	{
	public:
		std::map<Genre, unsigned> numGenres;
		std::map<TagId, unsigned> numTags;

	public:
		Result Run( Entry & entry ) override
		{
			for( int i = (int)Genre::FirstGenre; i < (int)Genre::NumOfGenres; ++i )
			{
				Genre genre = (Genre)i;

				if( Test( entry.genres, genre ) )
				{
					numGenres[genre]++;
				}
			}

			for( TagId tag : entry.tags )
			{
				numTags[tag]++;
			}

			return CONTINUE;
		}
	};
}
