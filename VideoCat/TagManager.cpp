// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "TagManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


TagManager::TagManager()
	: _nextId(0)
{}

const TagManager::TagName & TagManager::GetTag( TagId id ) const
{
	static TagName none;

	auto ii = _tags.find( id );
	if( ii != _tags.end() )
		return ii->second;
	
	return none;
}

TagId TagManager::GetTag( const TagName & tag ) const
{
	TagId id = EMPTY_TAG;

	for( const auto & tagItem : _tags )
	{
		if( tagItem.second == tag )
			return tagItem.first;
	}

	return id;
}

std::pair<TagId, TagId> TagManager::RenameTag( TagId id, const TagName & newTag )
{
	std::pair<TagId, TagId> changeTags( EMPTY_TAG, EMPTY_TAG );

	const TagId existTag = GetTag( newTag );
	if( existTag == EMPTY_TAG )
	{
		_tags[id] = newTag;
		return changeTags;
	}

	CString msg;
	msg.Format( L"Метка с именем '%s' уже существует.\r\nВыполнить объединение меток?", newTag.c_str() );
	if( IDNO == AfxMessageBox( msg, MB_YESNO ) )
		return changeTags;

	changeTags.first = id;
	changeTags.second = existTag;

	DeleteTag( id );

	return changeTags;
}

TagId TagManager::AddTag( const TagName & newTag )
{
	TagId oldId = GetTag( newTag );
	if( oldId != EMPTY_TAG )
		return oldId;

	++_nextId;
	_tags.emplace( _nextId, newTag );
	return _nextId;
}

void TagManager::DeleteTag( TagId id )
{
	_tags.erase( id );
}

TagId TagManager::FindTag( const TagName & tagName )
{
	for( AllTags::const_iterator ii = _tags.begin(); ii != _tags.end(); ++ii )
	{
		if( ii->second == tagName )
			return ii->first;
	}

	return EMPTY_TAG;
}
