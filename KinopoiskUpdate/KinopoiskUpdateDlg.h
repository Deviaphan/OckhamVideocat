#pragma once
#include <vector>
#include <unordered_map>
#include "afxwin.h"
#include "Kinopoisk/KinopoiskInfo.h"
#include <Net/CurlBuffer.h>
#include <sstream>

class CKinopoiskUpdateDlg;


class CKinopoiskUpdateDlg : public CDialog
{
// Создание
public:
	CKinopoiskUpdateDlg( CWnd* pParent = nullptr );

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KINOPOISKUPDATE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	
public:
	bool AddItem( const KinopoiskInfo & item );

	bool ProcessFilm( KinopoiskId id, const char * ptr, size_t size, bool isWindows1251 );
	void ImportFinished();
	bool IsStoped();

	void DrawDataBase( int currentIndex );

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual void OnOk()
	{}
	virtual void OnCancel()
	{}

	void BuildPath();

	void ProcessID();


	DECLARE_MESSAGE_MAP()
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnRefill();
	afx_msg void OnBtnImportKinopoisk();
	afx_msg void OnBtnFindEmpty();
	afx_msg void OnBtnExit();
	afx_msg void OnBtnShowFilm();
	afx_msg void OnBtnExportDB();
	afx_msg void OnBtnImportLost();
	afx_msg void OnBtnStop();
	afx_msg void OnPaint();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnBnClickedUpdateRatings();

private:
	CButton _btnImport;

	std::unordered_map<int, KinopoiskInfo> _idPath;
	volatile int _curProxy;
	CString _proxyTitle;
	volatile bool _stop;
	std::vector<int> _ids;

private:
	int _startID;
	int _numImportID;

	CString _progressTitle;
	int _filmID;
	int _year;
	CString _genre;
	CString _nameRU;
	CString _nameEN;
	CString _tagline;
	CString _description;

	CEdit _editProgress;
	double _rating;

	CListBox _refillList;

	int _stopOnError;

	Curl::Global _curlGlobalInit;

	friend void ThreadLoadInfo( CKinopoiskUpdateDlg * dlg );

	friend void ThreadUpdateRatings( CKinopoiskUpdateDlg * dlg );
};
