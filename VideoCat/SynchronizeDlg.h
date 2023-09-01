#pragma once
#include <vector>
#include <map>
#include "AdvancedToolTipCtrl.h"
#include "PluginManager.h"

struct SyncData
{
	explicit SyncData( const CString & str = CString() )
		: path( str )
		, allow( TRUE )
		, forceIgnore( FALSE )
	{}

	CString path;
	BOOL allow;
	BOOL forceIgnore; // специальный флаг, отмечающий имеющиеся папки, чтобы проигнорировать пользовательский флажок, при их выборе
};

class CSynchronizeDlg : public CDialog
{
	DECLARE_DYNAMIC(CSynchronizeDlg)

public:
	CSynchronizeDlg(CWnd* pParent = nullptr);
	virtual ~CSynchronizeDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYNCHRONIZE };
#endif

public:
	PluginID GetAutofillPlugin();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	virtual void OnOK();

	void RebuildTree();
	void SortItem( HTREEITEM item );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNewFilms();
	afx_msg void OnLostFilms();
	afx_msg void OnTvnItemChangedFileTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );

	void ChangeStates( HTREEITEM startItem, HTREEITEM currentItem, BOOL state );
	void SetParentStates( HTREEITEM currentItem );

private:
	AdvancedToolTipCtrl _tooltip;

	CTreeCtrl _fileTree;
	BOOL _showNewFilms;
	bool lockRecurse;
	CComboBox _pluginList;
	int _pluginId;

	std::map<CString, HTREEITEM> _pathToItem;
	int _minX;
	int _minY;

public:
	std::vector<SyncData> newFilms;
	std::vector<SyncData> lostedFilms;
	BOOL autofillInfo;
	BOOL fillTechinfo;
	BOOL generateThumbs;
};
