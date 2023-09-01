#pragma once
#include <vector>
#include <string>
#include "MediaInfoDLL.h"


namespace media
{
	void SetMediaInfoPath( const CString & fullPath );

	unsigned int GetVideoLength( const CString & filmPath );
	CString GetMovieName( const CString & filmPath );

	class Info
	{
	public:
		void Parse( const std::wstring & input );

		std::vector< std::pair<std::wstring, std::wstring> > _data;
	};

}