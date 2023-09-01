#pragma once

#include <array>
#include "femstore.h"

namespace FileStore
{
	template< class T, unsigned SIZE >
	void Storage( Writer & w, ::std::array<T, SIZE> & object, unsigned /*version*/ )
	{
		const unsigned size = (unsigned)object.size();
		w.Write( size );

		if constexpr( StoreTraits<T>::isPrimitiveType )
		{
			// примитивные типы можно сериализовать более эффективно
			w.WriteBytes( (const char *)::std::data(object), size * sizeof( T ) );
		}
		else
		{
			for( ::std::array<T, SIZE>::iterator ii = object.begin(); ii != object.end(); ++ii )
			{
				w & *ii;
			}
		}
	}

	template< class T, unsigned SIZE >
	void Storage( Reader & r, ::std::array<T, SIZE> & object, unsigned /*version*/ )
	{
		unsigned size = 0;
		r.Read( size );

		if( size != object.size() )
			throw Exception( "std::array sizes not compatible" );

		if constexpr( StoreTraits<T>::isPrimitiveType )
		{
			// примитивные типы можно десериализовать более эффективно
			r.ReadBytes( (char*)::std::data(object), size * sizeof( T ) );
		}
		else
		{
			for( ::std::array<T, SIZE>::iterator ii = object.begin(); ii != object.end(); ++ii )
			{
				r & *ii;
			}
		}
	}
}
