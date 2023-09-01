#pragma once

#include <vector>

Gdiplus::Bitmap * CopyBitmap( Gdiplus::Bitmap * src );

int GetEncoderClsid( const wchar_t* format, CLSID* pClsid );

void SaveBitmap( const CString & fileName, Gdiplus::Bitmap * bitmap );

void SaveJpg( const CString & fileNamemm, Gdiplus::Bitmap * bitmap, ULONG quality = 75 );

Gdiplus::Bitmap * LoadBitmap( const std::vector<unsigned char> & buffer );
Gdiplus::Bitmap * LoadBitmap( const void * bytes, unsigned numBytes );

void SaveBitmap( Gdiplus::Bitmap * bitmap, ULONG quality, std::vector<unsigned char> & buffer );
