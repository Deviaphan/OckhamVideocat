// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandExecute.h"

CommandExecute & CommandExecute::Instance()
{
	static CommandExecute ce;
	return ce;
}

CommandExecute::CommandExecute()
{}

void CommandExecute::Add( CommandID id, ProcessCommandFunction processCommand, IconID iconID, const CString & title )
{
#ifndef NDEBUG
	// нельзя задавать два обработчика одной команде
	if( _functions.find( id ) != _functions.end() )
		DebugBreak();

	if( id == CommandID::None )
		DebugBreak();
#endif

	CommandData data;
	data.processCommand = processCommand;
	data.iconID = iconID;
	data.commandTitle = title;

	_functions.emplace( id, data );
}

IconID CommandExecute::GetIcon( CommandID id )
{
	auto func = _functions.find( id );
	if( func != _functions.end() )
	{
		return func->second.iconID;
	}

#ifndef NDEBUG
	DebugBreak();
#endif

	return IconManager::ProgramIcon;
}

const CString & CommandExecute::GetTitle( CommandID id )
{
	auto func = _functions.find( id );
	if( func != _functions.end() )
	{
		return func->second.commandTitle;
	}

#ifndef NDEBUG
	DebugBreak();
#endif

	throw std::exception( "No title" );

	__assume(false);
}

const CommandExecute::CommandData & CommandExecute::GetCommand( CommandID id )
{
	auto func = _functions.find( id );
	if( func != _functions.end() )
	{
		return func->second;
	}

#ifndef NDEBUG
	DebugBreak();
#endif

	throw std::exception( "No command data" );

	__assume(false);
}

void CommandExecute::Process( CommandID id, CommandInfo * info )
{
	if( id == CommandID::None )
	{
#ifndef NDEBUG
		DebugBreak();
#endif
		return;
	}

	if( !info )
	{
#ifndef NDEBUG
		DebugBreak();
#endif
		// в принципе, для некоторых команд info не испоьзуется. Но для общности лучше всегда требовать его задание
		return;
	}

	auto func = _functions.find( id );

	if( func == _functions.end() )
	{
#ifndef NDEBUG
		DebugBreak();
#endif
		throw std::exception( "Unknown command" );

		return;
	}

	func->second.processCommand( info );
}

void CommandExecute::GetAllCommands( std::vector<CommandName> & allCommands )
{
	allCommands.clear();

	for( const auto & command : _functions )
	{
		allCommands.emplace_back( command.second.commandTitle, command.first );
	}
}
