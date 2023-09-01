// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "VideoCatDoc.h"
#include "CollectionDB.h"
#include "plugin/AndroidExport.h"

void ExportToAndroid( CommandInfo * info )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	ExportForAndroid( *cdb );

	VC_CATCH_ALL;
}
