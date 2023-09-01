#pragma once

class CatHead
{
public:
	CatHead()
		: _eyeOffsetX( 0 )
		, _eyeOffsetY( 0 )
	{}

public:
	void Initialize( const CRect & rect );

	void Draw( HDC dc );
	void MouseMove( int x, int y );
	const CRect & GetRect() const;

private:
	CImage _backbuffer;
	CImage _bg;
	CImage _eyes;
	CRect _logoRect;
	CPoint _center;
	int _eyeOffsetX;
	int _eyeOffsetY;
};
