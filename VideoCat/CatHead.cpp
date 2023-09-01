// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CatHead.h"
#include "LoadPng.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void CatHead::Initialize( const CRect & rect )
{
	_eyeOffsetX = 0;
	_eyeOffsetY = 0;

	LoadPng( _bg, IDB_LOGO );

	_logoRect = rect;
	_logoRect.right = _logoRect.left + _bg.GetWidth();
	_logoRect.bottom = _logoRect.top + _bg.GetHeight();

	_center.SetPoint( _logoRect.left + _logoRect.Width() / 2, _logoRect.top + _logoRect.Height() / 2 );
	_eyes.LoadFromResource( AfxGetApp()->m_hInstance, IDB_EYES );

	_backbuffer.Create( _logoRect.Width(), _logoRect.Height(), 24 );
}

void CatHead::Draw( HDC dc )
{
	HDC bgdc = _backbuffer.GetDC();

	_bg.BitBlt( bgdc, 0, 0, SRCCOPY );

	const int eyesW = _eyes.GetWidth();
	const int eyesH = _eyes.GetHeight();

	const int eyesX = _logoRect.Width() / 2 - eyesW / 2 + _eyeOffsetX;
	const int eyesY = _logoRect.Height() / 2 - eyesH / 2 + _eyeOffsetY;

	_eyes.TransparentBlt( bgdc, eyesX, eyesY, eyesW, eyesH, RGB( 255, 255, 255 ) );

	::BitBlt( dc, _logoRect.left, _logoRect.top, _logoRect.Width(), _logoRect.Height(), bgdc, 0, 0, SRCCOPY );

	_backbuffer.ReleaseDC();
}

void CatHead::MouseMove( int x, int y )
{
	_eyeOffsetX = (x - _center.x) / 5;
	if( _eyeOffsetX < -10 )
		_eyeOffsetX = -10;
	if( _eyeOffsetX > 10 )
		_eyeOffsetX = 10;

	_eyeOffsetY = (y - _center.y) / 5;
	if( _eyeOffsetY < -15 )
		_eyeOffsetY = -15;
	if( _eyeOffsetY > 25 )
		_eyeOffsetY = 25;
}

const CRect & CatHead::GetRect() const
{
	return _logoRect;
}
