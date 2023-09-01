#pragma once

#include "types.h"

namespace FileStore
{
	/// Объект, обеспечивающий хранилищу доступ к закрытым полям объекта
	class Access
	{
	public:
		template<class Archive, class Object>
		inline static void Store( Archive & store, Object & object, unsigned version )
		{
			typename RemoveConst<Object>::Type & obj = const_cast<typename RemoveConst<Object>::Type &>(object);
			obj.Storage( store, version );
		}
	};

	/// Не интрузивная работа с хранилищем.	По умолчанию вызывает интрузивный метод.
	template<class Archive, class T>
	inline void Storage( Archive & store, T & t, unsigned version )
	{
		Access::Store( store, t, version );
	}
}
