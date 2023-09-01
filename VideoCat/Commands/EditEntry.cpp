// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"

#include "CommandInfo.h"

#include "VideoCatDoc.h"
#include "EditCard.h"
#include "poster.h"
#include "DisplayItem.h"
#include "CardTechInfo.h"


// редактирование карточки реального объекта (папки/фильма)
void EditSingleEntry( CommandInfo * info, CardTabName::Enum tabIndex )
{
	VC_TRY;

	CVideoCatDoc * doc = info->doc;
	if( !doc )
		return;

	if( !info->entry )
		return;

	if( !info->displayItem )
		return;

	// персоны редактируются отдельно
	if( info->entry->thisEntry.IsPerson() )
	{
		return;
	}

	// В процессе работы МОДАЛЬНОГО диалога список отображаемых фильмов не меняется, БД фильмов не меняется, поэтому указатель остаётся корректным

	PosterID oldPosterId = info->entry->posterId;

	CEditCard dlg;
	dlg.SetDocument( doc );
	dlg.SetEntry( info->entry );
	dlg.SetDefaultTab( tabIndex );
	if( IDOK == dlg.DoModal() )
	{
		if( oldPosterId != info->entry->posterId )
		{
			Gdiplus::Bitmap * poster = nullptr;
			doc->GetCurrentCollection()->GetPosterManager().LoadPoster( info->entry->posterId, poster );

			info->displayItem->_poster.reset( poster );
		}
	}

	VC_CATCH_ALL;
}

void EditSingleEntry( CommandInfo * info )
{
	EditSingleEntry( info, CardTabName::FilmInfo );
}

void EditTagInfo( CommandInfo * info )
{
	EditSingleEntry( info, CardTabName::TagInfo );
}

void EditTechInfo( CommandInfo * info )
{
	VC_TRY;

	if( !info->entry )
		return;

	// техническая информация бывает только у реальных объектов
	if( !info->entry->thisEntry.IsReal() )
		return;

	if( info->entry->IsFolder() )
	{
		// для папок получаем техническую информацию автоматически для всех фильмов в папке

		CVideoCatDoc * doc = info->doc;
		if( !doc )
			return;

		if( ::ReadFileInfo( *doc->GetCurrentCollection(), info->entry->thisEntry ) )
		{
			doc->InvalidateDatabase();
		}
		else
		{
			CTaskDialog taskDlg(
				  L"Получение технической информации не требуется, либо невозможно получить информацию из файлов."
				, L"Не удалось получить техническую информацию"
				, L"Ошибка!"
				, TDCBF_CLOSE_BUTTON
				, 0
			);

			taskDlg.SetDialogWidth( 220 );
			taskDlg.SetMainIcon( TD_ERROR_ICON );
			taskDlg.DoModal();
		}
	}
	else
	{
		EditSingleEntry( info, CardTabName::TechInfo );
	}

	VC_CATCH_ALL;
}
