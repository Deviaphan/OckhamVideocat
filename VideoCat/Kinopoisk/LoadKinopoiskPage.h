#pragma once
#include "KinopoiskInfo.h"
#include <vector>

void LoadPageData( const CString& startUrl, ::std::vector<char>& utf8 );

bool LoadKinopoiskInfoDirect( KinopoiskId id, KinopoiskInfo & info  );

bool ProcessFilm( KinopoiskId id, const std::wstring & unicode, KinopoiskInfo & info );
bool ProcessFilmJson( KinopoiskId id, const char * ptr, const char * endPtr, KinopoiskInfo & info );
