#pragma once

#include "SharedBase/PluginInterface.h"
#include <vector>

// Каждый плагин должен иметь уникальный идентификатор. Делать GUID не хочется, т.к. бльшой размер, а плагинов всё-равно будет очень мало
// 32 битного числа хватит всем. Назначать ID надо будет случайным образом

// Уникальный идентификатор плагина
using PluginID = unsigned int;

enum
{
	PluginNone = 0x00000000,
	PluginManual = 0xFFffFFff,

	PluginKinopoisk = 0x6DC41474, // kinopoisk.ru
	
	PluginKinorium = 0x7A58E0A7,	// kinorium.com
};

class PluginDll
{
public:
	PluginDll()
		: _dll( nullptr )
		, id( PluginNone )
	{
	}


public:

	void Initialize( HINSTANCE dll );
	void FreePlugin();

	IsVideoCatPlugin _IsVideoCatPlugin = nullptr;
	GetPluginID _GetPluginID = nullptr;
	GetPluginName _GetPluginName = nullptr;
	GetPluginDescription _GetPluginDescription = nullptr;
	GetSuportedOperations _GetSuportedOperations = nullptr;
	LoadFilmInformation _LoadFilmInformation = nullptr;

	PluginID id;

protected:
	HINSTANCE _dll;
};

// API, реализуемые плагином
namespace PluginType
{
	enum Enum
	{
		FillInformation = 1 << 0, // Заполнение данных о фильме
	};
}

class PluginManager
{
public:
	static PluginManager & Instance();

	void UnloadDll( PluginID id );

	CString GetName( PluginID id ) const;
	unsigned int NumOfPlugins( PluginType::Enum type ) const;

protected:
	std::vector<PluginDll> _plugins;
};

