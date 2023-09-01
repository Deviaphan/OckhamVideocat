#pragma once

#include <list>
#include "femstore.h"

namespace FileStore
{
	template< class T, class Alloc >
	void Storage( Writer & w, ::std::list<T, Alloc> & object, unsigned /*version*/ )
	{
		const unsigned size = object.size();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		for( typename ::std::list<T, Alloc>::iterator ii = object.begin(); ii != object.end(); ++ii )
		{
			w & *ii;
		}
	}

	template< class T, class Alloc >
	void Storage( Reader & r, ::std::list<T, Alloc> & object, unsigned /*version*/ )
	{
		object.clear();

		unsigned size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		for( unsigned i = 0; i < size; ++i )
		{
			T newObject;
			
			r & newObject;

			object.push_back( newObject );
		}
	}
}
