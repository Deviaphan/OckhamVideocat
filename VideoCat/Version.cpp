// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "Version.h"

#include "json/json.h"
#include "Net/CurlBuffer.h"
#include "GlobalSettings.h"
#include "Cyrillic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void DownloadBuffer( CurlBuffer & chunk )
{
	Curl::Global cGuard;
	Curl curl(15, false);

	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, "http://videocat.maagames.ru/files/videocat.version" ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );
	VerifyCurl( curl_easy_perform( curl ) );

	chunk.TerminateBuffer();
}

BOOL Version::HasNewVersion()
{
	VC_TRY;

	CWaitCursor waiting;

	CurlBuffer chunk;
	DownloadBuffer( chunk );

	if( !chunk.GetData() )
		return FALSE;

	Json::Value root;
	{
		Json::CharReaderBuilder rb;
		std::unique_ptr<Json::CharReader> reader( rb.newCharReader() );
		Json::String err;
		reader->parse( (char *)chunk.GetData(), (char *)chunk.GetData() + chunk.GetSize(), &root, &err );
	}

	if( root["version"].isString() )
		version = root["version"].asCString();

	Json::Value & jsonWhatNew = root["whatnew"];
	if( jsonWhatNew.isArray() )
	{
		Json::ArrayIndex size = jsonWhatNew.size();
		for( Json::ArrayIndex i = 0; i < size; ++i )
		{
			if( jsonWhatNew[i].isString() )
			{
				whatNew += "\r\n- ";
				whatNew += ToUnicode( jsonWhatNew[i].asString() ).c_str();
			}
		}
	}

	if( root["build"].isUInt() )
		build = root["build"].asUInt();

	return build > fileVersion;

	VC_CATCH( ... )
	{
		return FALSE;
	}
}

void Version::ReadVersion()
{
	VC_TRY;

	DWORD verHandle = 0;
	const DWORD verInfoSize = GetFileVersionInfoSize( GetGlobal().programName, &verHandle );
	if( verInfoSize )
	{
		std::vector<unsigned char> buffer( verInfoSize );

		GetFileVersionInfo( GetGlobal().programName, verHandle, verInfoSize, std::data(buffer) );

		VS_FIXEDFILEINFO *fixedFileInfo = nullptr;
		UINT versionLen = 0;
		VerQueryValue( std::data(buffer), _T( "\\" ), (void**)&fixedFileInfo, (UINT *)&versionLen );

		productMS = fixedFileInfo->dwProductVersionMS;
		productLS = fixedFileInfo->dwProductVersionLS;

		fileVersion = HIWORD( fixedFileInfo->dwFileVersionMS );
	}

	VC_CATCH( ... )	{}
}

CString Version::GetVersionName() const
{
	CString productVersion;
	productVersion.Format(
		L"Номер версии: %u.%u.%u.%u",
		HIWORD( productMS ),
		LOWORD( productMS ),
		HIWORD( productLS ),
		LOWORD( productLS )
		);

	return productVersion;
}
