#pragma once
#include <utility>

namespace FileStore
{
	template< class Archive, class T1, class T2 >
	void Storage( Archive & a, ::std::pair<T1, T2> & object, unsigned /*version*/ )
	{
		a & object.first & object.second;
	}
}
