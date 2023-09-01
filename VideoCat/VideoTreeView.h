#pragma once

#include <map>

#include "VideoCatDoc.h"
#include "AdvancedMenu.h"
#include "VividTree.h"
#include "Entry.h"
#include "ListBoxCH.h"
#include "resource.h"


class CVideoTreeView : public CFormView
{
	DECLARE_DYNCREATE(CVideoTreeView)

protected:
	CVideoTreeView();
	~CVideoTreeView() override;

public:
	enum {
		IDD = IDD_VIDEOTREEVIEW
	};

#ifdef _DEBUG
	void AssertValid() const override;
#ifndef _WIN32_WCE
	void Dump(CDumpContext& dc) const override;
#endif
#endif
	void InitializeTree();

	CVideoCatDoc* GetDocument() const;
	void SelectItem( Entry * item );
	Entry * GetCurrentItem();
	Entry * GetParentItem( Entry * item );
	void GoBack();

	Entry* GetTreeEntry( HTREEITEM hitem );

	void UpdateCollectionList();
	void ClearTree();
	void RebuildTree();

	void UpdateView();
	void SetDarkTheme( bool dark );

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnCommand( WPARAM wParam, LPARAM lParam ) override;


	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg HBRUSH OnCtlColor( CDC * pDC, CWnd * pWnd, UINT nCtlColor );
	afx_msg void OnBnClickedHelp();
	afx_msg void OnBnClickedMainMenu();
	afx_msg void OnBnClickedOpenFilter();
	afx_msg void OnBnClickedViewSettings();
	afx_msg void OnBtnKeyboard();
	afx_msg void OnChangeFindByName();
	afx_msg void OnChangedTreeItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu( CWnd* /*pWnd*/, CPoint /*point*/ );
	afx_msg void OnLbnSelchangeDatabaseList();
	afx_msg void OnLbnSelchangeRootList();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnCollectionMenu( CWnd * pWnd, CPoint point );
	afx_msg void OnRootMenu( CWnd * pWnd, CPoint point );

private:
	CListBoxCH _collectionList;
	CListBoxCH _rootList;

	VividTree _tree;

	CEdit _findByName;
	CMFCButton _btnOpenFilter;
	CMFCButton _btnMainMenu;
	CMFCButton _btnHelp;
	CMFCButton _btnViewSettings;
	CMFCButton _btnKeyboard;

	CString _nameFilter;
	Entry * _activeMenuItem;
	bool _isRoot;

	std::map< HTREEITEM, Entry* > _treeMap;

	CEdit _infoBox;
	CEdit _infoBoxValues;
	CString _info;

	int _lastUsedDB;

public:

	CBrush _ctrlBrush;
	COLORREF _bgColor;
	COLORREF _textColor;
};

#ifndef _DEBUG
inline CVideoCatDoc* CVideoTreeView::GetDocument() const
{ return reinterpret_cast<CVideoCatDoc*>(m_pDocument); }
#endif

