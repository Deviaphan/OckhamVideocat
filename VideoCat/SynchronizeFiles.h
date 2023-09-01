#pragma once

#include <vector>
#include "Entry.h"
#include "CollectionDB.h"

extern const wchar_t DVD_IFO_FILE_NAME[];

CString GetNameFromFile( const wchar_t * path );

class SynchronizeFiles
{
public:
	explicit SynchronizeFiles( CollectionDB * db );

	bool Process( const Entry & startFolder );

private:
	bool ProcessFolder( const Entry & startFolder, unsigned rootPathLength );
	
private:
	CollectionDB * _db;
	const Entry * _rootEntry;
	const Entry * _startEntry;

public:
	std::vector<Entry> newFiles; // список новых файлов, которых не было в исходном дереве файлов

	BOOL autoFill;
	BOOL fillTechInfo;
	BOOL generateThumbs;
	PluginID fillPlugin;
};

bool IsSerial( const CString & fileName );


class FindGuard
{
public:
	explicit FindGuard( HANDLE & h )
		: _file( h )
	{}

	~FindGuard()
	{
		Close();
	}

	void Close()
	{
		if( _file != INVALID_HANDLE_VALUE )
		{
			::FindClose( _file );
			_file = INVALID_HANDLE_VALUE;
		}
	}

private:
	HANDLE & _file;
};
