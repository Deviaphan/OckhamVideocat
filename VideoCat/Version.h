#pragma once

class Version
{
public:
	Version()
		: build( 0 )
		, productMS( 0 )
		, productLS( 0 )
		, fileVersion( 0 )
	{
	}

public:
	BOOL HasNewVersion();

	void ReadVersion();

	CString GetVersionName() const;

public:
	CString version;
	CString whatNew;
	unsigned int build;

	DWORD productMS;
	DWORD productLS;
	DWORD fileVersion;
};
