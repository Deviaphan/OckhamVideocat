// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"

#include "VideoCatDoc.h"
#include "VideoTreeView.h"
#include "OnscreenKeyboardDlg.h"

// заполнение эпизодов сериалов
void OskFindByName( CommandInfo * info )
{
	VC_TRY;

	if( !info->doc )
		return;

	OnscreenKeyboardDlg osk;
	if( IDOK != osk.DoModal() )
		return;

	const CString searchThis = osk.GetText();

	info->doc->RebuildFilteredFiles( searchThis, FALSE );

	VC_CATCH_ALL;
}
