#pragma once
#include "KinopoiskInfo.h"

bool LoadKinopoiskInfo( KinopoiskId id, KinopoiskInfo & info, const CStringA & proxy );
bool LoadKinopoiskInfoDirect( KinopoiskId id, KinopoiskInfo & info  );

bool ProcessFilm( KinopoiskId id, const std::wstring & unicode, KinopoiskInfo & info );
bool ProcessFilmJson( KinopoiskId id, const char * ptr, const char * endPtr, KinopoiskInfo & info );
