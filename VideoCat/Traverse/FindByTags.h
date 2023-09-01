#pragma once
#include <vector>
#include <set>
#include "TraverseCollection.h"
#include "../TagManager.h"

namespace Traverse
{
	struct FindByTags : public Base
	{
	private:
		std::set<TagId> _searchFor;
		bool _strictCheck;

	public:
		std::vector<EntryHandle> handles;

	public:
		void SearchFor( std::set<TagId> & tags, bool strictCheck )
		{
			_searchFor.swap(tags);
			_strictCheck = strictCheck;
		}

		Result Run( Entry & entry ) override
		{
			unsigned int count = 0;

			for( TagId tag : entry.tags )
			{
				if( _searchFor.find( tag ) != _searchFor.end() )
				{
					++count;
				}
			}
			
			if( _strictCheck )
			{
				if( count == (unsigned int)_searchFor.size() )
					handles.push_back( entry.thisEntry );
			}
			else if( count )
				handles.push_back( entry.thisEntry );

			return CONTINUE;
		}
	};
}
