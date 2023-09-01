#pragma once

namespace FileStore
{
	// Версия объекта по умолчанию ( 1 )
	template< class T >
	struct Version
	{
		enum : int { value = 1 };
	};
}


#define FILESTORE_VERSION( CLASS_NAME, VERSION_ID )			\
namespace FileStore											\
{															\
template<>													\
struct Version< CLASS_NAME >								\
{															\
	enum : int { value = (VERSION_ID)  };					\
};															\
}

