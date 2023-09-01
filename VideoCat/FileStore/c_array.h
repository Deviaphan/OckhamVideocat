#pragma once

#include "types.h"
#include "femstore_fwd.h"
#include "StoreException.h"

namespace FileStore
{
	template< class T, int SIZE >
	void Storage( Writer & w, T (&object)[SIZE], unsigned /*version*/ )
	{
		const unsigned size = (unsigned)SIZE;
		w.Write( size );

		if( StoreTraits<T>::isPrimitiveType )
		{
			// примитивные типы можно сериализовать более эффективно
			w.WriteBytes( (const char *)::std::data(object), size * sizeof( T ) );
		}
		else
		{
			for( unsigned i = 0; i < SIZE; ++i )
			{
				w & object[i];
			}
		}
	}

	template< class T, int SIZE >
	void Storage( Reader & r, T (&object)[SIZE], unsigned /*version*/ )
	{
		unsigned size = 0;
		r.Read( size );

		if( size != SIZE )
			throw Exception( "c_array sizes not compatible" );

		if( StoreTraits<T>::isPrimitiveType )
		{
			// примитивные типы можно десериализовать более эффективно
			r.ReadBytes( (char*)::std::data(object), size * sizeof( T ) );
		}
		else
		{
			for( unsigned i = 0; i < SIZE; ++i )
			{
				r & object[i];
			}
		}
	}
}
