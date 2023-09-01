// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "About.h"

class CommandInfo;

void AboutVideocat( CommandInfo * )
{
	VC_TRY;

	CAboutDlg dlg;
	dlg.DoModal();

	VC_CATCH_ALL;
}
