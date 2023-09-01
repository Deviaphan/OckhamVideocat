// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CommentButton.h"
#include "IconManager.h"

IMPLEMENT_DYNAMIC( CommentButton, CMFCButton )

CommentButton::CommentButton()
{
}

CommentButton::~CommentButton()
{
}
BEGIN_MESSAGE_MAP( CommentButton, CMFCButton )
END_MESSAGE_MAP()

void CommentButton::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
	ENSURE( lpDIS != nullptr );
	ENSURE( lpDIS->CtlType == ODT_BUTTON );

	const BOOL isDisabled = !IsWindowEnabled();

	CDC * pDC = CDC::FromHandle( lpDIS->hDC );
	//ASSERT_VALID( pDCPaint );

	CRect rect = lpDIS->rcItem;

	//CMemDC memDC( *pDCPaint, this );
	//CDC * pDC = &memDC.GetDC();
	
	OnFillBackground( pDC, rect );
	OnDrawBorder( pDC, rect, lpDIS->itemState );

	if( (lpDIS->itemState & ODS_FOCUS) && m_bDrawFocus )
	{
		OnDrawFocusRect( pDC, rect );
	}

	rect.InflateRect( -6, -12, -6, -6 );

	CString strText;
	GetWindowText( strText );

	CFont titleFont;
	CFont commentFont;
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

		logFont.lfHeight = 18;
		logFont.lfWeight = FW_BOLD;
		logFont.lfQuality = PROOF_QUALITY;
		titleFont.CreateFontIndirect( &logFont );

		logFont.lfHeight = 16;
		logFont.lfWeight = FW_NORMAL;
		commentFont.CreateFontIndirect( &logFont );
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
	OnDrawText( pDC, rect, strText, DT_CENTER | DT_TOP | DT_SINGLELINE, lpDIS->itemState );

	rect.InflateRect( -4, -36, -4, -4 );

	pDC->SelectObject( &commentFont );
	pDC->SetTextColor( RGB( 85, 85, 85 ) );
	OnDrawText( pDC, rect, comment, DT_LEFT | DT_TOP | DT_WORDBREAK, lpDIS->itemState );

	pDC->SelectObject( pOldFont );
}