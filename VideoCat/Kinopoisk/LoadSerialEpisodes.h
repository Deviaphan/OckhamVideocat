#pragma once

#include <map>
#include <string>
#include "KinopoiskInfo.h"

struct Episode
{
	CString nameRU;
	CString nameEN;
	int episode;
};

typedef std::map< unsigned int, std::map<unsigned int, Episode> > SeriesDescription;

void LoadSerialEpisodes( KinopoiskId serialID, SeriesDescription & description );
