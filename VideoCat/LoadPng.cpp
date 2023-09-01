// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "LoadPng.h"
#include "BitmapTools.h"
#include <memory>

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void LoadPng( CImage & img, UINT id )
{
	CPngImage pngImage;
	pngImage.Load( id );
	img.Attach( (HBITMAP)pngImage.Detach() );

	PremulAlpha( img );
}


 void LoadPng( const CString & fullPath, CImage & img )
 {
	 img.Load( fullPath );

	 PremulAlpha( img );
 }

 void PremulAlpha( CImage & img )
 {
	 if( img.GetBPP() != 32 )
		 return;

	 // в png прозрачные пиксели бывают не чёрные. Чтобы на всех карточках правильно работала прозрачность, нужно убедиться, что прозрачные пиксели чёрные

	 unsigned char * bits = (unsigned char*)img.GetBits();
	 const int pitch = img.GetPitch();

	 const int width = img.GetWidth();
	 const int height = img.GetHeight();

	 for( int y = 0; y < height; ++y )
	 {
		 unsigned int * pixel = (unsigned int*)bits;
		 unsigned int * end = pixel + width;

		 for( ; pixel < end; ++pixel )
		 {
			 // если пиксель полностью прозрачный, то задаём ему чёрный цвет
			 if( (*pixel & 0xFF000000) == 0 )
			 {
				 *pixel = 0;
			 }
			 else
			 {
				 // предумножаем полупрозрачные пикселы
				 const unsigned int a = (*pixel >> 24) & 0x000000FF;

				 unsigned int r = ((*pixel & 0xFF) * a) >> 8;
				 unsigned int g = (((*pixel >> 8) & 0xFF) * a) >> 8;
				 unsigned int b = (((*pixel >> 16) & 0xFF) * a) >> 8;

				 *pixel = (a << 24) | (b << 16) | (g << 8) | r;
			 }
		 }

		 bits += pitch;
	 }
 }

 void LoadPng( const CString & fullPath, Gdiplus::Bitmap * & img )
 {
	 delete img;	 

	std::unique_ptr<Gdiplus::Bitmap> temp = std::make_unique<Gdiplus::Bitmap>( fullPath );

	 img = CopyBitmap( temp.get() );

	 //PremulAlpha( *img );
 }

 void PremulAlpha( Gdiplus::Bitmap & img )
 {
	 //if( img.GetBPP() != 32 )
	//	 return;
	 const Gdiplus::PixelFormat format = img.GetPixelFormat();
	 if( (format & PixelFormatAlpha) == 0 )
		 return;

	 // в png прозрачные пиксели бывают не чёрные. Чтобы на всех карточках правильно работала прозрачность, нужно убедиться, что прозрачные пиксели чёрные

	 Gdiplus::Rect rect( 0, 0, img.GetWidth(), img.GetHeight() );
	 Gdiplus::BitmapData bData;

	 img.LockBits( &rect, 0, PixelFormat32bppARGB, &bData );

	 unsigned char * bits = (unsigned char*)bData.Scan0;//img.GetBits();
	 const int pitch = bData.Stride;//img.GetPitch();

	 const int width = bData.Width;//img.GetWidth();
	 const int height = bData.Height;//img.GetHeight();

	 for( int y = 0; y < height; ++y )
	 {
		 unsigned int * pixel = (unsigned int*)bits;
		 unsigned int * end = pixel + width;

		 for( ; pixel < end; ++pixel )
		 {
			 // если пиксель полностью прозрачный, то задаём ему чёрный цвет
			 if( (*pixel & 0xFF000000) == 0 )
			 {
				 *pixel = 0;
			 }
			 else
			 {
				 // предумножаем полупрозрачные пикселы
				 const unsigned int a = (*pixel >> 24) & 0x000000FF;

				 unsigned int r = ((*pixel & 0xFF) * a) >> 8;
				 unsigned int g = (((*pixel >> 8) & 0xFF) * a) >> 8;
				 unsigned int b = (((*pixel >> 16) & 0xFF) * a) >> 8;

				 *pixel = (a << 24) | (b << 16) | (g << 8) | r;
			 }
		 }

		 bits += pitch;
	 }

	 img.UnlockBits( &bData );
 }

 Gdiplus::Bitmap * LoadPng( UINT id )
 {
	 Gdiplus::Bitmap * img = nullptr;

	 HRSRC hrsrc = ::FindResource( AfxGetInstanceHandle(), MAKEINTRESOURCE( id ), L"PNG" );
	 if( !hrsrc )
		 return img;

	 HGLOBAL hgrsrc = ::LoadResource( AfxGetInstanceHandle(), hrsrc );
	 if( !hgrsrc )
		 return img;

	 LPBYTE pData = (LPBYTE)::LockResource( hgrsrc );
	 DWORD dwTotalSize = ::SizeofResource( 0, hrsrc );

	 HGLOBAL hg = ::GlobalAlloc( GHND, dwTotalSize );
	 LPVOID pvData = ::GlobalLock( hg );
	 CopyMemory( pvData, pData, dwTotalSize );
	 ::GlobalUnlock( hg );

	 // здесь конвертим в битмап
	 LPSTREAM pStream = nullptr;
	 if( CreateStreamOnHGlobal( hg, TRUE, &pStream ) == S_OK && pStream )
	 {
		 std::unique_ptr< Gdiplus::Bitmap> temp( Gdiplus::Bitmap::FromStream( pStream ) );

		 img = CopyBitmap( temp.get() );

		 pStream->Release();
	 }
	 GlobalFree( hg );

	 return img;
 }
