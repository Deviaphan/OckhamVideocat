// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "LicenseBox.h"



bool LicenseBox::Initialize( const CString & keyPath, const CRect & rect )
{
	_keyImg.Load( keyPath );
	if( _keyImg.IsNull() )
		return false;

	if( !_keyImg.IsIndexed() )
		return false;

	if( _keyImg.GetWidth() != 256 || _keyImg.GetHeight() != 256 )
		return false;

	_rect = rect;
	_rect.right = rect.left + 256;
	_rect.bottom = rect.top + 256;

	return true;
}

void LicenseBox::Draw( HDC dc )
{
	_keyImg.BitBlt( dc, _rect, CPoint( 0, 0 ), SRCCOPY );
}

void LicenseBox::NextFrame()
{
	if( _keyImg.IsNull() )
		return;

	const int pitch = _keyImg.GetPitch();
	unsigned char * bits = (unsigned char*)_keyImg.GetBits();

	for( int y = 0; y < 256; ++y )
	{
		for( int x = 0; x < 256; ++x )
		{
			int pix = bits[x];
			pix += 1;
			if( pix > 255 )
				pix -= 256;

			bits[x] = (unsigned char)pix;
		}

		bits += pitch;
	}
}
