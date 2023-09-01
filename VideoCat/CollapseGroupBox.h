#pragma once
#include <vector>
/*
//////////////////////////////////
// add this function to parentWndow  //
//////////////////////////////////
//////////////////////////////////
BOOL ParentWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == UM_UPDATEFORM)
	{// groupbox changed his state (Header-Only Style / Full State) 
	
	}
	if (wParam == UM_MOUSELEAVE)
	{// mouse leave groupbox
		
	}
	if (wParam == UM_MOUSEENTRY)
	{// mouse come to groupbox
		
	}

	return ownerParentWnd::OnCommand(wParam, lParam);
}
*/

/* caption variants
SS_LEFT
SS_CENTER
SS_RIGHT
*/
#define UM_MOUSEENTRY WM_USER + 0x100
#define UM_MOUSELEAVE WM_USER + 0x101
#define UM_UPDATEFORM WM_USER + 0x102

class CollapseGroupBox : public CButton
{
	DECLARE_DYNAMIC(CollapseGroupBox);

public:
	std::vector<CWnd*> intoElements; // vector of groupbox's components
	bool		m_frameMinimized; // Header-Only Style
	CRect		rectTitle; // Caption rect
	CollapseGroupBox();
	virtual ~CollapseGroupBox();
	void setBGColor(COLORREF bgColor); // use it if your BGColor is not COLOR_BTNFACE
	virtual CollapseGroupBox& SetBorderColor(COLORREF clrBorder);
	virtual CollapseGroupBox& SetCatptionTextColor(COLORREF clrText);
	virtual CollapseGroupBox& SetBackgroundColor(COLORREF clrBKTilte);
	virtual CollapseGroupBox& SetText(LPCTSTR lpszTitle);
	virtual CollapseGroupBox& SetFontBold(BOOL bBold);
	virtual CollapseGroupBox& SetFontName(const CString& strFont, BYTE byCharSet = ANSI_CHARSET);
	virtual CollapseGroupBox& SetFontUnderline(BOOL bSet);
	virtual CollapseGroupBox& SetFontItalic(BOOL bSet);
	virtual CollapseGroupBox& SetFontSize(int nSize);
	virtual CollapseGroupBox& SetFont(LOGFONT lf);
	virtual CollapseGroupBox& SetAlignment(DWORD dwType);
	virtual CollapseGroupBox& SetFrameExpand(bool boxToLine);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void RefreshCtrl();
protected:
	virtual void PreSubclassWindow();
	void UpdateSurface();
	void ReconstructFont();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	CDC m_BackgroundDC;	
	CRect		rectButton;
	CRect		rectContent; 
	CString		m_strTitle; 
	COLORREF	m_clrBorder;
	COLORREF	m_clrTitleBackground;
	COLORREF	m_clrTitleBackgroundFree;
	COLORREF	m_clrTitleBackgroundHighLight;
	COLORREF	m_clrClientBackground;
	COLORREF	m_clrTitleText;
	COLORREF	m_clrTitleArrow;
	DWORD       m_dwAlignment;
	LOGFONT		m_lf;
	CFont		m_font;
	bool m_bMemDCCreated;
	BOOL IsInGroupBox(CWnd* pCtrl) const;
	void GetGroupBoxElements();
	void ShowElements(bool bShow);
};