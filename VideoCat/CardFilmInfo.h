#pragma once
#include <map>
#include "genres.h"
#include "Entry.h"
#include "CardPageBase.h"

class CVideoCatDoc;

class BrowseForFolderCtrl : public CMFCEditBrowseCtrl
{
public:
	virtual void OnBrowse();

	CString openFolder;
};

class BrowseForUrlCtrl : public CMFCEditBrowseCtrl
{
public:
	virtual void OnBrowse()
	{
		CString path;
		GetWindowText( path );
		::ShellExecute( nullptr, L"open", path, 0, 0, SW_SHOWNORMAL );
	}

	CString GetURL() const
	{
		CString urlPath;
		GetWindowText( urlPath );
		return urlPath;
	}
};

class CCardFilmInfo : public CMFCPropertyPage, public ICardPageBase
{
	DECLARE_DYNAMIC(CCardFilmInfo)

public:
	CCardFilmInfo();
	virtual ~CCardFilmInfo();

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_TAB_FILM_INFO
	};
#endif

public:
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );

	void RefreshGenreList( Genres genres );

public:
	virtual void ReinitTab() override;
	virtual bool SaveData() override;

private:

	// Группа "Видео"
	CString _titleRussian;
	CString _titleOriginal;

	double _rating;

	UINT _titleYear;

	UINT _season;
	UINT _episode;
	
	CString _filmFullPath;
	BrowseForFolderCtrl _filmPath;
	BrowseForUrlCtrl _browseUrl;

	CCheckListBox _genres;
	BOOL _film3D;

	FILETIME _date;
	UINT _numViews;
};
