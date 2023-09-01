#pragma once
#include "TraverseCollection.h"

namespace Traverse
{
	struct CountViewedFilms : public Traverse::Base
	{
		unsigned totalFilms = 0;
		unsigned viewed = 0;

		Traverse::Result Run( Entry & entry ) override
		{
			if( !entry.IsFolder() )
			{
				++totalFilms;
				if( entry.numViews > 0 )
					++viewed;
			}

			return Traverse::CONTINUE;
		}
	};

}