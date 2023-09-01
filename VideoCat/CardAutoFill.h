#pragma once

#include <vector>
#include "CardPageBase.h"
#include "Kinopoisk/KinopoiskInfo.h"

class CCardAutoFill : public CMFCPropertyPage, public ICardPageBase
{
	DECLARE_DYNAMIC(CCardAutoFill)

public:
	CCardAutoFill();
	virtual ~CCardAutoFill();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_AUTOFILL };
#endif

public:
	virtual BOOL OnInitDialog();
	virtual void ReinitTab() override;
	virtual bool SaveData() override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	void EnableControls( BOOL on );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedFillByIdDb();
	afx_msg void OnBnClickedFillByIdSite();
	afx_msg void OnBnClickedFillByName();
	afx_msg void OnBnClickedRadioFillSkip();
	afx_msg void OnBnClickedRadioFillUser();
	afx_msg void OnBnClickedRadioFillAuto();
	afx_msg void OnBnClickedApplyFilm();

private:
	int _fillType;
	UINT _filmId;
	CString _title;
	CComboBox _pluginList;

	CListBox _filmList;
	std::vector<KinopoiskInfo> _foundedFilms;
};

void Copy( const KinopoiskInfo & ki, Entry & entry );
