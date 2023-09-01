#pragma once
#include <vector>
#include "../Cyrillic.h"
#include "TraverseCollection.h"
#include "../genres.h"

namespace Traverse
{
	struct FindByGenres : public Base
	{
	private:
		Genres _searchFor;
		bool _strictCheck;

	public:
		std::vector<EntryHandle> handles;

	public:
		FindByGenres()
			:_searchFor( (Genres)Genre::None )
			, _strictCheck( false )
		{
		}

		void SearchFor( Genres genres, bool strictCheck )
		{
			_searchFor = genres;
			_strictCheck = strictCheck;
		}

		Result Run( Entry & entry ) override
		{
			if( _strictCheck )
			{
				if( (entry.genres & _searchFor) == _searchFor )
					handles.push_back( entry.thisEntry );
			}
			else
			{
				if( entry.genres & _searchFor )
					handles.push_back( entry.thisEntry );
			}

			return CONTINUE;
		}
	};
}
