// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ListBoxCH.h"
#include "GlobalSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//IMPLEMENT_DYNAMIC( CListBoxCH, CListBox );

CListBoxCH::CListBoxCH()
{
}



BEGIN_MESSAGE_MAP( CListBoxCH, CListBox )
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CListBoxCH::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
	if( !lpDIS )
		return;

	if( lpDIS->itemID >= GetCount() )
		return;

	COLORREF bgColor = RGB( 255, 255, 255 );
	COLORREF textColor = RGB( 0, 0, 0 );

	if( GetGlobal().theme )
	{
		bgColor = GetGlobal().theme->bgTopColor;
		textColor = GetGlobal().theme->tree.textColor;
	}

	CDC * pDC = CDC::FromHandle( lpDIS->hDC );
	if( !pDC )
		return;

	CRect rectFull( lpDIS->rcItem );
	rectFull.top += 1;
	rectFull.bottom -= 1;

	CString text;
	GetText( lpDIS->itemID, text );

	if( lpDIS->itemState & ODS_SELECTED )
	{
		std::swap( bgColor, textColor );
	}
	
	if( lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE) )
	{
		pDC->FillSolidRect( rectFull, bgColor );

		rectFull.left += 8;

		::DrawIconEx( pDC->GetSafeHdc(), rectFull.left, rectFull.top, _icon, 16, 16, 0, nullptr, DI_NORMAL );

		rectFull.left += 22;

		CPen pen( PS_DOT, 1, RGB( 127, 127, 127 ) );
		pDC->SelectObject( &pen );
		pDC->MoveTo( rectFull.left, rectFull.bottom );
		pDC->LineTo( rectFull.right, rectFull.bottom );

		pDC->SetBkColor( bgColor );
		pDC->SetTextColor( textColor );
		pDC->DrawText( text, text.GetLength(), rectFull, DT_LEFT | DT_SINGLELINE | DT_VCENTER );
	}
}

void CListBoxCH::OnContextMenu( CWnd * /*pWnd*/, CPoint point )
{
	::SendMessage( GetParent()->GetSafeHwnd(), WM_CONTEXTMENU, (WPARAM)GetSafeHwnd(), (LPARAM)MAKELONG(point.x, point.y) );
}
