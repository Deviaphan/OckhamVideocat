#pragma once

#include <vector>
#include "FileStore/access.h"

#include "CStringStore.h"

#define Film3dExtension L"3D video"

// Расширение файла и связанные с ним параметры запуска
struct ExtData
{
	explicit ExtData( const CString & ext = CString() )
		: extension(ext)
		, ignoreIn3d( FALSE )
	{}

	CString extension;	//< расширение файла
	CString player;	///< Плеер для этого расширения. Если не задан - используется плеер, ассоциированный с расширением в ОС
	CString commandLine;	///< Командная строка для запуска. Если не задана и есть плеер, то передаётся только полный путь к файлу
	BOOL ignoreIn3d;	///< Если TRUE и файл помечен как 3D, то вместо плеера, ассоциированного с расширением, будет вызван 3D плеер.

	bool operator < (const ExtData & rhs) const
	{
		return extension < rhs.extension;
	}

	bool operator == ( const ExtData & rhs ) const
	{
		if( extension.CompareNoCase( rhs.extension ) != 0 )
			return false;

		if( player.CompareNoCase( rhs.player ) != 0 )
			return false;

		if( commandLine.CompareNoCase( rhs.commandLine ) != 0 )
			return false;

		return ignoreIn3d == rhs.ignoreIn3d;
	}


private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & extension;
		ar & player;
		ar & commandLine;
		ar & ignoreIn3d;
	}
};

// Список доступных расширений файлов и связанных с параметров запуска воспроизведения
using ExtensionsList = std::vector<ExtData>;

