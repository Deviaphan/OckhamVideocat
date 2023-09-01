#pragma once

#include <iostream>
#include "version.h"
#include "access.h"
#include "types.h"
#include "StoreException.h"

namespace FileStore
{
	class Writer
	{
	public:
		enum { isReader = 0, isWriter = 1 };
		enum { version = 1, _Force32 = 0x0FFFFFFF };

		const bool useClassVersion;

	public:
		explicit Writer( ::std::ostream & stream, bool useVersions = true )
			: _stream(stream)
			, useClassVersion( useVersions )
		{
			// не буду поддерживать называние и версионность хранилища
			if( useVersions )
			{
				static const unsigned int name = MAKEFOURCC( '.', 'M', 'A', 'A' );
				Write( name );

				const unsigned char ver = version;
				Write( ver );
			}
		}

	public:

		// сериализация указателей запрещена (чтобы не пришлось делать поддержку полиморфизма и множественную сериализацию одного объекта)
		template< typename T >
		void Write( const T * )
		{
			throw Exception( "Writer doesn't support pointer serialization" );
		}

		void Write( const bool object )
		{
			const unsigned char boolean = static_cast<const unsigned char>(object);
			WriteBytes( &boolean, sizeof( boolean ) );
		}

		// Бинарная запись в поток встроенных типов
		template< typename T >
		void Write( const T & object )
		{
			WriteBytes( &object, sizeof(T) );
		}

		// Бинарная запись в поток массива байт
		void WriteBytes( const void * data, unsigned numBytes )
		{
			_stream.write( static_cast<const char*>(data), numBytes );
		}

		template< class T >
		void WriteEnum(  const T & object, ToType<true> )
		{
			unsigned long e = (unsigned long)object;
			Write( e );
		}

		template< class T >
		void WriteEnum(  const T & object, ToType<false> )
		{
			Write(object);
		}

		template< class T >
		void Write(  const T & object, ToType<true> )
		{
			// перечисления всегда сохраняем как 32 битное целое, чтобы не зависить от версии компилятора и состава перечисления
			WriteEnum( object, ToType< ::std::is_enum<T>::value >() );
		}

		template< class T >
		void Write( const T & object, ToType<false> )
		{
			// отключаю версионность классов
			const unsigned short version = Version<T>::value;
			if( useClassVersion )
				Write( version );

			T & obj = const_cast<T&>(object);
			Storage( *this, obj, version );
		}

	private:
		::std::ostream & _stream;

	private:
		Writer( const Writer& ) = delete;
		Writer & operator = ( const Writer& ) = delete;
	};

	template< class T >
	inline Writer & operator & ( Writer & w, const T & object )
	{
		w.Write( object, ToType< StoreTraits<T>::isPrimitiveType >() );
		return w;
	}

}
