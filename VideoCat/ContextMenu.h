#pragma once

#include "AdvancedMenu.h"
#include "Entry.h"
#include "VideoCatDoc.h"
#include "Commands/CommandID.h"

void CreateMainMenu( adv_mfc::AdvancedMenu & menu, CVideoCatDoc* doc );

void CreateFolderMenu( adv_mfc::AdvancedMenu & menu, const Entry & entry, bool extendedMenu );

void CreateFileMenu( adv_mfc::AdvancedMenu & menu, const Entry & entry );

void CreatePersonMenu( adv_mfc::AdvancedMenu & menu, const Entry & entry );

void CreateImportExportMenu( CMenu & menu );
