#pragma once

class CXPGroupBox : public CButton
{
	DECLARE_DYNAMIC( CXPGroupBox );

public:
	CXPGroupBox();

	enum XPGroupBoxStyle
	{
		XPGB_FRAME,
		XPGB_WINDOW
	};

public:
	virtual CXPGroupBox& SetBorderColor( COLORREF clrBorder );
	virtual CXPGroupBox& SetCatptionTextColor( COLORREF clrText );
	virtual CXPGroupBox& SetBackgroundColor( COLORREF clrBKClient );
	virtual CXPGroupBox& SetBackgroundColor( COLORREF clrBKTilte, COLORREF clrBKClient );
	virtual CXPGroupBox& SetXPGroupStyle( XPGroupBoxStyle eStyle );

	virtual CXPGroupBox& SetText( LPCTSTR lpszTitle );
	virtual CXPGroupBox& SetFontBold( BOOL bBold );
	virtual CXPGroupBox& SetFontName( const CString& strFont, BYTE byCharSet = ANSI_CHARSET );
	virtual CXPGroupBox& SetFontUnderline( BOOL bSet );
	virtual CXPGroupBox& SetFontItalic( BOOL bSet );
	virtual CXPGroupBox& SetFontSize( int nSize );
	virtual CXPGroupBox& SetFont( LOGFONT lf );

	virtual CXPGroupBox& SetAlignment( DWORD dwType );

public:
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = nullptr );

protected:
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
	virtual void PreSubclassWindow();

public:
	virtual ~CXPGroupBox();

protected:
	void UpdateSurface();
	void ReconstructFont();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();

private:
	CString m_strTitle;

	COLORREF m_clrBorder;
	COLORREF m_clrTitleBackground;
	COLORREF m_clrClientBackground;
	COLORREF m_clrTitleText;

	XPGroupBoxStyle m_nType;
	DWORD m_dwAlignment;

	LOGFONT m_lf;
	CFont m_font;

};
