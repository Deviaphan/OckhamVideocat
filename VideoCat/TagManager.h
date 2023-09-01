#pragma once

#include <map>
#include <string.h>
#include "FileStore/access.h"
#include "FileStore/std_map.h"
#include "FileStore/std_string.h"

// идентификатор метки
using TagId = unsigned int;
enum: unsigned int
{ EMPTY_TAG = 0 };

// Менеджер меток
class TagManager
{
public:
	using TagName = std::wstring;

	using AllTags = std::map<TagId, TagName>;

public:
	TagManager();

public:
	const TagName & GetTag( TagId id ) const;
	TagId GetTag( const TagName & tag ) const;

	std::pair<TagId, TagId> RenameTag( TagId id, const TagName & newTag );
	TagId AddTag( const TagName & newTag );

	void DeleteTag( TagId id );

	TagId FindTag( const TagName & tagName );

	const AllTags & GetAllTags() const
	{
		return _tags;
	}

private:
	AllTags _tags;
	TagId _nextId;

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & _nextId;
		ar & _tags;
	}
};
