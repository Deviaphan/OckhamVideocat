#pragma once

#include <unordered_map>
#include "femstore.h"

namespace FileStore
{
	template< class K, class T, class H, class KE, class A >
	void Storage( Writer & w, ::std::unordered_map<K, T, H, KE, A> & object, unsigned /*version*/ )
	{
		const unsigned size = object.size();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		for( typename ::std::unordered_map<K, T, H, KE, A>::iterator ii = object.begin(); ii != object.end(); ++ii )
		{
			w & *ii;
		}
	}

	template< class K, class T, class H, class KE, class A >
	void Storage( Reader & r, ::std::unordered_map<K, T, H, KE, A> & object, unsigned /*version*/ )
	{
		object.clear();

		unsigned size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		for( unsigned i = 0; i < size; ++i )
		{
			::std::pair<K, T> newObject;

			r & newObject;

			object.emplace( newObject.first, newObject.second );
		}
	}
}
