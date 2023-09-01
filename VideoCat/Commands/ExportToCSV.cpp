// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include <fstream>
#include "CommandInfo.h"
#include "ResString.h"
#include "CollectionDB.h"
#include "Traverse/TraverseCollection.h"
#include "Cyrillic.h"
#include "VideoCatDoc.h"
#include "resource.h"

struct CollectFilms: public Traverse::Base
{
public:
	std::ostream * stream;
	const CollectionDB * cdb;

public:
	CollectFilms()
		: stream( nullptr )
		, cdb( nullptr )
	{
	}

	Traverse::Result Run( Entry & entry ) override
	{
		if( entry.IsFolder() )
			return Traverse::CONTINUE;

		const Entry * root = cdb->FindEntry( entry.rootEntry );

		(*stream) << "\"" << Encode( entry.title ) << "\",";
		(*stream) << "\"" << Encode( entry.titleAlt ) << "\",";
		(*stream) << "\"";
		if( root )
			(*stream) << Encode( root->filePath );
		(*stream) << Encode( entry.filePath ) << "\"\n";

		return Traverse::CONTINUE;
	}
};

void ExportToCSV( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;
		
	CFileDialog dlgFile( FALSE, L"*.csv", cdb->GetName(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Таблица (*.csv)|*.csv||" );

	if( IDOK != dlgFile.DoModal() )
		return;

	CWaitCursor waiting;

	std::ofstream fout( dlgFile.GetPathName().GetString() );

	CollectFilms allFilms;
	allFilms.stream = &fout;
	allFilms.cdb = cdb;

	cdb->ForEach( &allFilms );

	VC_CATCH_ALL;
}
