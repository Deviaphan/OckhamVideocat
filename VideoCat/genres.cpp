// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "genres.h"
#include <map>

namespace
{
	std::map< Genre, CString > g_genreMap;

	void InitGenreMap( )
	{
		g_genreMap;

#define ADD_DESC( ID, NAME ) g_genreMap.emplace( ID, NAME )

		ADD_DESC( Genre::Action,	L"боевик" );
		ADD_DESC( Genre::Adventures,	L"приключения" );
		ADD_DESC( Genre::Anime,		L"аниме" );
		ADD_DESC( Genre::AnimatedSeries,	L"мультсериал" );
		ADD_DESC( Genre::Biography,	L"биография" );
		ADD_DESC( Genre::Cartoon,	L"мультфильм" );
		ADD_DESC( Genre::Children,	L"детский");
		ADD_DESC( Genre::Comedy,	L"комедия" );
		ADD_DESC( Genre::Concert,	L"концерт" );
		ADD_DESC( Genre::Crime,		L"криминал" );
		ADD_DESC( Genre::Detective,	L"детектив" );
		ADD_DESC( Genre::Documentary,	L"документальный" );
		ADD_DESC( Genre::Drama,		L"драма" );
		ADD_DESC( Genre::Erotica,	L"эротика" );
		ADD_DESC( Genre::Family,	L"семейный" );
		ADD_DESC( Genre::Fantasy,	L"фэнтези" );
		ADD_DESC( Genre::Fantastic,	L"фантастика" );
		ADD_DESC( Genre::FilmNoir,	L"фильм - нуар" );
		ADD_DESC( Genre::History,	L"исторический" );
		ADD_DESC( Genre::Horrors,	L"ужасы" );
		ADD_DESC( Genre::Melodrama,	L"мелодрама" );
		ADD_DESC( Genre::Military,	L"военный" );
		ADD_DESC( Genre::Music,		L"музыка" );
		ADD_DESC( Genre::Musical,	L"мюзикл" );
		ADD_DESC( Genre::Mysticism,	L"мистика" );
		ADD_DESC( Genre::News,		L"новости" );
		ADD_DESC( Genre::Porn,		L"для взрослых" );
		ADD_DESC( Genre::RealTV,	L"реальное ТВ" );
		ADD_DESC( Genre::Series,	L"телесериал" );
		ADD_DESC( Genre::Science,	L"научный" );
		ADD_DESC( Genre::Short,		L"короткометражка" );
		ADD_DESC( Genre::Sport,		L"спорт" );
		ADD_DESC( Genre::TalkShow,	L"ток - шоу" );
		ADD_DESC( Genre::Thriller,	L"триллер" );
		ADD_DESC( Genre::Western,	L"вестерн" );

		ADD_DESC( Genre::ArtHouse, L"арт-хаус" );
		ADD_DESC( Genre::Catastrophe, L"катастрофа" );
		ADD_DESC( Genre::Tutorial, L"туториал" );
	}
}

const CString & Descript( Genre genreId )
{
	if( g_genreMap.empty() )
		InitGenreMap();

	return g_genreMap[genreId];
}

Genre Find( const CString & genreName )
{
	if( g_genreMap.empty() )
		InitGenreMap();

	for( auto & ii : g_genreMap )
	{
		if( ii.second == genreName )
			return ii.first;
	}

	return Genre::None;
}
