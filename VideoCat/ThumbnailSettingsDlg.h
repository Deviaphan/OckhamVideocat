#pragma once
#include "XPGroupBox.h"
#include "CommentBox.h"

class ThumbnailSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(ThumbnailSettingsDlg)

public:
	ThumbnailSettingsDlg(CWnd* pParent = nullptr);
	virtual ~ThumbnailSettingsDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THUMBNAIL_SETTINGS };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();

private:
	CXPGroupBox _groupCreate;
	CXPGroupBox _groupFrame;
	CXPGroupBox _groupSize;
	CCommentBox _commentBox;

public:
	int createMode;
	int selectMode;
	int sizeMode;
};
