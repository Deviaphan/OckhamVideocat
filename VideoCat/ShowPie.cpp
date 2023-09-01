// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ShowPie.h"

#include "CollectionDB.h"
#include "Cyrillic.h"
#include <fstream>
#include <string.h>
#include "Traverse/TraverseCollection.h"
#include "resource.h"

void LoadChartHeader( std::string & chartHeader )
{
	HMODULE module = GetModuleHandle( nullptr );
	if( !module )
		return;

	HRSRC resource = FindResource( module, MAKEINTRESOURCE( IDR_CHART_TEMPLATE ), L"TEXT" );
	if( !resource )
		return;

	HGLOBAL global = LoadResource( module, resource );
	if( !global )
		return;

	char * charArray = (char*)LockResource( global );
	if( charArray )
		chartHeader = charArray;

	FreeResource( global );
}

const wchar_t chartColor[][8] =
{
	L"#FF1A00",
	L"#FF6600",
	L"#D1C200",
	L"#47C247",
	L"#7AAFFF",
	L"#0066FF",
	L"#9900FF"
};


void ShowGenrePie( const CollectionDB * cdb )
{
	VC_TRY;
	if( !cdb )
		return;

	CWaitCursor waiting;

	struct CalcGenres : public Traverse::Base
	{
		std::vector<unsigned> numGenres;
		int totalFilms;

	public:
		CalcGenres()
			: totalFilms(0)
		{
			numGenres.resize( (int)Genre::NumOfGenres );
		}

		virtual Traverse::Result Run( Entry & entry ) override
		{
			if( entry.IsFolder() )
				return Traverse::CONTINUE;

			++totalFilms;

			for( int genre = (int)Genre::FirstGenre; genre < (int)Genre::NumOfGenres; ++genre )
			{
				if( Test( entry.genres, (Genre)genre ) )
					++numGenres[genre];
			}

			return Traverse::CONTINUE;
		}
	};

	CalcGenres allGenres;

	cdb->ForEach( &allGenres );

	if( allGenres.totalFilms <= 0 )
		return;

	std::string chartHeader;
	LoadChartHeader( chartHeader );
	if( chartHeader.empty() )
		return;

	CString statisticPath;
	{
		wchar_t tempPath[MAX_PATH];
		::GetTempPath( MAX_PATH, tempPath );
		statisticPath = tempPath;
		statisticPath += L"VideoCat-genres-chart.html";
	}

	{
		std::ofstream fout( statisticPath.GetString(), std::ios_base::binary );

		fout << chartHeader;
		fout << Encode( std::wstring( L"<body><div id = \"title\"><b>Всего фильмов в коллекции : " ) )
			<< allGenres.totalFilms
			<< "</b></div><div class=\"container\"><div id=\"container\"><canvas id = \"chart\" width=\"700\" height=\"700\"></canvas><table id=\"chartData\">";
		fout << Encode( std::wstring( L"<tr><th>Жанр</th><th>Фильмов</th></tr>" ) );

		const wchar_t tr_template[] = L"<tr style=\"color: %s\"><td>%s</td><td>%i</td></tr>";

		const int numColors = _countof( chartColor );

		for( int i = 0; i < (int)Genre::NumOfGenres; ++i )
		{
			if( allGenres.numGenres[i] <= 0 )
				continue;

			int colorIndex = i % numColors;
			CString row;
			row.Format( tr_template, chartColor[colorIndex], Descript( (Genre)i ).GetString(), allGenres.numGenres[i] );
			fout << Encode( row );
		}

		fout << "</table></div></body></html>" << std::endl;
	}

	::ShellExecute( nullptr, L"open", statisticPath, nullptr, nullptr, SW_SHOWNORMAL );

	VC_CATCH( ... )
	{}
}


void ShowYearPie( const CollectionDB * cdb )
{
	VC_TRY;
	if( !cdb )
		return;

	CWaitCursor waiting;

	struct CalcYears : public Traverse::Base
	{
		std::map<unsigned short, unsigned int> numYears;
		int totalFilms = 0;

	public:
		virtual Traverse::Result Run( Entry & entry ) override
		{
			if( entry.IsFolder() )
				return Traverse::CONTINUE;

			if( entry.year == 0 )
				return Traverse::CONTINUE;

			++totalFilms;

			++numYears[entry.year];

			return Traverse::CONTINUE;
		}
	};

	CalcYears allYears;

	cdb->ForEach( &allYears );

	if( allYears.totalFilms <= 0 )
		return;

	std::string chartHeader;
	LoadChartHeader( chartHeader );
	if( chartHeader.empty() )
		return;

	CString statisticPath;
	{
		wchar_t tempPath[MAX_PATH];
		::GetTempPath( MAX_PATH, tempPath );
		statisticPath = tempPath;
		statisticPath += L"VideoCat-years-chart.html";
	}

	{
		std::ofstream fout( statisticPath.GetString(), std::ios_base::binary );

		fout << chartHeader;
		fout << Encode( std::wstring( L"<body><div id = \"title\"><b>Всего фильмов в коллекции : " ) )
			<< allYears.totalFilms
			<< "</b></div><div class=\"container\"><div id=\"container\"><canvas id = \"chart\" width=\"700\" height=\"700\"></canvas><table id=\"chartData\">";
		fout << Encode( std::wstring( L"<tr><th>Год</th><th>Фильмов</th></tr>" ) );

		const wchar_t tr_template[] = L"<tr style=\"color: %s\"><td>%u</td><td>%u</td></tr>";

		const int numColors = _countof( chartColor );

		int colorIndex = 0;
		for( auto info = allYears.numYears.rbegin(); info != allYears.numYears.rend(); ++info )
		{
			colorIndex = colorIndex % numColors;
			++colorIndex;

			CString row;
			row.Format( tr_template, chartColor[colorIndex], (unsigned int)info->first, info->second );
			fout << Encode( row );
		}

		fout << "</table></div></body></html>" << std::endl;
	}

	::ShellExecute( nullptr, L"open", statisticPath, nullptr, nullptr, SW_SHOWNORMAL );

	VC_CATCH( ... )
	{}
}

