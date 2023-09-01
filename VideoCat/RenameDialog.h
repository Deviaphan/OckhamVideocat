#pragma once
#include <vector>
#include <string>
#include "afxwin.h"

struct NameFormat
{
	NameFormat()
		: useYear( TRUE )
		, useRu( TRUE )
		, useEn( TRUE )
		, preEpisode()
		, postEpisode()
		, preYear()
		, postYear( L" - " )
		, preRu()
		, postRu()
		, preEn( L" (" )
		, postEn( L")" )
	{
		order[0] = 0;
		order[1] = 1;
		order[2] = 2;
	}

	BOOL useYear;
	BOOL useRu;
	BOOL useEn;
	CString preEpisode;
	CString postEpisode;
	CString preYear;
	CString postYear;
	CString preRu;
	CString postRu;
	CString preEn;
	CString postEn;

	int order[3];
};

class CRenameDialog: public CDialog
{
	DECLARE_DYNAMIC( CRenameDialog )

public:
	CRenameDialog( CWnd* pParent = NULL );
	virtual ~CRenameDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RENAME };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );
	void RefreshExampleList();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnUp();
	afx_msg void OnBtnDown();

private:
	CListBox _orderList;
	CListBox _exampleList;

public:
	struct Film
	{
		explicit Film( const std::wstring & r = std::wstring(), const std::wstring & e = std::wstring(), unsigned short y = 0, const std::wstring & ep = std::wstring() )
			: rus( r )
			, en( e )
			, year( y )
			, episode( ep )
		{
		}

		std::wstring rus;
		std::wstring en;
		std::wstring episode;
		unsigned short year;
	};

	std::vector<Film> exampleNames;
	NameFormat nameFormat;

	unsigned numFiles;
};

CString BuildNewName( const NameFormat & nameFormat, const std::wstring & ru, const std::wstring & en, unsigned short у, const std::wstring & episode );
