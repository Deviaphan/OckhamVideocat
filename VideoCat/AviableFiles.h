#pragma once

#include "FileStore/access.h"
#include "FileStore/std_vector.h"

#include "Extensions.h"
#include "Cyrillic.h"

// Перечень всех поддерживаемых расширений файлов
class AviableFiles
{
public:
	// Проверить, поддерживается ли тип файла
	BOOL TestFile( const CString & filePath ) const
	{
		const int dot = filePath.ReverseFind( L'.' );
		if( dot < 1 )
			return FALSE;

		CString fileExt = filePath.Right( filePath.GetLength() - dot - 1 );
		ToLower( fileExt );

		for( auto & ext : extensions )
		{
			if( ext.extension == fileExt )
				return TRUE;
		}

		return FALSE;
	}

	// Получить плеер и командную строку для файла
	const ExtData * GetExtData( const CString & filePath ) const
	{
		const int dot = filePath.ReverseFind( L'.' );
		if( dot < 1 )
			return nullptr;

		CString fileExt = filePath.Right( filePath.GetLength() - dot - 1 );
		ToLower( fileExt );

		for( auto & ext : extensions )
		{
			if( ext.extension == fileExt )
				return &ext;
		}

		return nullptr;
	}

	const ExtData * Get3DData() const
	{
		for( auto & ext : extensions )
		{
			if( ext.extension == Film3dExtension )
				return &ext;
		}
		return nullptr;
	}

	void BuildDefaultExtensions()
	{
		extensions.clear();
		extensions.emplace_back( L"mkv" );
		extensions.emplace_back( L"avi" );
		extensions.emplace_back( L"m4v" );
		extensions.emplace_back( L"ifo" );
		extensions.emplace_back( L"mov" );
		extensions.emplace_back( L"mpeg" );
		extensions.emplace_back( L"mpg" );
		extensions.emplace_back( L"wmv" );
		extensions.emplace_back( L"mp4" );
		extensions.emplace_back( Film3dExtension );
	}

public:
	ExtensionsList extensions;

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & extensions;
	}
};
