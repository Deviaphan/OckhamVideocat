#pragma once

#include <type_traits>

#if _MSC_VER < 1600

namespace std
{
	using std::tr1::is_enum;
	using std::tr1::is_integral;
	using std::tr1::is_floating_point;
}

#endif

namespace FileStore
{
	/// Интерпретация целочисленной константы, как типа
	template< int T_Value >
	struct ToType
	{
		enum
		{
			_value = T_Value
		};
	};


	// Определение встроенных типов

	template< class T >
	struct StorePrimitive
	{
		enum
		{
			isPrimitive = std::is_enum<T>::value | std::is_integral<T>::value | std::is_floating_point<T>::value
		};
	};


#define FEMSTORE_SET_POD_TYPE( TYPE )	\
	template<> struct ::FileStore::StorePrimitive< TYPE >{ enum { isPrimitive = true }; };


	template<class T>
	struct RemovePointer
	{
		using Type = T;
	};
	template< class T>
	struct RemovePointer< T * >
	{
		using Type = T;
	};


	template< class T >
	struct RemoveReference
	{
		using Type = T;
	};
	template< class T >
	struct RemoveReference< T & >
	{
		using Type = T;
	};


	template< class T >
	struct RemoveConst
	{
		using Type = T;
	};
	template< class T >
	struct RemoveConst< const T >
	{
		using Type = T;
	};


	template< class T >
	struct StoreTraits
	{
		using CleanType = typename RemoveConst< typename RemoveReference< typename RemovePointer< T >::Type >::Type >::Type;

		enum
		{
			isPrimitiveType = StorePrimitive< CleanType >::isPrimitive
		};
	};

	enum
	{
		IS_CONTAINER = 0xDeadBeef
	}; // Флаговое значение того, что сохранён контейнер.
// Пусть будет таким значение, это не принципиально. Служит для того, чтобы при чтении битого файла уменьшить вероятность
// ошибочного создания гигантских размеров контейнеров с отжиранием всей памяти и мусором внутри.

#define WRITE_FEM_CONTAINER_FLAG( WRITER )	\
	if( (WRITER).useClassVersion && (size > 0) )	\
	{	\
		(WRITER).Write( ::FileStore::IS_CONTAINER ); \
	}

#define READ_FEM_CONTAINER_FLAG( READER )	\
	if( (READER).useClassVersion && (size > 0) )	\
	{	\
		unsigned flag(0);	\
		(READER).Read(flag);	\
		if( flag != ::FileStore::IS_CONTAINER )	\
			throw ::FileStore::Exception("File format error");	\
	}

}
