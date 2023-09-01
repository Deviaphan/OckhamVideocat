// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "EditTrans.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEditTrans::CEditTrans()
	: m_TextColor( RGB( 0, 0, 0 ) )
	, m_BackColor( TRANS_BACK )
{
}

CEditTrans::~CEditTrans()
{
}

BEGIN_MESSAGE_MAP( CEditTrans, CEdit )
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT( EN_UPDATE, OnUpdate )
	ON_WM_LBUTTONDOWN()
	ON_CONTROL_REFLECT( EN_KILLFOCUS, OnKillfocus )
END_MESSAGE_MAP()

void CEditTrans::SetBackColor( COLORREF col )
{
	m_BackColor = col;

	m_Brush.DeleteObject();

	UpdateCtrl();
}

HBRUSH CEditTrans::CtlColor( CDC* pDC, UINT /*nCtlColor*/ )
{
	if( !m_Brush.m_hObject )
	{
		m_Brush.CreateSolidBrush( m_BackColor );
	}

	pDC->SetBkColor( m_BackColor );
	pDC->SetTextColor( m_TextColor );

	return (HBRUSH)m_Brush;
}

void CEditTrans::OnKillfocus()
{
	//UpdateCtrl();
}

void CEditTrans::OnUpdate()
{
	//UpdateCtrl();
}

void CEditTrans::OnLButtonDown( UINT nFlags, CPoint point )
{
	//UpdateCtrl();

	CEdit::OnLButtonDown( nFlags, point );
}

void CEditTrans::UpdateCtrl()
{
	//CWnd* pParent = GetParent();
	//CRect rect;

	//GetWindowRect( rect );
	//pParent->ScreenToClient( rect );
	//rect.DeflateRect( 2, 2 );

	//pParent->InvalidateRect( rect, FALSE );
}
