// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"

#include "ManageCollectionsDlg.h"

void ManageCollections( CommandInfo * info )
{
	VC_TRY;

	if( !info->doc )
		return;

	ManageCollectionsDlg dlg( info->doc );
	dlg.DoModal();

	VC_CATCH_ALL;
}
