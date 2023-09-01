#pragma once

#include <map>
#include <vector>
#include "CommandID.h"
#include "CommandInfo.h"
#include "../IconManager.h"

class CommandName
{
public:
	CommandName()
		: id( CommandID::None )
	{
	}

	CommandName( const CString & t, CommandID i )
		: title( t )
		, id( i )
	{
	}

public:
	CString title;
	CommandID id;
};

class CommandExecute
{
public:
	static CommandExecute & Instance();

	class CommandData
	{
	public:
		CommandData()
			: processCommand( nullptr )
			, iconID( 0 )
		{
		}

	public:
		ProcessCommandFunction processCommand;
		IconID iconID;
		CString commandTitle;
	};

public:
	void Add( CommandID id, ProcessCommandFunction processCommand, IconID iconID, const CString & title );

	IconID GetIcon( CommandID id );
	const CString & GetTitle( CommandID id );

	const CommandData & GetCommand( CommandID id );

	void Process( CommandID id, CommandInfo * info );

	void GetAllCommands( std::vector<CommandName> & allCommands );

private:
	CommandExecute();

	CommandExecute( const CommandExecute & ) = delete; // без реализации
	CommandExecute & operator = ( const CommandExecute & ) = delete; //без реалиации


private:

	std::map<CommandID, CommandData> _functions;
};

