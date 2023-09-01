#pragma once

#include "Entry.h"
#include "CardPageBase.h"
#include "poster.h"

class CCardPoster : public CMFCPropertyPage, public ICardPageBase
{
	DECLARE_DYNAMIC(CCardPoster)

public:
	CCardPoster();
	virtual ~CCardPoster();

	virtual BOOL OnInitDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_POSTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedLoadPoster();
	afx_msg void OnEnSetfocusEditPosterUrl();
	afx_msg void OnEnSetfocusPosterPath();
	afx_msg void OnCopyFromClipboard();
	afx_msg void OnPaint();
	afx_msg void OnSaveToDisk();
	afx_msg void OnCopyWebcam();


public:
	virtual void ReinitTab() override;
	virtual bool SaveData() override;

	void LoadPosterById();

private:
	int _posterMode;
	int _usedPosterMode;

	CString _posterUrl;
	CMFCEditBrowseCtrl _posterPath;


	PosterID _oldPosterID;

	Gdiplus::Bitmap * _preview;
};
