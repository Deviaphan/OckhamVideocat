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
		g_genreMap.emplace( Genre::Action,	L"боевик" );
		g_genreMap.emplace( Genre::Adventures,	L"приключения" );
		g_genreMap.emplace( Genre::Anime,		L"аниме" );
		g_genreMap.emplace( Genre::AnimatedSeries,	L"мультсериал" );
		g_genreMap.emplace( Genre::Biography,	L"биография" );
		g_genreMap.emplace( Genre::Cartoon,	L"мультфильм" );
		g_genreMap.emplace( Genre::Children,	L"детский");
		g_genreMap.emplace( Genre::Comedy,	L"комедия" );
		g_genreMap.emplace( Genre::Concert,	L"концерт" );
		g_genreMap.emplace( Genre::Crime,		L"криминал" );
		g_genreMap.emplace( Genre::Detective,	L"детектив" );
		g_genreMap.emplace( Genre::Documentary,	L"документальный" );
		g_genreMap.emplace( Genre::Drama,		L"драма" );
		g_genreMap.emplace( Genre::Erotica,	L"эротика" );
		g_genreMap.emplace( Genre::Family,	L"семейный" );
		g_genreMap.emplace( Genre::Fantasy,	L"фэнтези" );
		g_genreMap.emplace( Genre::Fantastic,	L"фантастика" );
		g_genreMap.emplace( Genre::FilmNoir,	L"фильм - нуар" );
		g_genreMap.emplace( Genre::History,	L"исторический" );
		g_genreMap.emplace( Genre::Horrors,	L"ужасы" );
		g_genreMap.emplace( Genre::Melodrama,	L"мелодрама" );
		g_genreMap.emplace( Genre::Military,	L"военный" );
		g_genreMap.emplace( Genre::Music,		L"музыка" );
		g_genreMap.emplace( Genre::Musical,	L"мюзикл" );
		g_genreMap.emplace( Genre::Mysticism,	L"мистика" );
		g_genreMap.emplace( Genre::News,		L"новости" );
		g_genreMap.emplace( Genre::Porn,		L"для взрослых" );
		g_genreMap.emplace( Genre::RealTV,	L"реальное ТВ" );
		g_genreMap.emplace( Genre::Series,	L"телесериал" );
		g_genreMap.emplace( Genre::Science,	L"научный" );
		g_genreMap.emplace( Genre::Short,		L"короткометражка" );
		g_genreMap.emplace( Genre::Sport,		L"спорт" );
		g_genreMap.emplace( Genre::TalkShow,	L"ток - шоу" );
		g_genreMap.emplace( Genre::Thriller,	L"триллер" );
		g_genreMap.emplace( Genre::Western,	L"вестерн" );

		g_genreMap.emplace( Genre::ArtHouse, L"арт-хаус" );
		g_genreMap.emplace( Genre::Catastrophe, L"катастрофа" );
		g_genreMap.emplace( Genre::Tutorial, L"туториал" );
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
