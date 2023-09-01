#pragma once

namespace FileStore
{
	class Writer;
	class Reader;
}

namespace FileStore
{
	template< class T >
	class ArrayWrap
	{
	public:
		ArrayWrap( T * data, unsigned length )
			: _data(data)
			, _length(length)
		{
		}

	private:
		T * _data;
		unsigned _length;

	private:
		friend FileStore::Access;

		template< class Archive >
		void Storage( Archive & store, unsigned /*version*/ )
		{
			store & _length;

			if( _length > 0 )
			{
				for( unsigned i = 0; i < _length; ++i )
				{
					store & _data[i];
				}
			}
		}
	};
}