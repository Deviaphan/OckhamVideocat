#pragma once


class DisplayItem;

using DisplayItemComparator = bool (*)(const DisplayItem &, const DisplayItem &);


namespace sort_predicates
{
	enum
	{
		SortDefault = 0xff,
		SortYearDown = 0,
		SortYearUp,
		SortRu,
		SortEn,
		SortDate,
		SortFileUp,
		SortFileDown,
	};

	using SortType = unsigned char;

	DisplayItemComparator GetPredicate( SortType type );
	SortType GetPredicateId( DisplayItemComparator predicate );
}
