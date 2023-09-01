#pragma once

#include <map>
#include "femstore.h"

namespace FileStore
{
	template< class K, class T, class P, class A >
	void Storage( Writer & w, ::std::map<K, T, P, A> & object, unsigned /*version*/ )
	{
		const unsigned size = (unsigned)object.size();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		for( typename ::std::map<K, T, P, A>::iterator ii = object.begin(); ii != object.end(); ++ii )
		{
			w & *ii;
		}
	}

	template< class K, class T, class P, class A >
	void Storage( Reader & r, ::std::map<K, T, P, A> & object, unsigned /*version*/ )
	{
		object.clear();

		unsigned size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		for( unsigned i = 0; i < size; ++i )
		{
			::std::pair<K, T> newObject;

			r & newObject;

			object.insert( newObject );
		}
	}
}
