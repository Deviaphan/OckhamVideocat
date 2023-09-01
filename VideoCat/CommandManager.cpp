// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CommandManager.h"

#include "ProgramSettingsDlg.h"
#include "ThemeEditor.h"

bool AllCommandsDisabled()
{
	if( CProgramSettings::refCount > 0 )
		return true;

	if( ThemeEditor::refCount > 0 )
		return true;

	return false;
}
