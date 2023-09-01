#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <vector>
#include <memory>

struct FilmInfo
{
	CString path;
	CString hash;
};

class CThumbsDlg : public CDialog
{
public:
	CThumbsDlg( CWnd* pParent = nullptr );

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_THUMBS_DIALOG
	};
#endif

public:
	bool ProcessFilms();
	bool ProcessFilm();
	void CleanThread();
	bool Autoselect() const
	{
		return _autoselect;
	}
	void UpdateFrameList();
	void DrawSelected();
	void CheckButton();

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK()
	{}

	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnItemchangedListFrames( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBtnCreateThumbs();
	afx_msg void OnBtnSkip();


protected:
	HICON m_hIcon;
	CEdit _editCtrl;
	CProgressCtrl _progressCtrl;
	CListBox _listCtrl;

	int _currentStep;
	int _totalSteps;

public:
	CString _filePath;
	CString _thumbsDir;

protected:
	std::vector<FilmInfo> _films;
	CListCtrl _listFrames;
	CImageList _imageListThumb;

	std::vector< std::shared_ptr<CImage> > _frameList;
	std::vector<unsigned> _selectedImages;

	HANDLE _workingThread;
	int _units;
	int _autoselect;

	int _selectedItem;
	bool _skip;
};
