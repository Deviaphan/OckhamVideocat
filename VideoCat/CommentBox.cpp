// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CommentBox.h"

IMPLEMENT_DYNAMIC(CCommentBox, CEdit)

CCommentBox::CCommentBox()
: _front( RGB(0,0,0) )
, _back( RGB(255,255,225) )
{
	_brush.CreateSolidBrush( _back );
}

CCommentBox::~CCommentBox()
{
}


BEGIN_MESSAGE_MAP(CCommentBox, CEdit)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CCommentBox::SetText( const CString & text )
{
	SetWindowText(text);
}

void CCommentBox::SetFontColor( COLORREF rgb )
{
	_front = rgb;
}

void CCommentBox::SetBackColor( COLORREF rgb )
{
	_back = rgb;
	_brush.CreateSolidBrush( _back );
}

HBRUSH CCommentBox::CtlColor( CDC * pDC, UINT nCtlColor )
{
	pDC->SetTextColor( _front );
	pDC->SetBkColor( _back );
	return _brush;
}

