// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "TestIP.h"
#include <string>
#include "Net/CurlBuffer.h"
#include "json/json.h"

size_t FindTag( const std::wstring & str, size_t pos, const wchar_t * tag )
{
	pos = str.find( tag, pos );
	if( pos == std::wstring::npos )
		return pos;

	pos = str.find( L"ipaddress", pos );
	if( pos == std::wstring::npos )
		return pos;

	while( str[pos] != L'>' )
		++pos;
	++pos;

	return pos;
}

void ReadData( const std::wstring & str, size_t pos, CString & data )
{
	while( str[pos] != L'<' )
	{
		data += str[pos];
		++pos;
	}
}

void TestIP( unsigned timeout, const CStringA * torIP, const CStringA * proxyIP )
{
	Curl::Global cGuard;
	Curl curl( timeout );

	CurlBuffer chunk;

	VerifyCurl( curl_easy_setopt( curl, CURLOPT_URL, "https://api.myip.com/" ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
	VerifyCurl( curl_easy_setopt( curl, CURLOPT_WRITEDATA, &chunk ) );

	curl.InitHeaders();

	if( torIP )
	{
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5 ) );
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_PROXY, torIP->GetString() ) );
	}
	else if( proxyIP )
	{
		VerifyCurl( curl_easy_setopt( curl, CURLOPT_PROXY, proxyIP->GetString() ) );
	}

	if( CURLE_OK != curl_easy_perform( curl ) )
		return;

	chunk.TerminateBuffer();

	const char * ptr = (char*)chunk.GetData();
	if( !ptr )
		return;

	Json::Value root;
	{
		Json::CharReaderBuilder rb;
		std::unique_ptr<Json::CharReader> reader( rb.newCharReader() );
		Json::String err;
		reader->parse( (char*)chunk.GetData(), (char*)chunk.GetData() + chunk.GetSize(), &root, &err );
	}

	if( !root["ip"].isString() )
		return;

	std::string ip = root["ip"].asString();

	std::wstring unicode;
	unicode.resize( chunk.GetSize() * 4 );

	int count = MultiByteToWideChar( CP_UTF8, 0, ip.c_str(), ip.length(), std::data(unicode), unicode.size() );
	if( count <= 0 )
		return;
	unicode[count] = L'\0';

	CString msg;
	msg.Format( L"ip: %s", unicode.c_str() );
	AfxMessageBox( msg, MB_OK );
}
