#pragma once

class CUpdaterDlg : public CDialog
{
public:
	CUpdaterDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATER_DIALOG };
#endif

public:
	void SetThread( HANDLE h );
	void CleanThread();

	void Get7zPath();
	void GetParentDir();

	bool DownloadUpdate();
	bool UnpackUpdate();
	bool InstallUpdate();

	void SetDownloadStep( int percent );
	void SetUnpackStep();

	void SetInstallationStep( int curStep );

	static void StartNewUpdater( CString newUpdater, CString updateDir );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

protected:
	HICON m_hIcon;

	CProgressCtrl _progress1;
	CProgressCtrl _progress2;
	CProgressCtrl _progress3;

	HANDLE _workingThread;

	CString _updateZipPath;

	CString _workDir;
	CString _unzipPath;

public:
	int currentStep;

	CString downloadUrl; // url звгрузки архива
	CString unzipDir;
	CString installPath; // папка в которую копировать обновление
};
