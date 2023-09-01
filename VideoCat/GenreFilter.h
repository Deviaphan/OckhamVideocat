#pragma once

#include <set>
#include "genres.h"
#include "TagManager.h"

namespace FilterType
{
	enum Enum
	{
		ByGenre,
		ByTag,
	};
}

class CGenreFilter : public CDialog
{
	DECLARE_DYNAMIC(CGenreFilter)

public:
	explicit CGenreFilter( CWnd* pParent = nullptr );
	~CGenreFilter() override;

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_GENRE
	};
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedByGenres();
	afx_msg void OnBnClickedByGenreName();
	afx_msg void OnBnClickedRadioGenre();
	afx_msg void OnBnClickedRadioTag();

	void ReadGenre();

private:
	CCheckListBox _checkList;
	
public:
	CPoint position;

	BOOL film3D;
	BOOL strictGenre;

	std::map<Genre, unsigned> inputGenres;
	std::map<TagId, unsigned> inputTags;

	Genres outputGenres;

	TagManager * tagManager;
	std::set<TagId> selectedTags;

	BOOL extraFiltering;

	int filterType;
};
