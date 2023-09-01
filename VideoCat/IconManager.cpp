// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "IconManager.h"
#include <map>
#include "GlobalSettings.h"
#include "ErrorDlg.h"
#include "LoadPng.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IconManager & GetIconManager()
{
	static IconManager global;
	return global;
}

IconManager::IconManager()
	: _iconMap( nullptr )
	, _stars( nullptr )
	, _numRows(0)
	, _numCols(0)
{
	_iconMap = LoadPng( IDB_ICON_MAP );

	if( !_iconMap || _iconMap->GetWidth() < 64 || _iconMap->GetHeight() < 64 )
	{
		ShowError( L"Не найден файл с иконками", false );
		throw std::exception();
	}

	_numCols = _iconMap->GetWidth() / _size;
	_numRows = _iconMap->GetHeight() / _size;	

	_stars = LoadPng( IDB_STARS );
}

IconManager::~IconManager()
{
	Destroy();
}

void IconManager::Destroy()
{
	delete _iconMap;
	_iconMap = nullptr;

	delete _stars;
	_stars = nullptr;
}

void IconManager::Draw( Gdiplus::Graphics & gdi, IconID id, int posX, int posY, int size, COLORREF color )
{
	// смешивание картинки с цветом
	Gdiplus::ColorMatrix cm;
	ZeroMemory( &cm.m, sizeof( cm ) );
	cm.m[0][0] = GetRValue( color ) / 255.0f;
	cm.m[1][1] = GetGValue( color ) / 255.0f;
	cm.m[2][2] = GetBValue( color ) / 255.0f;
	cm.m[3][3] = 1.0f;
	cm.m[4][4] = 1.0f;

	Gdiplus::ImageAttributes ia;
	ia.SetColorMatrix( &cm );

	const Gdiplus::RectF destRect( posX, posY, size, size );

	const int sx = (id % _numCols) * _size;
	const int sy = (id / _numCols) * _size;

	const Gdiplus::RectF srcRect( sx, sy, _size, _size );

	const auto oldMode = gdi.GetInterpolationMode();

	gdi.SetSmoothingMode( Gdiplus::SmoothingMode::SmoothingModeHighQuality );
	gdi.SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic );

	gdi.DrawImage( _iconMap, destRect, srcRect, Gdiplus::Unit::UnitPixel, &ia);

	gdi.SetInterpolationMode( oldMode );
}

void IconManager::DrawStars( Gdiplus::Graphics & gdi, unsigned int rating, int centerX, int topY, int size )
{
	const Gdiplus::Rect starSrc[3] = { Gdiplus::Rect( 64, 0, 32, 32 ), Gdiplus::Rect( 32, 0, 32, 32 ), Gdiplus::Rect( 0, 0, 32, 32 ) };

	int srcIndex[5] = { 0 }; // пустые звёздочки
	if( rating == 1 )
		srcIndex[0] = 1;
	if( rating > 1 )
		srcIndex[0] = 2;

	if( rating == 3 )
		srcIndex[1] = 1;
	if( rating > 3 )
		srcIndex[1] = 2;

	if( rating == 5 )
		srcIndex[2] = 1;
	if( rating > 5 )
		srcIndex[2] = 2;

	if( rating == 7 )
		srcIndex[3] = 1;
	if( rating > 7 )
		srcIndex[3] = 2;

	if( rating == 9 )
		srcIndex[4] = 1;
	if( rating > 9 )
		srcIndex[4] = 2;

	const auto oldMode = gdi.GetInterpolationMode();
	gdi.SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );

	Gdiplus::Rect destRect( centerX - (size + size + size/2), topY, size, size );
	for( int i : srcIndex )
	{
		int x = starSrc[i].GetLeft();

		gdi.DrawImage(
			_stars,
			destRect,
			x, 0, 32, 32,
			Gdiplus::Unit::UnitPixel );

		destRect.Offset( size, 0 );
	}

	gdi.SetInterpolationMode( oldMode );
}
