#pragma once

class CImageSmartDc
{
public:
	CImageSmartDc()
		: _image( nullptr )
		, _ptr( nullptr )
		, _hdc( nullptr )
	{}

	explicit CImageSmartDc( CImage & image )
		: _image( &image )
		, _ptr( 0 )
		, _hdc( 0 )
	{
		_hdc = _image->GetDC();
		_ptr = CDC::FromHandle( _hdc );
	}

	~CImageSmartDc()
	{
		if( _ptr )
			_image->ReleaseDC();
	}

	void Attach( CImage & image )
	{
		_image = &image;
		_hdc = _image->GetDC();
		_ptr = CDC::FromHandle( _hdc );
	}

public:
	operator CDC*()
	{
		return _ptr;
	}

	operator HDC()
	{
		return _hdc;
	}


private:
	CImage * _image;
	CDC * _ptr;
	HDC _hdc;
};
