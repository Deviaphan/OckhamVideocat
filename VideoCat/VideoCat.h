#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "afxwinappex.h"

class SingleInstance
{
public:
	SingleInstance()
		: _mutex( nullptr )
	{
		_mutex = OpenMutex( SYNCHRONIZE, FALSE, L"VIDEOCAT6FA26005A846411DA1E7B6986125DCF9" );
		if( _mutex != nullptr )
		{
			ReleaseMutex( _mutex );
			_mutex = nullptr;
		}
		else
		{
			_mutex = CreateMutex( nullptr, FALSE, L"VIDEOCAT6FA26005A846411DA1E7B6986125DCF9" );
		}
	}

	~SingleInstance()
	{
		CloseHandle( _mutex );
	}

	bool IsSecondInstance() const
	{
		return !_mutex;
	}

private:
	HANDLE _mutex;
};

class CVideoCatApp : public CWinAppEx
{
public:
	CVideoCatApp();

public:
	BOOL InitInstance() override;
	int ExitInstance() override;

	DECLARE_MESSAGE_MAP()

private:
	SingleInstance _si;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

};

extern CVideoCatApp theApp;
