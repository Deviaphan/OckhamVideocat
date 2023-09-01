#pragma once


class CBehaviorDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CBehaviorDlg)

public:
	CBehaviorDlg();
	virtual ~CBehaviorDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BEHAVIOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void OnOK(){}
	void OnCancel(){}

	DECLARE_MESSAGE_MAP()

public:
	int marktype;
	BOOL closeBeforePlay;
	BOOL alwaysShowRating;
	BOOL hideSecondTitle;
	BOOL tipOfDay;
	int numRandomFilms;
	BOOL autoplayRandomFilm;
};
