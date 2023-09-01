// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "PosterDownloader.h"

#include <string>

#include "json/json.h"

#include "GlobalSettings.h"
#include "poster.h"
#include "Cyrillic.h"
#include "BitmapTools.h"


std::string DecorateRequest( const std::string & request )
{
	//	"Query='постер "
	//std::string fullRequest = "https://api.datamarket.azure.com/Bing/Search/Image?Query=%27%D0%BF%D0%BE%D1%81%D1%82%D0%B5%D1%80%20";
	std::string fullRequest;
	return fullRequest;
}

void GetImageUrl( CURL * curlHandle, const CString & request, std::vector<std::string> & imageUrls )
{
	//VC_TRY;

	return;
	/*
	if( !curlHandle )
		return;

	const std::string fullRequest = ::DecorateRequest( EncodeClean( request.GetString() ) );

	CurlBuffer chunk;

	VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_URL, fullRequest.c_str() ) );
	VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );

	VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_WRITEDATA, &chunk ) );
	VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC ) );

	//VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_USERPWD, GetGlobal().GetBingId()) );
	
	VerifyCurl( curl_easy_perform( curlHandle ) );

	chunk.TerminateBuffer();

	const char * json = (const char*)chunk.GetData();

	Json::Value root;
	Json::Reader reader;
	reader.parse( json, json + chunk.GetSize(), root );
	if( !root["d"].isObject() )
	{
		return;
	}
	
	Json::Value results = root["d"]["results"];
	if( !results.isArray() )
		return;

	const Json::ArrayIndex numResults = results.size();
	for( Json::ArrayIndex index = 0; index < numResults; ++index )
	{
		if( !results[index]["Thumbnail"].isObject() )
			continue;

		const std::string url = results[index]["Thumbnail"]["MediaUrl"].asString();
		if( !url.empty() )
			imageUrls.push_back( url );
	}

	VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_HTTPAUTH, CURLAUTH_NONE ) );

	VC_CATCH( ... )
	{
		// отключаем авторизацию
		VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_HTTPAUTH, CURLAUTH_NONE ) );
	}
	*/
}

BOOL DownloadImage( CURL * curlHandle, const std::vector<std::string> & imageUrls, Gdiplus::Bitmap * & image )
{
	VC_TRY;

	delete image;
	image = nullptr;

	if( !curlHandle )
		return FALSE;

	for( auto & imageUrl : imageUrls )
	{
		CurlBuffer chunk;

		VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_URL, imageUrl.c_str() ) );
		VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_WRITEFUNCTION, CurlBuffer::WriteMemoryCallback ) );
		VerifyCurl( curl_easy_setopt( curlHandle, CURLOPT_WRITEDATA, &chunk ) );

		if( CURLE_OK != curl_easy_perform( curlHandle ) )
			continue;

		// картинки может не быть или переадресация или ещё по какой-то причине не качается. Считаем, что меньше 4 килобайт - не картинка
		if( chunk.GetSize() < 1024 * 4 )
			continue;

		chunk.TerminateBuffer();

		std::shared_ptr<Gdiplus::Bitmap> tempImage( LoadBitmap( chunk.GetData(), chunk.GetSize() ) );
		if( !tempImage )
			continue;

		const BOOL resizeResult = ResizePoster( tempImage.get(), image );

		return resizeResult;
	}

	return FALSE;

	VC_CATCH( ... )
	{
		return FALSE;
	}
}

BOOL ResizePoster( Gdiplus::Bitmap * inputPoster, Gdiplus::Bitmap * & outputPoster )
{
	VC_TRY;

	const int th = inputPoster->GetHeight();
	const int tw = inputPoster->GetWidth();
	const double aspect = (double)th / (double)tw;

	int h = 200;
	int w = (int)((double)h / aspect);
	int srcX = 0;
	int srcW = tw;
	if( w > 160 )
	{
		w = 160;
		srcW = (int)((double)th / (200.0 / 160.0));
		srcX = (tw - srcW) / 2;
	}

	delete outputPoster;
	outputPoster = new Gdiplus::Bitmap( w, h, PixelFormat32bppPARGB );

	std::unique_ptr<Gdiplus::Graphics> memDC( Gdiplus::Graphics::FromImage( outputPoster ) );
	memDC->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic );
	memDC->DrawImage( inputPoster, Gdiplus::Rect(0,0,w,h), srcX, 0, srcW, th, Gdiplus::UnitPixel );

	Gdiplus::Pen pen( Gdiplus::Color( 32, 32, 32 ), 1.0f );
	memDC->DrawRectangle( &pen, 0, 0, w - 1, h - 1 ); // нужно на 1 пиксель уменьшить, чтобы влезло в видимые границы

	return TRUE;

	VC_CATCH( ... )
	{
		return FALSE;
	}
}
