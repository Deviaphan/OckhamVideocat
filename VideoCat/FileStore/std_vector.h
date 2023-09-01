#pragma once

#include <vector>
#include "femstore.h"

namespace FileStore
{
	template< class T, class Alloc >
	void WriteVector( Writer & w, ::std::vector<T, Alloc> & object, ToType<true> )
	{
		const unsigned size = (unsigned)object.size();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		if( size > 0 )
			w.WriteBytes( (const char *)std::data(object), size * sizeof(T) );
	}

	template< class T, class Alloc >
	void WriteVector( Writer & w, ::std::vector<T, Alloc> & object, ToType<false> )
	{
		const unsigned size = (unsigned)object.size();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		for( typename ::std::vector<T, Alloc>::iterator ii = object.begin(); ii != object.end(); ++ii )
		{
			w & *ii;
		}
	}

	template< class T, class Alloc >
	inline void Storage( Writer & w, ::std::vector<T, Alloc> & object, unsigned /*version*/ )
	{
		WriteVector( w, object, ToType<StoreTraits<T>::isPrimitiveType>() );
	}


	template< class T, class Alloc >
	void ReadVector( Reader & r, ::std::vector<T, Alloc> & object, ToType<true> )
	{
		unsigned size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		object.resize( size );

		if( size > 0 )
			r.ReadBytes( (char*)std::data(object), size * sizeof(T) );
	}

	template< class T, class Alloc >
	void ReadVector( Reader & r, ::std::vector<T, Alloc> & object, ToType<false> )
	{
		unsigned size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		object.resize( size );

		for( typename ::std::vector<T, Alloc>::iterator ii = object.begin(); ii != object.end(); ++ii )
		{
			r & *ii;
		}
	}

	template< class T, class Alloc >
	inline void Storage( Reader & r, ::std::vector<T, Alloc> & object, unsigned /*version*/ )
	{
		ReadVector( r, object, ToType<StoreTraits<T>::isPrimitiveType>() );
	}
}
