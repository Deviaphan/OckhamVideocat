#pragma once

#include <fstream>
#include <vector>
#include "BitmapTools.h"

inline void LoadJpg( CImage & img, UINT id )
{
	HRSRC hrsrc = ::FindResource( nullptr, MAKEINTRESOURCE(id), L"JPG" );
	if( !hrsrc )
		return;

	HGLOBAL hgrsrc = ::LoadResource( nullptr, hrsrc );
	if( !hgrsrc )
		return;

	LPBYTE pData = static_cast<LPBYTE>(::LockResource( hgrsrc ));
	DWORD dwTotalSize = ::SizeofResource( nullptr, hrsrc );
	HGLOBAL hg = ::GlobalAlloc( GHND, dwTotalSize );
	if( hg == nullptr )
		return;

	LPVOID pvData = ::GlobalLock( hg );
	if( !pvData )
		return;

	CopyMemory( pvData, pData, dwTotalSize );
	::GlobalUnlock( hg );

	// здесь конвертим в битмап
	LPSTREAM pStream = nullptr;
	if( CreateStreamOnHGlobal( hg, TRUE, &pStream ) == S_OK && pStream )
	{
		img.Load( pStream );
		pStream->Release();
	}
}

inline Gdiplus::Bitmap * LoadBitmapFromFile( const wchar_t * filename )
{
	std::unique_ptr<Gdiplus::Bitmap> b( Gdiplus::Bitmap::FromFile( filename ) );
	return CopyBitmap( b.get() );
}
