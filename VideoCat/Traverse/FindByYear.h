#pragma once
#include <set>
#include "TraverseCollection.h"

namespace Traverse
{
	struct FindByYear : public Base
	{
	private:
		std::set<unsigned> _years;

		bool _findLess = false;
		bool _findGreater = false;
		unsigned _cmpYear = 0;

	public:
		std::vector<EntryHandle> handles;

	public:
		void SearchFor( const CString & searchString )
		{
			unsigned year1( 0 ), year2( 0 );

			std::wstring str( searchString.GetString() );

			wchar_t token[256];
			unsigned start = 0;

			if( str[start] == L'#' )
				++start;

			if( str[start] == L'<' )
			{
				_findLess = true;
				++start;
			}
			else if( str[start] == L'>' )
			{
				_findGreater = true;
				++start;
			}

			const bool multyYear = !_findLess && !_findGreater;

			do
			{
				const unsigned tokenSize = Tokenize( str, start, token );
				if( tokenSize == 0 )
				{
					if( year1 != 0 )
						_years.insert( year1 );

					break;
				}

				// в поиске по году токены должны начинаться с цифры
				if( !iswdigit( token[0] ) )
					break;

				const int y = _wtoi( token );
				if( y <= 1800 || y >= 2100 )
				{
					if( year1 == 0 )
						break;

					_years.insert( year1 );
					break;
				}

				if( year1 == 0 )
				{
					year1 = (unsigned)y;

					while( iswspace( str[start] ) )
						++start;

					// если не тире, то добавляем один год и переходим к следующему. Если тире, то пытаемся установить диапазон лет
					if( str[start] != L'-' )
					{
						_years.insert( year1 );
						year1 = 0;
					}
				}
				else
				{
					year2 = (unsigned)y;

					if( year1 > year2 )
						std::swap( year1, year2 );

					for( ; year1 <= year2; ++year1 )
						_years.insert( year1 );

					year1 = 0;
					year2 = 0;
				}

				if( _findLess || _findGreater )
					break;
			}
			while( multyYear );

			if( !multyYear && !_years.empty() )
			{
				_cmpYear = *_years.begin();
			}
		}

		Result Run( Entry & entry ) override
		{
			if( _cmpYear )
			{
				if( _findLess )
				{
					if( entry.year && (entry.year < _cmpYear) )
						handles.push_back( entry.thisEntry );
				}
				else if( _findGreater )
				{
					if( entry.year && (entry.year > _cmpYear) )
						handles.push_back( entry.thisEntry );
				}
			}
			else if( entry.year && (_years.find( entry.year ) != _years.end()) )
			{
				handles.push_back( entry.thisEntry );
			}

			return CONTINUE;
		}
	};
}