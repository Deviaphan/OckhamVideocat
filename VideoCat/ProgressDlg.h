#pragma once

#define UPDATE_PROGRESS (WM_USER + 1)

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	explicit CProgressDlg(CWnd* pParent=nullptr);
	~CProgressDlg() override;

	static const DWORD IDD;

	void SetTitle( const CString & title );
	void SetProgressCount( int total );
	void SetCurrentStep( int step );
	void NextStep();

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnUpdateProgress( WPARAM wParam, LPARAM lParam );

	void UpdateProgress();

private:
	CProgressCtrl _progressBar;
	CString _progressTitle;
	CString _progressStep;

	int _curStep;
	int _total;
protected:
};


class ProgressController
{
public:
	ProgressController();
	~ProgressController();

	void Create( const CString & title );
	void Destroy();
	void SetTitle( const CString & title );
	void SetProgressCount( int total );
	void SetCurrentStep( int step );
	void NextStep();
	HWND GetHWND()
	{
		return _bar->m_hWnd;
	}

public:

private:
	CProgressDlg * _bar;
};
