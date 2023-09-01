#pragma once

#include <atlimage.h>
#include "VideoInput/videoInput.h"
#include "ImageData.h"


// CameraViewDlg dialog

class CameraViewDlg : public CDialog
{
	DECLARE_DYNAMIC(CameraViewDlg)

public:
	CameraViewDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CameraViewDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMERA_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnPaint();
	virtual void OnOK();
	virtual void OnCancel();


	void StartCapture();
	void StopCapture();

public:
	virtual BOOL OnInitDialog();

	CButton _btnOK;
	static int cameraFilterType;

private:
	videoInput _vi;
	CImage _image;

	ImageData _imageData;
	const int _numSamples = 8;
	int _curSample = 0;
	int _totalFrames = 0;
	UINT_PTR _timerId;
};
