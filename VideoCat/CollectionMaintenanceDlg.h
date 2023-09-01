#pragma once

class CollectionMaintenanceDlg : public CDialog
{
	DECLARE_DYNAMIC(CollectionMaintenanceDlg)

public:
	CollectionMaintenanceDlg(CWnd* pParent = nullptr);
	virtual ~CollectionMaintenanceDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINTENANCE };
#endif

	void AddString( const CString & line );

protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	CListBox _stageList;
};
