// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "MediaInfo.h"

std::wstring g_libraryPath;

namespace media
{
	void SetMediaInfoPath( const CString & fullPath )
	{
		g_libraryPath = fullPath.GetString();
	}

	unsigned int ConvertToSeconds( const std::wstring & rhs )
	{
		unsigned time = 0;

		unsigned hour = 0;
		unsigned minutes = 0;
		unsigned sec = 0;

		const wchar_t * ptr = std::data(rhs);
		const wchar_t * end = ptr + rhs.size();

		while( ptr < end )
		{
			while( *ptr == L' ' )
				++ptr;

			if( iswdigit( *ptr ) )
			{
				time = _wtoi( ptr );
				while( iswdigit( *ptr ) )
					++ptr;
			}
			else if( *ptr == L'h' )
			{
				hour = time;
				time = 0;
				++ptr;
			}
			else if( *ptr == L'm' )
			{
				minutes = time;
				time = 0;
				ptr += 3;
			}
			else if( *ptr == L's' )
			{
				sec = time;
				time = 0;
				break;
			}
			else if( *ptr == L'\r' )
				break;
			else
				++ptr;
		}

		time = ((hour * 60) + minutes) * 60 + sec;

		return time;
	}

	unsigned int GetVideoLength( const CString & filmPath )
	{
		std::wstring inform;

		MediaInfoDLL::MediaInfo MI;
		MI.Open( filmPath.GetString() );
		{
			MI.Option( L"Complete" );
			inform = MI.Inform();
		}
		MI.Close();

		size_t offset = inform.find( L"Duration", 0 );
		if( offset == std::wstring::npos )
			return 0;

		const wchar_t * ptr = std::data(inform) + offset;
		const wchar_t * end = std::data(inform) + inform.size();
		while( ptr < end )
		{
			if( *ptr == L':' )
			{
				++ptr;
				++ptr;
				return ConvertToSeconds( ptr );
			}
			++ptr;
		}

		return 0;
	}

	CString GetMovieName( const CString & filmPath )
	{
		std::wstring inform;

		MediaInfoDLL::MediaInfo MI;
		MI.Open( filmPath.GetString() );
		{
			MI.Option( L"Complete" );
			inform = MI.Inform();
		}
		MI.Close();

		size_t offset = inform.find( L"Movie name", 0 );
		if( offset == std::wstring::npos )
			return CString();

		const wchar_t * ptr = std::data(inform) + offset + 10;
		const wchar_t * end = std::data(inform) + inform.size();
		while( ptr < end )
		{
			if( *ptr == L':' )
			{
				++ptr;
				++ptr;
				break;
			}
			++ptr;
		}

		const wchar_t * start = ptr;
		while( ptr < end && *ptr != L'\r' )
		{
			++ptr;
		}

		inform[ptr - start + offset + 10] = L'\0';
		CString result = start;
		return result;
	}

	void Info::Parse( const std::wstring & input )
	{
		_data.clear();

		std::wstring lhs;
		std::wstring rhs;

		bool useLhs = true;
		bool insertSpace = false;

		const wchar_t * ptr = std::data(input);
		const wchar_t * end = ptr + input.size();
		while( ptr < end )
		{
			if( *ptr == L'\r' )
			{
				++ptr; // \r\n

				if( !lhs.empty() )
					_data.emplace_back( lhs, rhs  );

				lhs.clear();
				rhs.clear();
				useLhs = true;
				insertSpace = false;
			}
			else if( *ptr == L':' )
			{
				++ptr; // пробел в начале
				useLhs = false;
				insertSpace = false;
			}
			else
			{
				if( useLhs )
				{
					if( *ptr != L' ' )
					{
						if( insertSpace )
						{
							insertSpace = false;
							lhs += L' ';
						}

						lhs += *ptr;
					}
					else
					{
						insertSpace = true;
					}
				}
				else
				{
					rhs += *ptr;
				}
			}

			++ptr;
		}
	}

}