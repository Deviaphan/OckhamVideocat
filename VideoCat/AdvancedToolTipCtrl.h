#pragma once

#include <vector>

class TooltipItem
{
public:
	TooltipItem()
		: wnd( nullptr )
		, hicon( nullptr )
	{}

public:
	CWnd * wnd; // дескриптор окна, для которого создаётся всплывашка
	CString title; // заголовок, если есть
	CString description; // описание, если есть
	HICON hicon; // иконка, если есть
};

class AdvancedToolTipCtrl : public CMFCToolTipCtrl
{
public:
	AdvancedToolTipCtrl();
	
public:
	void AddTooltip( CWnd * wnd, const CString & title, const CString & description = CString(), HICON icon = nullptr );
	void EditTooltip( CWnd * wnd, const CString & title, const CString & description = CString(), HICON icon = nullptr );

	virtual CSize GetIconSize();
	virtual BOOL OnDrawIcon( CDC* pDC, const CRect & rectImage );

public:
	virtual ~AdvancedToolTipCtrl();

protected:
	afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

public:
	std::vector<TooltipItem> _tooltips;
	CWnd * _currentControl;
	int _currentItem;
};
