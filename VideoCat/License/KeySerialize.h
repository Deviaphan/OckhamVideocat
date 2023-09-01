#pragma once

enum class LicenseType: int
{
	Personal_1PC = 1,
	Home_3PC = 2,
	Network_AnyPC = 3,
	Commercial = 4,
};

class KeySerialize
{
public:
	KeySerialize()
		: _ptr( nullptr )
		, _end( nullptr )
		, licenseType( 0 )
	{
	}

	void Serialize();
	bool Deserialize();

private:
	void WriteHeader( const char * str );
	void ReadHeader( char * str );

	void WriteString( const wchar_t * str );
	void ReadString( wchar_t * str );

	unsigned char * _ptr;
	const unsigned char * _end;

public:
	CString email;
	CString name;
	int licenseType;

	unsigned char buffer[256];
};

void ReadBuffer( const CImage & image, unsigned char buffer[256] );
