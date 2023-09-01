// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "ViewStyle.h"

#include <thread>
#include <vector>
#include <algorithm>
#include <omp.h>
#include "GlobalSettings.h"
#include "LoadJpg.h"
#include "BitmapTools.h"

void MakeGlassImage( unsigned char * __restrict bits, INT width, INT height, INT stride, INT radius, COLORREF glassColor, INT opacity );

Gdiplus::Color MakeColor( COLORREF color, BYTE a )
{
	return Gdiplus::Color( a, GetRValue( color ), GetGValue( color ), GetBValue( color ) );
}

ItemStyle::ItemStyle()
: width(0)
, height(0)
, offset(0)
, borderColor( 0 )
, borderOpacity(0)
, shadowColor( 0)
, shadowOpacity(0)
, posterHeight(0)
, fontName()
, sizeTitle1(0)
,colorTitle1(0)
, sizeTitle2(0)
, colorTitle2(0)
, sizeGenre(0)
, colorGenre(0)
, sizeTag(0)
, colorTag(0)
, btnSize(0)
, btnColor(0)
, btnColorHover(0)
, fontTitle1( nullptr )
, fontTitle2( nullptr )
, fontTags( nullptr )
, fontGenres( nullptr )
{
	ZeroMemory( &button, sizeof( button ) );
}

MenuStyle::MenuStyle()
: //fontName()
//, font(nullptr)
 iconSize(32)
//, fontSize(14)
, iconColor(0)
{

}


TreeStyle::TreeStyle()
	: fontName()
	, font(nullptr)
	, fontSize( 13 )
	, textColor( RGB( 0, 0, 0 ) )
	, textColorSelected( RGB( 255, 255, 255 ) )
{
}


ViewStyle::ViewStyle()
	: bgImageFile()
	, bgTopColor( 0 )
	, bgBottomColor( 0 )
	, glassColor( 0 )
	, glassOpacity( 0 )
	, glassBlur( 0 )
	, tileItem()
	, tileItemSelected()
	, listItem()
	, listItemSelected()
	, markerColor( 0 )
	, markerOpacity( 0 )
	, menu()
	, bgImage( nullptr )
	, bgBlured( nullptr )
	, fontCommandbar( nullptr )
{

}

ViewStyle::~ViewStyle()
{
	delete bgImage;
	bgImage = nullptr;

	delete bgBlured;
	bgBlured = nullptr;

	delete fontCommandbar;
	fontCommandbar = nullptr;

	delete tileItem.fontTitle1;
	delete tileItemSelected.fontTitle1;
	delete listItem.fontTitle1;
	delete listItemSelected.fontTitle1;

	delete tileItem.fontTitle2;
	delete tileItemSelected.fontTitle2;
	delete listItem.fontTitle2;
	delete listItemSelected.fontTitle2;

	delete tileItem.fontTags;
	delete tileItemSelected.fontTags;
	delete listItem.fontTags;
	delete listItemSelected.fontTags;

	delete tileItem.fontGenres;
	delete tileItemSelected.fontGenres;
	delete listItem.fontGenres;
	delete listItemSelected.fontGenres;
}

void ViewStyle::SaveStyle( const CString & fileName )
{

}

void ViewStyle::LoadStyle( const CString & fileName )
{

}

void ViewStyle::Initialize( bool waitResult )
{
	CWaitCursor waitCursor;

	UpdateFonts();

	delete bgImage;
	bgImage = nullptr;

	delete bgBlured;
	bgBlured = nullptr;

	if( !bgImageFile.IsEmpty() )
	{
		if( bgImageFile.Find( L':', 0 ) < 0 )
		{
			CString fullPath = GetGlobal().GetThemeDirectory();
			fullPath += bgImageFile;
			fullPath += L"\\bg.jpg";
			bgImageFile = fullPath;
		}
	}

	if( !bgImageFile.IsEmpty() && PathFileExists( bgImageFile ) )
	{
		bgImage = LoadBitmapFromFile( bgImageFile );
	}

	if( !bgImage )
	{
		bgImage = new Gdiplus::Bitmap( 1280, 720, PixelFormat32bppPARGB );

		std::unique_ptr<Gdiplus::Graphics> gdi( Gdiplus::Graphics::FromImage( bgImage ) );

		Gdiplus::LinearGradientBrush brush( Gdiplus::Point( 400, 0 ), Gdiplus::Point( 880, 720 ), MakeColor( GetGlobal().theme->bgTopColor ), MakeColor( GetGlobal().theme->bgBottomColor ) );
		brush.SetWrapMode( Gdiplus::WrapMode::WrapModeTileFlipXY );
		gdi->FillRectangle( &brush, Gdiplus::Rect( 0, 0, 1280, 720 ) );
	}

	UpdateGlass( waitResult );
}

void UpdateFonts( ItemStyle & item, bool createAdditional )
{
	delete item.fontTitle1;
	delete item.fontTitle2;
	delete item.fontGenres;
	delete item.fontTags;

	item.fontTitle1 = nullptr;
	item.fontTitle2 = nullptr;
	item.fontGenres = nullptr;
	item.fontTags = nullptr;


	if( !item.fontName.IsEmpty() )
		item.fontTitle1 = new Gdiplus::Font( item.fontName, (Gdiplus::REAL)item.sizeTitle1, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
	if( !item.fontTitle1 )
		item.fontTitle1 = new Gdiplus::Font( Gdiplus::FontFamily::GenericSansSerif(), (Gdiplus::REAL)item.sizeTitle1, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );

	if( !item.fontName.IsEmpty() )
		item.fontTitle2 = new Gdiplus::Font( item.fontName, (Gdiplus::REAL)item.sizeTitle2, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
	if( !item.fontTitle2 )
		item.fontTitle2 = new Gdiplus::Font( Gdiplus::FontFamily::GenericSansSerif(), (Gdiplus::REAL)item.sizeTitle2, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );

	if( !createAdditional )
		return;

	if( !item.fontName.IsEmpty() )
		item.fontGenres = new Gdiplus::Font( item.fontName, (Gdiplus::REAL)item.sizeGenre, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
	if( !item.fontGenres )
		item.fontGenres = new Gdiplus::Font( Gdiplus::FontFamily::GenericSansSerif(), (Gdiplus::REAL)item.sizeGenre, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );

	if( !item.fontName.IsEmpty() )
		item.fontTags = new Gdiplus::Font( item.fontName, (Gdiplus::REAL)item.sizeTag, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
	if( !item.fontTags )
		item.fontTags = new Gdiplus::Font( Gdiplus::FontFamily::GenericSansSerif(), (Gdiplus::REAL)item.sizeTag, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );

}

void ViewStyle::UpdateFonts()
{
	::UpdateFonts( tileItem, false );
	::UpdateFonts( tileItemSelected, false );
	::UpdateFonts( listItem, true );
	::UpdateFonts( listItemSelected, true );

	delete tree.font;
	tree.font = nullptr;
	if( !tree.fontName.IsEmpty() )
		tree.font = new Gdiplus::Font( tree.fontName, (Gdiplus::REAL)tree.fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
	if( !tree.font )
		tree.font = new Gdiplus::Font( Gdiplus::FontFamily::GenericSansSerif(), (Gdiplus::REAL)tree.fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );

	delete fontCommandbar;
	fontCommandbar = nullptr;
	if( !tree.fontName.IsEmpty() )
		fontCommandbar = new Gdiplus::Font( tree.fontName, (Gdiplus::REAL)32.0, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
	if( !fontCommandbar )
		fontCommandbar = new Gdiplus::Font( Gdiplus::FontFamily::GenericSansSerif(), (Gdiplus::REAL)32.0, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
}

Gdiplus::Bitmap * ViewStyle::GetGlassImage()
{
	return bgBlured ? bgBlured : bgImage;
}

void UpdateGlassThreaded( ViewStyle  * viewStyle )
{
	delete viewStyle->bgBlured;
	viewStyle->bgBlured = nullptr;

	Gdiplus::Bitmap * bgBlured = nullptr;

	if( viewStyle->bgImage )
	{
		bgBlured = CopyBitmap( viewStyle->bgImage );
	}

	if( !bgBlured )
	{
		bgBlured = new Gdiplus::Bitmap( 1280, 720, PixelFormat32bppPARGB );
		
		Gdiplus::Graphics * gdi = new Gdiplus::Graphics( bgBlured );

		Gdiplus::SolidBrush bgBrush( MakeColor( viewStyle->bgTopColor ) );

		gdi->FillRectangle( &bgBrush, 0, 0, bgBlured->GetWidth(), bgBlured->GetHeight() );

		delete gdi;
	}

	if( !bgBlured )
		return;

	Gdiplus::Rect lockRect( 0, 0, bgBlured->GetWidth(), bgBlured->GetHeight() );
	Gdiplus::BitmapData bitmapData;
	Gdiplus::Status status = bgBlured->LockBits( &lockRect, Gdiplus::ImageLockMode::ImageLockModeRead | Gdiplus::ImageLockMode::ImageLockModeWrite, PixelFormat32bppPARGB, &bitmapData );
	if( status != Gdiplus::Status::Ok )
		return;

	//DWORD startTime = GetTickCount();

	MakeGlassImage( (unsigned char*)bitmapData.Scan0, bitmapData.Width, bitmapData.Height, bitmapData.Stride, viewStyle->glassBlur, viewStyle->glassColor, viewStyle->glassOpacity );

	//DWORD endTime = GetTickCount();

	//double time = (endTime - startTime) / 1000.0;

	//CString msg;
	//msg.Format( L"Width = %i\r\nHeight = %i\r\nRadius = %i\r\nBlur time = %g", bgImage->GetWidth(), bgImage->GetHeight(), glassBlurRadius, time );
	//AfxMessageBox( msg );

	bgBlured->UnlockBits( &bitmapData );

	viewStyle->bgBlured = bgBlured;
}

void ViewStyle::UpdateGlass( bool waitResult )
{
	std::thread glassThread( UpdateGlassThreaded, this );
	if( waitResult )
		glassThread.join();
	else
		glassThread.detach();
}


namespace /*private*/
{
	struct RGBPix
	{
		RGBPix()
		{
			rgb[0] = 0;
			rgb[1] = 0;
			rgb[2] = 0;
		}

		RGBPix( const RGBPix & rhs )
		{
			rgb[0] = rhs.rgb[0];
			rgb[1] = rhs.rgb[1];
			rgb[2] = rhs.rgb[2];
		}

		RGBPix & operator= ( const RGBPix & rhs )
		{
			rgb[0] = rhs.rgb[0];
			rgb[1] = rhs.rgb[1];
			rgb[2] = rhs.rgb[2];

			return *this;
		}


		int MakeRGB() const
		{
			return (rgb[0] & 0x00FF) | ((rgb[1] & 0x00FF) << 8) | ((rgb[2] & 0x00FF) << 16);
		}

		inline void Add( const RGBPix & rhs )
		{
			rgb[0] += rhs.rgb[0];
			rgb[1] += rhs.rgb[1];
			rgb[2] += rhs.rgb[2];
		}

		inline void Sub( const RGBPix & rhs )
		{
			rgb[0] -= rhs.rgb[0];
			rgb[1] -= rhs.rgb[1];
			rgb[2] -= rhs.rgb[2];
		}


		inline void SetValue( unsigned color )
		{
			rgb[0] = (color) & 0x000000FF;
			rgb[1] = (color >> 8) & 0x000000FF;
			rgb[2] = (color >> 16) & 0x000000FF;
		}

		inline void Mul( unsigned short v )
		{
			rgb[0] *= v;//(int)floor( b*v + 0.5 );
			rgb[1] *= v;//(int)floor( g*v + 0.5 );
			rgb[2] *= v;//(int)floor( r*v + 0.5 );
		}

		inline void Div( unsigned short v )
		{
			rgb[0] /= v;
			rgb[1] /= v;
			rgb[2] /= v;
		}

		unsigned short rgb[3];
	};

}

void boxBlurH_4( const std::vector< RGBPix> & __restrict src, std::vector< RGBPix> & __restrict dest, int w, int h, int r )
{
	const unsigned short iarr = (r + r + 1);
	const unsigned short radi = r + 1;

#pragma omp parallel for
	for( int i = 0; i < h; ++i )
	{
		unsigned ti = i * w;
		unsigned li = ti;
		unsigned ri = ti + r;

		const RGBPix fv( src[ti] );
		const RGBPix lv( src[ti + w - 1] );
		RGBPix val( fv );
		val.Mul( radi );

		for( int j = 0; j < r; ++j )
		{
			val.Add( src[ti + j] );
		}

		for( int j = 0; j <= r; ++j )
		{
			val.Add( src[ri++] );
			val.Sub( fv );

			RGBPix & d = dest[ti++];
			d = val;
			d.Div( iarr );
		}

		for( int j = r + 1; j < w - r; ++j )
		{
			val.Add( src[ri++] );
			val.Sub( src[li++] );

			RGBPix & d = dest[ti++];
			d = val;
			d.Div( iarr );
		}

		for( int j = w - r; j < w; ++j )
		{
			val.Add( lv );
			val.Sub( src[li++] );

			RGBPix & d = dest[ti++];
			d = val;
			d.Div( iarr );
		}
	}
}

void boxBlurT_4( const std::vector< RGBPix> & __restrict src, std::vector< RGBPix> & __restrict dest, int w, int h, int r )
{
	const unsigned short iarr = (r + r + 1.0);
	const unsigned short radi = r + 1;

#pragma omp parallel for
	for( int i = 0; i < w; ++i )
	{
		unsigned ti = i;
		unsigned li = ti;
		unsigned ri = ti + r * w;

		const RGBPix fv = src[ti];
		const RGBPix lv = src[ti + w * (h - 1)];
		RGBPix val = fv;
		val.Mul( radi );

		for( int j = 0; j < r; ++j )
		{
			val.Add( src[ti + j * w] );
		}

		for( int j = 0; j <= r; ++j )
		{
			val.Add( src[ri] );
			val.Sub( fv );

			RGBPix & d = dest[ti];
			d = val;
			d.Div( iarr );

			ri += w;
			ti += w;
		}

		for( int j = r + 1; j < h - r; ++j )
		{
			val.Add( src[ri] );
			val.Sub( src[li] );

			RGBPix & d = dest[ti];
			d = val;
			d.Div( iarr );

			li += w;
			ri += w;
			ti += w;
		}

		for( int j = h - r; j < h; ++j )
		{
			val.Add( lv );
			val.Sub( src[li] );

			RGBPix & d = dest[ti];
			d = val;
			d.Div( iarr );

			ti += w;
		}
	}
}

void boxBlur_4( std::vector< RGBPix> & __restrict scl, std::vector< RGBPix> & __restrict tcl, int w, int h, int r )
{
	if( r <= 0 )
		return;

	tcl.swap( scl );

	//DWORD start = GetTickCount();

	boxBlurH_4( tcl, scl, w, h, r );
	boxBlurT_4( scl, tcl, w, h, r );

	//DWORD end = GetTickCount();

	//CString str;
	//str.Format( L"time = %f", (end - start) / 1000.0 );
	//AfxMessageBox( str, IDOK );
}

void gaussBlur_4( unsigned char * __restrict bits, INT width, INT height, INT stride, INT radius, COLORREF glassColor, int opacity )
{
	const int numGausPasses = 1;

	float wIdeal = sqrt( (12.0 * radius*radius / numGausPasses) + 1.0 );

	int wl = floor( wIdeal );
	if( (wl % 2) == 0 )
	{
		wl -= 1;
		if( wl == -1 )
			wl = 0;
	}

	int wu = wl + 2;

	float mIdeal = (12.0 * radius*radius - numGausPasses * wl*wl - 4.0 * numGausPasses*wl - 3.0 * numGausPasses) / (-4.0 * wl - 4.0);
	int m = floor( mIdeal + 0.5 );

	int bxs[numGausPasses];
	for( int i = 0; i < numGausPasses; ++i )
	{
		bxs[i] = i < m ? wl : wu;
	}


	std::vector< RGBPix> src( width*height );
	std::vector< RGBPix> dest( width*height );
	{
		unsigned char * rgb = bits;
		auto px = src.begin();

		for( int h = 0; h < height; ++h )
		{
			int * color = (int*)rgb;
			for( int w = 0; w < width; ++w )
			{
				//int col = color[w];

				px->SetValue( color[w] );
				++px;
			}

			rgb += stride;
		}
	}

	boxBlur_4( src, dest, width, height, (int)((bxs[0] - 1) / 2.0) );
	//boxBlur_4( dest, src, width, height, (bxs[1] - 1) / 2.0 );
	//boxBlur_4( src, dest, width, height, (bxs[2] - 1) / 2.0 );

	{
		const unsigned short ro = ((glassColor) & 0x000000FF) * opacity;
		const unsigned short go = ((glassColor >> 8) & 0x000000FF) * opacity;
		const unsigned short bo = ((glassColor >> 16) & 0x000000FF) * opacity;

		unsigned char * rgb = bits;
		auto px = dest.begin();

		for( int h = 0; h < height; ++h )
		{
			int * color = (int*)rgb;

			for( int w = 0; w < width; ++w )
			{
				const unsigned pb = (std::min)(((px->rgb[0] * (255 - opacity)) + bo) / 255, 255);
				const unsigned pg = (std::min)(((px->rgb[1] * (255 - opacity)) + go) / 255, 255);
				const unsigned pr = (std::min)(((px->rgb[2] * (255 - opacity)) + ro) / 255, 255);

				//*color = px->MakeRGB();

				*color = 0xFF000000 | (pb) | (pg << 8) | (pr << 16);

				++px;
				++color;
			}

			rgb += stride;
		}
	}
}

inline void MakeGlassImage( unsigned char * __restrict bits, INT width, INT height, INT stride, INT radius, COLORREF glassColor, INT opacity )
{
	gaussBlur_4( bits, width, height, stride, radius, glassColor, opacity );

}

void DefaultMenuTheme( MenuStyle & menu )
{
	//menu.fontName = L"Microsoft Sans Serif";
	//menu.fontSize = 24;
	menu.iconSize = 32;
	menu.iconColor = RGB( 52, 52, 52 );
}

void DefaultTileItemTheme( ItemStyle & item )
{
	item.width = 260;
	item.height = 260;
	item.offset = 10;

	item.shadowColor = RGB( 0, 0, 0 );
	item.shadowOpacity = 60;

	item.borderColor = RGB( 50, 50, 50 );
	item.borderOpacity = 150;

	item.btnColor = RGB( 52, 52, 52 );
	item.btnColorHover = RGB( 51, 147, 255 );

	item.btnSize = 32;

	item.colorTitle1 = RGB( 52, 52, 52 );
	item.colorTitle2 = RGB( 100, 100, 100 );

	item.sizeTitle1 = 14;
	item.sizeTitle2 = 13;
	item.sizeGenre = 12;
	item.sizeTag = 12;

	item.button[0] = { 16, 16 };//PlayBtn
	item.button[1] = { 218, 16 };//MenuBtn
	item.button[2] = { 16, 80 };//DescriptionBtn
	item.button[3] = { 218, 80 };//TagsBtn
	item.button[4] = { 16, 138 };//ShareBtn
	item.button[5] = { 218, 138 };//TechInfoBtn

}

void DefaultListItemTheme( ItemStyle & item )
{
	item.width = 0;
	item.height = 100;
	item.offset = 10;

	item.shadowColor = RGB( 0, 0, 0 );
	item.shadowOpacity = 60;

	item.borderColor = RGB( 50, 50, 50 );
	item.borderOpacity = 150;

	item.btnColor = RGB( 52, 52, 52 );
	item.btnColorHover = RGB( 51, 147, 255 );

	item.btnSize = 32;

	item.colorTitle1 = RGB( 52, 52, 52 );
	item.colorTitle2 = RGB( 100, 100, 100 );
	item.colorGenre = RGB( 0, 125, 32 );
	item.colorTag = RGB( 0, 32, 125 );

	item.sizeTitle1 = 14;
	item.sizeTitle2 = 13;
	item.sizeGenre = 12;
	item.sizeTag = 12;

	item.button[0] = { 80, 8 };//PlayBtn
	item.button[4] = { 128, 8 };//ShareBtn
	item.button[1] = { 176, 8 };//MenuBtn

	item.button[2] = { 80, 48 };//DescriptionBtn
	item.button[3] = { 128, 48 };//TagsBtn
	item.button[5] = { 176, 48 };//TechInfoBtn

}


void DefaultTheme( ViewStyle & theme )
{
	DefaultTileItemTheme( theme.tileItem );
	DefaultTileItemTheme( theme.tileItemSelected );

	DefaultListItemTheme( theme.listItem );
	DefaultListItemTheme( theme.listItemSelected );

	DefaultMenuTheme( theme.menu );


	//bgImageFile; // фоновая картинка
	theme.bgTopColor = RGB(255,255,255); // однородный фон, если нет картинки
	theme.bgBottomColor = RGB(192,192,192); // однородный фон, если нет картинки

	theme.glassColor = RGB(255,255,255); // цвет выбранного элемента
	theme.glassOpacity = 150; // прозрачность фона выбранного элемента
	theme.glassBlur = 1; // радиус размытия фоновой картинки

	// маркер на объекте
	theme.markerColor = RGB(0,255,0); // цвет выделения просмотренного/непросмотренного фильма
	theme.markerOpacity = 255;

}
