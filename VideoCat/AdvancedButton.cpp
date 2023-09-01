// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AdvancedButton.h"
#include "IconManager.h"

IMPLEMENT_DYNAMIC( AdvancedButton, CMFCButton )

AdvancedButton::AdvancedButton()
{
}

AdvancedButton::~AdvancedButton()
{
}
BEGIN_MESSAGE_MAP( AdvancedButton, CMFCButton )
END_MESSAGE_MAP()

void AdvancedButton::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
	/*
	ENSURE( lpDIS != nullptr );
	ENSURE( lpDIS->CtlType == ODT_BUTTON );

	const BOOL isDisabled = !IsWindowEnabled();

	CDC * pDCPaint = CDC::FromHandle( lpDIS->hDC );
	ASSERT_VALID( pDCPaint );

	CMemDC memDC( *pDCPaint, this );
	CDC * pDC = &memDC.GetDC();

	CRect rect = lpDIS->rcItem;

	OnFillBackground( pDC, rect );
	OnDrawBorder( pDC, rect, lpDIS->itemState );

	CRect rectText = rect;
	rectText.left += 40;
	rectText.top += 4;

	{
		Gdiplus::Graphics gdi( pDC->GetSafeHdc() );

		GetIconManager().Draw( gdi, _iconID, rect.left+4, rect.top+4, 32, isDisabled ? RGB(127,127,127) : RGB( 32, 32, 32 ) );
	}

	CString strText;
	GetWindowText( strText );

	// Draw text:
	CFont titleFont;
	{
		CFont * font = nullptr;
		if( m_hFont )
		{
			font = CFont::FromHandle( m_hFont );
		}
		else
		{
			font = CFont::FromHandle( (HFONT)::GetStockObject( DEFAULT_GUI_FONT ) );
		}

		LOGFONT logFont;
		font->GetLogFont( &logFont );

		logFont.lfHeight = 12;
		logFont.lfWeight = 400;
		titleFont.CreateFontIndirect( &logFont );
	}

	CFont * pOldFont = pDC->SelectObject( &titleFont );
	ENSURE( pOldFont != nullptr );

	pDC->SetBkMode( TRANSPARENT );
	COLORREF clrText = (m_clrRegular == (COLORREF)-1) ? GetGlobalData()->clrBtnText : m_clrRegular;

	if( m_bHighlighted && m_clrHover != (COLORREF)-1 )
	{
		clrText = m_clrHover;
	}
	else if( isDisabled )
	{
		clrText = RGB( 127, 127, 127 );
	}

	pDC->SetTextColor( clrText );
	OnDrawText( pDC, rectText, strText, DT_LEFT | DT_VCENTER | DT_SINGLELINE, lpDIS->itemState );

	if( (lpDIS->itemState & ODS_FOCUS) && m_bDrawFocus )
	{
		OnDrawFocusRect( pDC, rect );
	}*/

	CMFCButton::DrawItem( lpDIS );

	{
		const BOOL isDisabled = !IsWindowEnabled();

		CDC * pDCPaint = CDC::FromHandle( lpDIS->hDC );

		CMemDC memDC( *pDCPaint, this );
		CDC * pDC = &memDC.GetDC();

		Gdiplus::Graphics gdi( pDC->GetSafeHdc() );

		CRect rect = lpDIS->rcItem;

		GetIconManager().Draw( gdi, _iconID, rect.left + 4, (rect.Height()-32)/2, 32, isDisabled ? RGB( 127, 127, 127 ) : RGB( 32, 32, 32 ) );
	}
}


IMPLEMENT_DYNAMIC( AdvancedMenuButton, CMFCMenuButton )

AdvancedMenuButton::AdvancedMenuButton()
{
}

AdvancedMenuButton::~AdvancedMenuButton()
{
}
BEGIN_MESSAGE_MAP( AdvancedMenuButton, CMFCMenuButton )
END_MESSAGE_MAP()

void AdvancedMenuButton::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{/*
	ENSURE( lpDIS != nullptr );
	ENSURE( lpDIS->CtlType == ODT_BUTTON );

	const BOOL isDisabled = !IsWindowEnabled();

	CDC * pDCPaint = CDC::FromHandle( lpDIS->hDC );
	ASSERT_VALID( pDCPaint );

	CMemDC memDC( *pDCPaint, this );
	CDC * pDC = &memDC.GetDC();

	CRect rect = lpDIS->rcItem;

	OnFillBackground( pDC, rect );
	OnDrawBorder( pDC, rect, lpDIS->itemState );

	CRect rectText = rect;
	rectText.left += 40;
	rectText.top += 4;

	{
		Gdiplus::Graphics gdi( pDC->GetSafeHdc() );

		GetIconManager().Draw( gdi, _iconID, rect.left + 4, rect.top + 4, 32, isDisabled ? RGB( 127, 127, 127 ) : RGB( 32, 32, 32 ) );
	}

	CString strText;
	GetWindowText( strText );

	// Draw text:
	CFont titleFont;
	{
		CFont * font = nullptr;
		if( m_hFont )
		{
			font = CFont::FromHandle( m_hFont );
		}
		else
		{
			font = CFont::FromHandle( (HFONT)::GetStockObject( DEFAULT_GUI_FONT ) );
		}

		LOGFONT logFont;
		font->GetLogFont( &logFont );

		logFont.lfHeight = 12;
		logFont.lfWeight = 400;
		titleFont.CreateFontIndirect( &logFont );
	}

	CFont * pOldFont = pDC->SelectObject( &titleFont );
	ENSURE( pOldFont != nullptr );

	pDC->SetBkMode( TRANSPARENT );
	COLORREF clrText = (m_clrRegular == (COLORREF)-1) ? GetGlobalData()->clrBtnText : m_clrRegular;

	if( m_bHighlighted && m_clrHover != (COLORREF)-1 )
	{
		clrText = m_clrHover;
	}
	else if( isDisabled )
	{
		clrText = RGB( 127, 127, 127 );
	}

	pDC->SetTextColor( clrText );
	OnDrawText( pDC, rectText, strText, DT_LEFT | DT_VCENTER | DT_SINGLELINE, lpDIS->itemState );

	if( (lpDIS->itemState & ODS_FOCUS) && m_bDrawFocus )
	{
		OnDrawFocusRect( pDC, rect );
	}
	*/

	CMFCMenuButton::DrawItem( lpDIS );

	{
		const BOOL isDisabled = !IsWindowEnabled();

		CDC * pDCPaint = CDC::FromHandle( lpDIS->hDC );

		CMemDC memDC( *pDCPaint, this );
		CDC * pDC = &memDC.GetDC();

		Gdiplus::Graphics gdi( pDC->GetSafeHdc() );

		CRect rect = lpDIS->rcItem;

		GetIconManager().Draw( gdi, _iconID, rect.left + 4, rect.top + 4, 32, isDisabled ? RGB( 127, 127, 127 ) : RGB( 32, 32, 32 ) );
	}

	//__super::DrawMenuBar();
}
