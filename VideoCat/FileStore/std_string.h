#pragma once

#include <string>
#include "femstore.h"

namespace FileStore
{
	template< class T1, class TTraits, class TAlloc >
	void Storage( ::FileStore::Writer & w, ::std::basic_string< T1, TTraits, TAlloc > & str, unsigned /*version*/ )
	{
		// Количество символов в строке
		// таких длинных описаний всё-равно нет, но до 1 байта уменьшить нельзя
		unsigned short size = (unsigned short)str.length();
		w.Write( size );

		WRITE_FEM_CONTAINER_FLAG( w );

		if( size > 0 )
		{
			// запись всех символов, не включая нуль терминант
			w.WriteBytes( std::data( str ), size * sizeof( T1 ) );
		}
	}

	template< class T1, class TTraits, class TAlloc >
	void Storage( ::FileStore::Reader & r, ::std::basic_string< T1, TTraits, TAlloc > & str, unsigned /*version*/ )
	{
		unsigned short size = 0;
		r.Read( size );

		READ_FEM_CONTAINER_FLAG( r );

		if( size > 0 )
		{
			// Такой способ записи в строку не очень кроссплатформенно-безопасный, но самый эффективный.
			// Нуль терминант НЕ перезаписывается!
			//
			// При необходимости можно переписать чтение с промежуточным буфером.
			str.resize( size ); // выделяется память под stringLength символов + нуль терминант
			r.ReadBytes( str.data(), size * sizeof( T1 ) );
		}
		else
		{
			str.clear();
		}
	}
}
