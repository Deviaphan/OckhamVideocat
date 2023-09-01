#pragma once

class Entry;
class CVideoCatDoc;

class ICardPageBase
{
public:
	ICardPageBase()
		: initialized(false)
		, entry( nullptr )
		, doc( nullptr )
	{}

	virtual ~ICardPageBase()
	{}

public:
	virtual void ReinitTab()
	{
		initialized = true;
	}

	virtual bool SaveData() = 0;

protected:
	bool initialized;

public:
	Entry * entry;
	CVideoCatDoc * doc;

};
