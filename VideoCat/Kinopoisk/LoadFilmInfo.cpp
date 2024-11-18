// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "LoadFilmInfo.h"
#include <vector>
#include "../CollectionDB.h"
#include "Net/CurlBuffer.h"
#include "../ErrorDlg.h"
#include "../PosterDownloader.h"
#include "../ProgressDlg.h"
#include "../ResString.h"
#include "../SerialFillDlg.h"
#include "../WaitDlg.h"
#include "../SynchronizeFiles.h"
#include "../resource.h"
#include "Kinopoisk.h"
#include "LoadSerialEpisodes.h"

namespace Traverse
{
	struct GetAllNotFilled : public Base
	{
		std::vector<EntryHandle> allFilms;	// Все фильмы без KinopoiskId и папки сериалов
		std::vector<EntryHandle> allFolders;	// Все папки без постеров
		std::vector<EntryHandle> allSeries;	// Все сериалы

		virtual Result Run( Entry & entry ) override
		{
			if( Test( entry.flags, EntryTypes::TVseries) )
			{
				// Папку сериала добавляем в фильмы, но внутрь папки не заходим. Сериалы обрабатываются индивидуально
				if( entry.filmId == NO_FILM_ID )
				{
					allFilms.push_back( entry.thisEntry );
					allSeries.push_back( entry.thisEntry );
				}

				return GOBACK;
			}

			if( Test( entry.flags, EntryTypes::TVseason ) )
			{
				// папку сезона сразу пропускаем
				return GOBACK;
			}

			if( entry.IsFolder() )
			{
				if( entry.posterId == DefaultPosterID )
					allFolders.push_back( entry.thisEntry );

				return CONTINUE;
			}

			if( (entry.filmId == NO_FILM_ID) && (entry.pluginID == PluginNone) )
				allFilms.push_back( entry.thisEntry );

			return CONTINUE;
		}
	};

	struct GetAllEpisodes : public Base
	{
		std::vector<EntryHandle> allEpisodes;

		virtual Result Run( Entry & entry ) override
		{
			if( entry.IsFile() )
			{
				allEpisodes.push_back( entry.thisEntry );
			}

			return CONTINUE;
		}
	};


	struct MarkManual : public Base
	{
		std::vector<EntryHandle> allEpisodes;
		int numMarked = 0;

		virtual Result Run( Entry & entry ) override
		{
			if( entry.pluginID == (PluginID)PluginNone )
			{
				entry.pluginID = (PluginID)PluginManual;
				++numMarked;
			}

			return CONTINUE;
		}
	};

}

void ProcessTvSeries( CollectionDB * cdb, const EntryHandle & rootHandle );


void LoadFilmsInfo( CollectionDB * cdb, const EntryHandle & rootHandle )
{
	VC_TRY;

	Entry * rootEntry = cdb->FindEntry( rootHandle );
	if( !rootEntry )
		return;

	Traverse::GetAllNotFilled all;
	
	if( Test( rootEntry->flags, EntryTypes::TVseries ) )
	{
		if( rootEntry->filmId != NO_FILM_ID )
		{
			// Если сериал уже распознан, можно пронумеровать эпизоды
			ProcessTvSeries( cdb, rootHandle );
			return;
		}
	}
	else if( Test( rootEntry->flags, EntryTypes::TVseason ) )
	{
		// Если сериал уже распознан и известно, что это папка сезона, можно пронумеровать эпизоды
		ProcessTvSeries( cdb, rootHandle );
		return;
	}
	else
	{
		cdb->TraverseAll( rootHandle, &all );
	}

	const int size = (int)all.allFilms.size();

	ProgressController pc;
	pc.Create( ResString( IDS_PROGRESS_FILL_CARDS ) );
	pc.SetProgressCount( (int)all.allFilms.size() );

	PosterManager & pm = cdb->GetPosterManager();

	int founded = 0;

	if( size > 0 )
	{
		Kinopoisk & kinopoisk = GetKinopoisk();

		Curl curl(15, false);

		for( const EntryHandle & h : all.allFilms )
		{
			Entry * entry = cdb->FindEntry( h );
			if( !entry )
				continue;

			const KinopoiskInfo & info = kinopoisk.SearchInfo( entry->title.c_str(), false );

			if( info.filmID != NO_KINOPOISK_ID )
			{
				++founded;

				// сохраняем "прочую" информацию, помимо жанра
				const Genres oldGenreData = entry->genres;

				// ПОКА ЧТО ПОДДЕРЖИВАЕТСЯ ТОЛЬКО КИНОПОИСК!
				entry->pluginID = PluginKinopoisk;

				entry->filmId = (FilmId)info.filmID;
				entry->genres = info.genres;
				entry->year = info.year;
				entry->rating = info.rating;
				entry->title = info.nameRU;
				entry->titleAlt = info.nameEN;
				entry->tagline = info.tagline;
				entry->description = info.description;
				// по умолчанию ссылка на кинопоиск
				entry->urlLink = ResFormat( L"https://www.kinopoisk.ru/film/%u/", info.filmID );

				// восстанавливаем "прочую" информацию, после получения жанров (к жанрам, заданным пользователем, добавляются жанры из карточки)
				entry->genres |= oldGenreData;

				if( entry->posterId == DefaultPosterID )
				{
					Gdiplus::Bitmap * poster = nullptr;

					// адресс постера на кинопоиске. Если кинопоиск что-то поменяет, тут нужно скорректировать
					char buffer[64];
					sprintf_s( buffer, 64, "http://st.kp.yandex.net/images/film_big/%u.jpg", entry->filmId );
					std::vector<std::string> url;
					url.push_back( buffer );

					if( DownloadImage( curl, url, poster ) )
					{
						entry->posterId = pm.SaveImage( poster );
						delete poster;
					}
				}
			}

			pc.NextStep();
		}

		// для сериалов нумеруем эпизоды
		for( const EntryHandle & h : all.allSeries )
		{
			ProcessTvSeries( cdb, h );
		}
	}

	pc.Destroy();

	// для создания постеров на папки нужн опочинить поиск картинок в интернете
	//CreatePosters( all.allFolders );

	if( size > 0 )
	{
		// обновляем списки фильмов для персон, только если новые фильмы были распознаны (иначе нечего обновлять)
		cdb->UpdatePersons();

		AfxMessageBox( ResFormat( IDS_PROGRESS_AUTOFILL_STATISTIC, founded, size, (founded * 100 / size) ), MB_OK | MB_ICONASTERISK | MB_TOPMOST );
	}
	else
	{
		AfxMessageBox( L"Заполнение карточек фильмов не требуется!", MB_OK );
	}

	// принудительно выношу на передний план, чтобы случайно пользователь не пропустил диалог и не думал, что всё зависло
	if( AfxGetApp() && AfxGetApp()->GetMainWnd() && AfxGetApp()->GetMainWnd()->m_hWnd )
		::SetForegroundWindow( AfxGetApp()->GetMainWnd()->m_hWnd );

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка заполнения карточек фильмов", ignore );
	}
}

void LoadSeriesInfo( CollectionDB * cdb, const EntryHandle & rootHandle )
{
	VC_TRY;

	SerialFillDlg serialDlg;
	if( IDOK != serialDlg.DoModal() )
		return;

	NoMadalWait waiting;
	

	Entry * rootEntry = cdb->FindEntry( rootHandle );
	if( !rootEntry )
		throw (int)1;

	if( !rootEntry->IsTV() )
		return;

	FilmId serialId = rootEntry->filmId;

	if( Test(rootEntry->flags, EntryTypes::TVseason) )
	{
		// Для папки сезона выбираем родительскую папку сериала, в которой и записан идентификатор сериала
		Entry * entryParent = cdb->FindEntry( rootEntry->parentEntry );
		if( !entryParent )
			throw( int )1;
		
		serialId = entryParent->filmId;
	}

	SeriesDescription seriesDescription;
	LoadSerialEpisodes( serialId, seriesDescription );

	if( seriesDescription.empty() )
		return;

	Traverse::GetAllEpisodes all;
	cdb->TraverseAll( rootHandle, &all );

	if( serialDlg.fillType == 0 )
	{
		// нумерация в пределах сезона
		for( const EntryHandle & h : all.allEpisodes )
		{
			Entry * entry = cdb->FindEntry( h );
			if( !entry )
				continue;

			if( !Test(entry->flags, EntryTypes::TVepisode) )
				continue;

			unsigned int season = entry->GetSeason();
			unsigned int episode = entry->GetEpisode();

			auto fullSeason = seriesDescription.find( season );
			if( fullSeason == seriesDescription.end() )
				continue;

			auto episodeDesc = fullSeason->second.find( episode );
			if( episodeDesc == fullSeason->second.end() )
				continue;

			entry->title = episodeDesc->second.nameRU;
			entry->titleAlt = episodeDesc->second.nameEN;
		}
	}
	else
	{
		// сквозная нумерация между сезонами
		std::map< int, const Episode * > lineEpisodes;
		int index = seriesDescription.begin()->second.begin()->first;
		for( const auto & s : seriesDescription )
		{
			for( const auto & e : s.second )
			{
				lineEpisodes[index] = &e.second;
				++index;
			}
		}

		for( const EntryHandle & h : all.allEpisodes )
		{
			Entry * entry = cdb->FindEntry( h );
			if( !entry )
				continue;

			if( !Test(entry->flags, EntryTypes::TVepisode) )
				continue;

			unsigned int season = entry->GetSeason();
			unsigned int episode = entry->GetEpisode();

			const Episode * episodeDesc = lineEpisodes[episode];
			if( !episodeDesc )
				continue;

			entry->title = episodeDesc->nameRU;
			entry->titleAlt = episodeDesc->nameEN;
			entry->SetEpisode( season, episodeDesc->episode );
		}
	}

	VC_CATCH( ... )
	{
		static bool ignore = false;
		ignore = ShowError( L"Ошибка получения информации о сериале", ignore );
	}
}


/**
Определение номера сезона и номера эпизода из имени файла

ищем шаблон sXXeXX в любом месте в имени файла
Перед сезоном обязательна буква S, перед эпизодом E в любом регистре
*/

bool Parse_SxEx( const CString & name, unsigned int & result )
{
	int season = -100;
	int episode = -100;

	const wchar_t * ptr = name.GetString();
	const wchar_t * end = ptr + name.GetLength();

	while( ptr < end )
	{
		if( (*ptr == L's' || *ptr == L'S') && iswdigit(*(ptr+1)) )
		{
			++ptr;
			season = _wtoi( ptr );
			// пропускаем цифры сезона
			while( ptr < end )
			{
				if( !::iswdigit( *ptr ) )
					break;
				++ptr;
			};
			
			if( (*ptr == L'e' || *ptr == L'E') && iswdigit( *(ptr + 1) ) )
			{
				++ptr;
				episode = _wtoi( ptr );

				season &= 0xFF;
				episode &= 0xFFFF;
				result = (season << 16) | episode;

				return true;
			}
		}

		++ptr;
	}
	
	return false;
}

unsigned int ParseSeasonFromName( const CString & name )
{
	unsigned int result = 0;
	if( Parse_SxEx( name, result ) )
		return result;
	else
		return 0;
}


void ProcessTvSeries( CollectionDB * cdb, const EntryHandle & rootHandle )
{
	Entry * rootEntry = cdb->FindEntry( rootHandle );
	if( !rootEntry )
		return;

	// должен быть папкой сериала или сезона
	if( !rootEntry->IsTV() || !rootEntry->IsFolder() )
		return;
	
	for( const EntryHandle & h : rootEntry->childs )
	{
		Entry * entry = cdb->FindEntry( h );
		if( !entry )
			continue;

		if( Test(entry->flags, EntryTypes::TVseason) )
		{
			ProcessTvSeries( cdb, h );
		}
		else
		{
			entry->seriesData = ParseSeasonFromName( GetNameFromFile( entry->filePath.c_str() ) );
		}
	}
}

int MarkAsUserDefinedInfo( CollectionDB * cdb, const EntryHandle & rootHandle )
{
	Traverse::MarkManual markManual;

	cdb->TraverseAll( rootHandle, &markManual );

	return markManual.numMarked;
}
