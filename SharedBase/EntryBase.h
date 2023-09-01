#pragma once

#include <string>
#include "Genre.h"
#include "EntryAdditionalData.h"

// идентификатор фильма. Может быть ID Кинопоиска или другим идентификатором в зависимости от используемого плагина
using FilmId = unsigned int;

// упакованная информация о номере сезона и эпизоде ( 0x00FF0000 - номер сезона, 0x0000FFFF - номер эпизода )
using SeriesData = unsigned int;

enum : unsigned int
{
	NO_FILM_ID = 0,
	NO_SERIES_DATA = 0 /*нет номера сезона-эпизода*/
};

// Описание фильма (реального на диске или виртуального/онлайн) или папки (папка может содержать все те же данные, что и фильм, но не может быть открыта в плеере, т.к. путь указывает на папку)
#pragma pack (push, 8)
class EntryBase
{
public:
	std::wstring title;	///< Основное название фильма (по умолчанию - русское)
	std::wstring titleAlt;	///< Альтернативное название фильма (оригинальное для иностранных фильмов)

	std::wstring tagline;	///< слоган фильма
	std::wstring description;	///< описание сюжета фильма
	std::wstring comment;	///< пользовательский комментарий

	std::wstring filePath;	///< Путь до файла/папки от корневой директории
	std::wstring urlLink;	///< связанная страница, если есть (может быть задана как для реального фильма, так и для виртуального/онлайн)

	std::vector<EntryAdditionalData> additional; ///< дополнительные данные

	Genres genres;	///< жанры фильма

	FilmId filmId;	///< идентификатор фильма
	SeriesData seriesData; //</ упакованный сезон и эпизор сериала

	unsigned short year;	///< год выпуска фильма
	unsigned char rating;	///< рейтинг фильма

public:
	EntryBase() noexcept
		: filmId( NO_FILM_ID )
		, seriesData( NO_SERIES_DATA )
		, genres( EmptyGenres )
		, year( 0 )
		, rating( 0 )
	{
	}
};
#pragma pack (pop)
