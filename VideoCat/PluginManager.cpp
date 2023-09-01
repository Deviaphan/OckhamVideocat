// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "PluginManager.h"


PluginManager & PluginManager::Instance()
{
	static PluginManager pm;

	return pm;
}

void PluginDll::Initialize( HINSTANCE dll )
{
	_dll = dll;

	_IsVideoCatPlugin = (IsVideoCatPlugin) GetProcAddress( _dll,"IsVideoCatPlugin" );
	_GetPluginID = (GetPluginID)GetProcAddress( _dll, "GetPluginID" );
	_GetPluginName = (GetPluginName)GetProcAddress( _dll, "GetPluginName" );
	_GetPluginDescription = (GetPluginDescription)GetProcAddress( _dll, "GetPluginDescription" );
	_GetSuportedOperations = (GetSuportedOperations)GetProcAddress( _dll, "GetSuportedOperations" );
	_LoadFilmInformation = (LoadFilmInformation)GetProcAddress( _dll, "LoadFilmInformation" );
}

void PluginDll::FreePlugin()
{
	::FreeLibrary( _dll );
}


void PluginManager::UnloadDll( PluginID id )
{
	for( auto ii = _plugins.begin(); ii != _plugins.end(); ++ii )
	{
		if( ii->id == id )
		{
			ii->FreePlugin();

			_plugins.erase( ii );

			return;
		}
	}
}
