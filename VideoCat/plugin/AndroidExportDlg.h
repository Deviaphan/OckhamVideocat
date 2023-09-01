#pragma once
#include <vector>
#include "../TagManager.h"
#include "afxwin.h"

class CollectionDB;

class CAndroidExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CAndroidExportDlg)

public:
	CAndroidExportDlg( CollectionDB & cdb );
	virtual ~CAndroidExportDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANDROID };
#endif

public:
	enum ExportType
	{
		ExportAllFilm = 0, // экспорт всех фильмов
		ExportSelectedTag, // экспортировать только фильмы с одним из заданных тэгов
		ExportNotSelectedTag, // экспортировать только фильмы, у которых нет заданных тэгов
	};

	enum MarkType
	{
		MarkNotViewed, // помечать е просмотренные фильмы
		MarkViewed, // помечать просмотреные фильмы
		MarkNone, // не помечать никакие фильмы
	};

protected:
	BOOL OnInitDialog() override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSelectTags();
	afx_msg void OnBnClickedTutorial();

private:
	CComboBox _themes;
	CollectionDB & _cdb;

public:
	CString exportDir;
	int exportType;
	BOOL filterTree;
	BOOL filterRu;
	BOOL filterEn;
	BOOL filterYear;
	BOOL filterGenre;
	BOOL filterTag;
	BOOL exportDescr;
	int markType;
	int themeIndex;

	std::vector< std::pair<TagId, BOOL> > tags;

	std::vector<CString> themeNames;
};
