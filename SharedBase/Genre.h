#pragma once
#include <assert.h>
#include <stdint.h>

using Genres = uint64_t;

enum { EmptyGenres = 0 };

// Величина битовых сдвигов для каждого жанра. Битовая маска задаёт несколько жанров
enum class Genre: int
{
	None = -1, // сдвинуть на столько нельзя...

	FirstGenre = 0,	// Индекс первого жанра, чтобы можно было итерировать

	Action = 0,	// боевик
	Adventures,	// приключения
	Anime,	// аниме
	AnimatedSeries,	// мультсериал
	Biography,	// биография
	Cartoon,	// мультфильм
	Children,	// детский
	Comedy,	// комедия
	Concert,	// концерт
	Crime,	// криминал
	Detective,	// детектив
	Documentary,	// документальный
	Drama,	// драма
	Erotica,	// эротика
	Family,	// семейный
	Fantasy,	// фэнтези
	Fantastic,	// фантастика
	FilmNoir,	// фильм - нуар
	History,	// исторический
	Horrors,	// ужасы
	Melodrama,	// мелодрама
	Military,	// военный
	Music,	// музыка
	Musical,	// мюзикл
	Mysticism,	// мистика
	News,	// новости
	Porn,	// порно
	RealTV,	// реальное ТВ
	Series,	// телесериал
	Science,	// научный
	Short,	// короткометражка
	Sport,	// спорт
	TalkShow,	// ток - шоу
	Thriller,	// триллер
	Western,	// вестерн

	// ФИЛЬМЫ ВНЕ ЖАНРОВ КИНОПОИСКА
	ArtHouse, // арт-хаус
	Catastrophe, // фильм-катастрофа
	Tutorial, // руководство, видео-урок

	NumOfGenres,	// Количество жанров в списке
};

inline Genres GetValue( const Genre genreId ) noexcept
{
	static_assert( (int)Genre::NumOfGenres < 64, "Превышено возможное количество жанров");

	return ((Genres)1ull) << (int)genreId;
}

inline void Set( Genres & genres, const Genre genreId ) noexcept
{
	genres |= GetValue(genreId);
}

inline void Clear( Genres & genres, const Genre genreId ) noexcept
{
	genres &= ~GetValue(genreId);
}

inline bool Test( const Genres & genres, const Genre genreId ) noexcept
{
	if( genreId != Genre::None )
		return (genres & GetValue(genreId)) != 0ull;

	return false;
}
