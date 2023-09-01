#pragma once

class LicenseBox
{
public:
	bool Initialize( const CString & keyPath, const CRect & rect );

	void NextFrame();
	void Draw( HDC dc );
	const CRect & GetRect() const
	{
		return _rect;
	}

public:
	CRect _rect;
	CImage _keyImg;
};
