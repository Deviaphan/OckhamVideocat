#pragma once

#include <string>
#include <vector>
#include "Net/CurlBuffer.h"

void GetImageUrl( CURL * curlHandle, const CString & request, std::vector<std::string> & imageUrls );

BOOL DownloadImage( CURL * curlHandle, const std::vector<std::string> & imageUrls, Gdiplus::Bitmap* & image );

BOOL ResizePoster( Gdiplus::Bitmap* inputPoster, Gdiplus::Bitmap * & outputPoster );
