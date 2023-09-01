// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"
#include "ProgramSettingsDlg.h"
#include "resource.h"

void ProgramSettings( CommandInfo * info )
{
	VC_TRY;

	if( !info->doc )
		return;

	CProgramSettings * ps = new CProgramSettings( info->doc );
	ps->Create( IDD_SETTINGS );
	ps->ShowWindow( SW_NORMAL );

	VC_CATCH_ALL; //-V773
}
