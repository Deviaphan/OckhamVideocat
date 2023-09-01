// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "SortPredicates.h"
#include "DisplayItem.h"
#include "Cyrillic.h"

#include <filesystem>


namespace sort_predicates
{
	bool ComparePath( const std::wstring & p1, const std::wstring & p2 )
	{
		return CompareNoCase( p1, p2 ) < 0;
	}

	__declspec(noinline) bool SortByRussianName( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		// для сериалов эпизоды всегда сортируем сперва по эпизоду, а не по имени
		if( lhs._info->seriesData == rhs._info->seriesData )
		{
			const int result = CompareNoCase( lhs._info->title, rhs._info->title);
			if( result != 0 )
				return result < 0;

			return CompareNoCase( lhs._info->titleAlt, rhs._info->titleAlt ) < 0;
		}
		else
		{
			return lhs._info->seriesData < rhs._info->seriesData;
		}
	}

	__declspec(noinline) bool SortByOriginalName( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		if( lhs._info->seriesData == rhs._info->seriesData )
		{
			const int result = CompareNoCase( lhs._info->titleAlt, rhs._info->titleAlt );
			if( result != 0 )
				return result < 0;

			return CompareNoCase( lhs._info->title, rhs._info->title ) < 0;
		}
		else
		{
			return lhs._info->seriesData < rhs._info->seriesData;
		}
	}

	__declspec(noinline) bool SortByYearUp( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		// для сериалов эпизоды всегда сортируем сперва по эпизоду, а не по году
		if( lhs._info->seriesData == rhs._info->seriesData )
		{
			if( lhsFolder || (lhs._info->year == rhs._info->year) )
			{
				const int result = CompareNoCase( lhs._info->title, rhs._info->title );
				if( result != 0 )
					return result < 0;

				return CompareNoCase( lhs._info->titleAlt, rhs._info->titleAlt ) < 0;
			}

			return lhs._info->year < rhs._info->year;
		}
		else
		{
			return lhs._info->seriesData < rhs._info->seriesData;
		}
	}

	__declspec(noinline) bool SortByYearDown( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		// для сериалов эпизоды всегда сортируем сперва по эпизоду, а не по году
		if( lhs._info->seriesData == rhs._info->seriesData )
		{
			if( lhsFolder || (lhs._info->year == rhs._info->year) )
			{
				const int result = CompareNoCase( lhs._info->title, rhs._info->title );
				if( result != 0 )
					return result < 0;

				return CompareNoCase( lhs._info->titleAlt, rhs._info->titleAlt ) < 0;
			}

			return lhs._info->year > rhs._info->year;
		}
		else
		{
			return lhs._info->seriesData > rhs._info->seriesData;
		}
	}

	__declspec(noinline) bool SortByDate( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder && rhsFolder )
			return SortByRussianName( lhs, rhs );

		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		if( lhs._info->date.dwHighDateTime > rhs._info->date.dwHighDateTime )
			return true;
		if( lhs._info->date.dwHighDateTime < rhs._info->date.dwHighDateTime )
			return false;

		return lhs._info->date.dwLowDateTime < rhs._info->date.dwLowDateTime;
	}

	__declspec(noinline) bool SortByFilenameUp( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		if( lhs._info->filePath.empty() || rhs._info->filePath.empty() )
		{
			return SortByRussianName( lhs, rhs );
		}

		return ComparePath( lhs._info->filePath, rhs._info->filePath );
	}

	__declspec(noinline) bool SortByFilenameDown( const DisplayItem & lhs, const DisplayItem & rhs )
	{
		const bool lhsFolder = lhs._info->IsFolder();
		const bool rhsFolder = rhs._info->IsFolder();
		if( lhsFolder != rhsFolder )
			return lhsFolder > rhsFolder;

		if( lhs._info->filePath.empty() || rhs._info->filePath.empty() )
		{
			return SortByRussianName( lhs, rhs );
		}

		return ComparePath( lhs._info->filePath, rhs._info->filePath );
	}


	DisplayItemComparator GetPredicate( SortType type )
	{
		switch( type )
		{
			case SortYearDown:
				return sort_predicates::SortByYearDown;	break;
			case SortYearUp:
				return sort_predicates::SortByYearUp;	break;

			case SortRu:
				return sort_predicates::SortByRussianName;	break;
			case SortEn:
				return sort_predicates::SortByOriginalName;	break;

			case SortDate:
				return sort_predicates::SortByDate;	break;

			case SortFileUp:
				return sort_predicates::SortByFilenameUp;	break;
			case SortFileDown:
				return sort_predicates::SortByFilenameDown;	break;

			default:
				return nullptr;
		}
	}

	SortType GetPredicateId( DisplayItemComparator predicate )
	{
		if( predicate == sort_predicates::SortByYearDown )
			return SortYearDown;
		else if( predicate == sort_predicates::SortByYearUp )
			return SortYearUp;
		else if( predicate == sort_predicates::SortByRussianName )
			return SortRu;
		else if( predicate == sort_predicates::SortByOriginalName )
			return SortEn;
		else if( predicate == sort_predicates::SortByDate )
			return SortDate;
		else if( predicate == sort_predicates::SortByFilenameUp )
			return SortFileUp;
		else if( predicate == sort_predicates::SortByFilenameDown )
			return SortFileDown;

		return SortYearDown;
	}
}