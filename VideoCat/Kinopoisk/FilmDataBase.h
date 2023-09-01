#pragma once

#include <vector>
#include <concurrent_vector.h>
#include "KinopoiskInfo.h"

bool IsDelimiter( wchar_t c );
unsigned Tokenize( const std::wstring & str, unsigned & pos, wchar_t * result );
bool IgnoredToken( const wchar_t * token );

class FilmDb
{
public:
	using HashType = unsigned int;

	// индексы фильмов в массиве data
	using Indices = concurrency::concurrent_vector<int>;

	//Загрузить базу данных из файла video.dat
	void LoadFromFile();

	// Построить словарь _hash для получения списка фильмов по хэшу слова из названия
	void BuildHash();

	// Получить список фильмов по хэшу слова из названия
	const Indices & GetFilms( HashType hash ) const;

	static HashType HashWord( const wchar_t * oneWord, unsigned size );

public:
	std::vector<KinopoiskInfo> data;

private:
	std::vector<Indices> _hash;
};
