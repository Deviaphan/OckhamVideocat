#pragma once

#include "CommandManager.h"

class CVideoCatDoc;
class Entry;
class DisplayItem;

class CommandInfo
{
public:
	CommandInfo()
		: doc( nullptr )
		, entry( nullptr )
		, displayItem( nullptr )
	{
	}

public:
	CString dbID; // идентификатор базы данных (если он нужен для команды)
	CVideoCatDoc * doc; // идентификатор документа для доступа к БД и окнам. Нужен всегда.
	Entry * entry; // запись БД, к которой применяется команда
	DisplayItem * displayItem; // выводимый на экран объект, содержащий загруженный в память постер
};

using ProcessCommandFunction = void (*)(CommandInfo *);
