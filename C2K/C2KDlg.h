#pragma once
#include "afxcmn.h"
#include "CinemateInfo.h"
#include "KinopoiskInfo.h"
#include "FileInfo.h"
#include <vector>
#include <map>


class CC2KDlg : public CDialog
{
public:
	CC2KDlg(CWnd* pParent = NULL);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_C2K_DIALOG };
#endif

public:
	void Step()
	{
		_progress.StepIt();
	}
	void SetStepCounts( int count )
	{
		_progress.SetRange32( 0, count );
		_progress.SetPos( 0 );
		_progress.SetStep( 1 );
	}

	void LoadCinemate();
	void LoadKinopoisk();
	void LoadC2K();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();


public:
	CString userLibrary;

private:
	HICON m_hIcon;
	CProgressCtrl _progress;

public:
	std::vector<CinemateInfo> _cinemateBase;
	std::vector<KinopoiskInfo> _kinopoiskBase;
	std::map<int, int> _c2k;

	FileTree _fileTree;
	virtual void OnOK();
};
