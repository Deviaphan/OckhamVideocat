#pragma once
#include <set>
#include "femstore.h"

namespace FileStore
{
	template< class K, class P, class A >
	void Storage( Writer & w, ::std::set<K, P, A> & object, unsigned /*version*/ )
	{
		const unsigned size = (unsigned)object.size();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		for( typename ::std::set<K, P, A>::iterator ii = object.begin(); ii != object.end(); ++ii )
		{
			w & *ii;
		}
	}

	template< class K, class P, class A >
	void Storage( Reader & r, ::std::set<K, P, A> & object, unsigned /*version*/ )
	{
		object.clear();

		unsigned size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		for( unsigned i = 0; i < size; ++i )
		{
			typename ::std::set<K, P, A>::value_type newObject;

			r & newObject;

			object.insert( newObject );
		}
	}
}
