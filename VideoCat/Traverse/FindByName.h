#pragma once
#include <vector>
#include "../Cyrillic.h"
#include "TraverseCollection.h"

namespace Traverse
{
	struct FindByName : public Base
	{
	private:
		std::vector<CString> _searchFor;

	public:
		std::vector<EntryHandle> handles;

	public:
		void SearchFor( const CString & name, bool fullEqual )
		{
			if( fullEqual )
			{
				_searchFor.push_back( name );
				ToLower( _searchFor[0] );
				_searchFor[0].Replace( L'ё', L'е' );
				return;
			}

			// разбиваем строку поиска на слова (не менее трёх символов)
			// находим фильмы, содержащие хотя бы одно из этих слов

			CString word;
			const int size = name.GetLength();
			for( int i = 0; i < size; ++i )
			{
				if( iswalnum( name[i] ) )
				{
					word += name[i];
				}
				else
				{
					if( !word.IsEmpty() && (word.GetLength() >= 3) )
					{
						ToLower( word );
						word.Replace( L'ё', L'е' );
						_searchFor.push_back( word );
						word.Empty();
					}
				}
			}

			if( !word.IsEmpty() && (word.GetLength() >= 3) )
			{
				ToLower( word );
				word.Replace( L'ё', L'е' );
				_searchFor.push_back( word );
			}
		}

		Result Run( Entry & entry ) override
		{
			for( const CString & s : _searchFor )
			{
				CString title = entry.title.c_str();
				ToLower( title );
				title.Replace( L'ё', L'е' );

				if( title.Find( s ) != -1 )
				{
					handles.push_back( entry.thisEntry );
					return CONTINUE;
				}

				title = entry.titleAlt.c_str();
				ToLower( title );

				if( title.Find( s ) != -1 )
				{
					handles.push_back( entry.thisEntry );
					return CONTINUE;
				}
			}

			return CONTINUE;
		}
	};
}
