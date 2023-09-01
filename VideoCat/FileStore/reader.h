#pragma once

#pragma once

#include <iostream>

#include "version.h"
#include "access.h"
#include "types.h"
#include "StoreException.h"

namespace FileStore
{
	class Reader
	{
	public:
		enum { isReader = 1, isWriter = 0 };
		enum { version = 1, _Force32 = 0x0FFFFFFF };
		const bool useClassVersion;

	public:
		Reader( ::std::istream & stream, bool useVersions = true )
			: _stream( stream )
			, useClassVersion( useVersions )
		{
			static const unsigned int name = MAKEFOURCC( '.', 'M', 'A', 'A' );

			if( useClassVersion )
			{
				unsigned int className = 0;
				Read( className );
				if( className != name )
				{
					throw Exception( "This is not MaaGames Store data file" );
				}

				unsigned char ver = 1;
				Read( ver );
			}
		}

	public:

		// сериализация указателей запрещена (чтобы не пришлось делать поддержку полиморфизма и множественную сериализацию одного объекта)
		template< typename T >
		void Read( T * )
		{
			throw Exception( "Reader doesn't support pointer deserialization" );
		}

		inline void Read( bool & object )
		{
			unsigned char boolean = 0;
			ReadBytes( &boolean, sizeof( boolean ) );
			object = boolean != 0;
		}

		// Бинарное чтение из потока встроенных типов
		template< typename T >
		inline void Read( T & object )
		{
			ReadBytes( &object, sizeof(T) );
		}

		// Бинарная запись в поток массива байт
		inline void ReadBytes( void * data, unsigned numBytes )
		{
			if( _stream.eof() )
				throw Exception( "MaaGames Store: EOF" );

			_stream.read( static_cast<char*>(data), numBytes );
		}

		template< class T >
		inline void ReadEnum( T & object, ToType<true> )
		{
			unsigned long e = 0;
			Read( e );
			object = (T)e;
		}

		template< class T >
		inline void ReadEnum( T & object, ToType<false> )
		{
			Read(object);
		}

		template< class T >
		inline void Read( T & object, ToType<true> )
		{
			ReadEnum( object, ToType< ::std::is_enum<T>::value >() );
		}

		template< class T >
		inline void Read( T & object, ToType<false> )
		{
			unsigned short version = 1;
			if( useClassVersion )
				Read( version );

			Storage( *this, object, version );
		}

	private:
		::std::istream & _stream;

	private:
		Reader( const Reader& ) = delete;
		Reader & operator = ( const Reader& ) = delete;
	};

	template< class T >
	inline Reader & operator & ( Reader & r, T & object )
	{
		r.Read( object, ToType< StoreTraits<T>::isPrimitiveType >() );
		return r;
	}

}
