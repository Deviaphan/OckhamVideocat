#pragma once

#include "FileStore/femstore.h"

namespace FileStore
{

	template< class T_Char, class T_Traits >
	void Storage( ::FileStore::Writer & w, CStringT< T_Char, T_Traits > & str, unsigned /*version*/ )
	{
		// Количество символов в строке
		// таких длинных строк всё-равно нет
		unsigned short size = (unsigned short)str.GetLength();
		w.Write( size );

		if( size > 0 )
		{
			WRITE_FEM_CONTAINER_FLAG( w ); //-V571

			// запись всех символов, не включая нуль терминант
			w.WriteBytes( str.GetString(), size * sizeof( T_Char ) );
		}
	}

	template< class T_Char, class T_Traits >
	void Storage( ::FileStore::Reader & r, CStringT< T_Char, T_Traits > & str, unsigned /*version*/ )
	{
		unsigned short size = 0;
		r.Read( size );

		if( size > 0 )
		{
			READ_FEM_CONTAINER_FLAG( r ); //-V571

			T_Char * ptr = str.GetBufferSetLength( size );
			r.ReadBytes( ptr, size * sizeof( T_Char ) );
			str.ReleaseBuffer();
		}
		else
		{
			str.Empty();
		}
	}

}
