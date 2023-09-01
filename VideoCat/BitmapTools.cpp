// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "BitmapTools.h"
#include <memory>

#include "Net/base64.h"

int GetEncoderClsid( const wchar_t * format, CLSID * pClsid )
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::GetImageEncodersSize( &num, &size );
	if( size == 0 )
		return -1;  // Failure

	Gdiplus::ImageCodecInfo * pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc( size ));
	if( pImageCodecInfo == nullptr )
		return -1;  // Failure

	Gdiplus::GetImageEncoders( num, size, pImageCodecInfo );

	for( UINT j = 0; j < num; ++j )
	{
		if( wcscmp( pImageCodecInfo[j].MimeType, format ) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free( pImageCodecInfo );
			return j;  // Success
		}
	}

	free( pImageCodecInfo );
	return -1;  // Failure
}

Gdiplus::Bitmap * CopyBitmap( Gdiplus::Bitmap * src )
{
	if( !src )
		return nullptr;

	Gdiplus::RectF imgRect( 0.0, 0.0, src->GetWidth(), src->GetHeight() );

	Gdiplus::Bitmap * result = new Gdiplus::Bitmap( src->GetWidth(), src->GetHeight(), PixelFormat32bppPARGB );

	Gdiplus::Graphics gdi( result );

	gdi.DrawImage( src, imgRect, imgRect, Gdiplus::UnitPixel, (Gdiplus::ImageAttributes *)nullptr );

	return result;
}


void SaveBitmap( const CString & fileName, Gdiplus::Bitmap * bitmap )
{
	CLSID pngClsid;
	GetEncoderClsid( L"image/png", &pngClsid );

	bitmap->Save( fileName, &pngClsid );
}

void SaveJpg( const CString & fileName, Gdiplus::Bitmap * bitmap, ULONG quality )
{
	Gdiplus::REAL dpi = 96;
	bitmap->SetResolution( dpi, dpi );

	CLSID encoderClsid;
	GetEncoderClsid( L"image/jpeg", &encoderClsid );

	Gdiplus::EncoderParameters encoderParameters;

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &quality;

	//Gdiplus::Status stat =
	bitmap->Save( fileName, &encoderClsid, &encoderParameters );
}

Gdiplus::Bitmap * LoadBitmap( const std::vector<unsigned char> & buffer )
{
	return LoadBitmap( std::data(buffer), buffer.size() );
}

Gdiplus::Bitmap * LoadBitmap( const void * bytes, unsigned numBytes )
{
	Gdiplus::Bitmap * result = nullptr;

	HGLOBAL hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, numBytes );
	if( !hGlobal )
		AfxThrowMemoryException();
	{
		std::unique_ptr<Gdiplus::Bitmap> bitmap( nullptr );

		LPBYTE lpByte = (LPBYTE)::GlobalLock( hGlobal );
		if( lpByte )
		{
			CopyMemory( lpByte, bytes, numBytes );
			::GlobalUnlock( hGlobal );

			{
				IStream * spStream = nullptr;
				if( SUCCEEDED( ::CreateStreamOnHGlobal( hGlobal, TRUE, &spStream ) ) )
				{
					bitmap.reset( Gdiplus::Bitmap::FromStream( spStream ) );
					//Gdiplus::Status status = bitmap->GetLastStatus();
					spStream->Release();
				}
			}

			result = CopyBitmap( bitmap.get() );
		}
	}

	GlobalFree( hGlobal );

	return result;
}


void SaveBitmap( Gdiplus::Bitmap * bitmap, ULONG quality, std::vector<unsigned char> & buffer )
{
	HGLOBAL hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, 1024 );
	if( !hGlobal )
		AfxThrowMemoryException();

	Gdiplus::REAL dpi = 96;
	bitmap->SetResolution( dpi, dpi );

	CLSID encoderClsid;
	GetEncoderClsid( L"image/jpeg", &encoderClsid );

	Gdiplus::EncoderParameters encoderParameters;

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &quality;

	{
		CComPtr<IStream> spStream;
		if( SUCCEEDED( ::CreateStreamOnHGlobal( hGlobal, FALSE, &spStream ) ) )
		{
			bitmap->Save( spStream, &encoderClsid, &encoderParameters );

			STATSTG stat;
			spStream->Stat( &stat, 0 );

			buffer.resize( stat.cbSize.LowPart );
		}
	}
	LPBYTE  lpByte = (LPBYTE)::GlobalLock( hGlobal );
	if( lpByte )
		CopyMemory( std::data(buffer), lpByte, buffer.size() );
	::GlobalUnlock( hGlobal );

	GlobalFree( hGlobal );
}

