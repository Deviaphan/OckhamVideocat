// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "VividTree.h"
#include ".\vividtree.h"
#include "GlobalSettings.h"
#include "MainFrm.h"
#include "LoadPng.h"
#include "SetWin10Theme.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Gradient Filling Helper Routine
void GradientFillRect( CDC *pDC, CRect &rect, COLORREF col_from, COLORREF col_to )
{
	TRIVERTEX vert[2];
	GRADIENT_RECT mesh;

	vert[0].x = rect.left;
	vert[0].y = rect.top;
	vert[0].Alpha = 0xff00;
	vert[0].Blue = GetBValue( col_from ) << 8;
	vert[0].Green = GetGValue( col_from ) << 8;
	vert[0].Red = GetRValue( col_from ) << 8;

	vert[1].x = rect.right;
	vert[1].y = rect.bottom;
	vert[1].Alpha = 0xf000;
	vert[1].Blue = GetBValue( col_to ) << 8;
	vert[1].Green = GetGValue( col_to ) << 8;
	vert[1].Red = GetRValue( col_to ) << 8;

	mesh.UpperLeft = 0;
	mesh.LowerRight = 1;

	pDC->GradientFill( vert, 2, &mesh, 1, GRADIENT_FILL_RECT_H  );
}



// CTreeCtrlEx

IMPLEMENT_DYNAMIC( VividTree, CTreeCtrl )

VividTree::VividTree()
	: _gdi(nullptr)
{
}


VividTree::~VividTree()
{
	delete _gdi;
}


BEGIN_MESSAGE_MAP( VividTree, CTreeCtrl )
	ON_NOTIFY_REFLECT( TVN_ITEMEXPANDING, &VividTree::OnTvnItemexpanding )
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


BOOL VividTree::OnEraseBkgnd( CDC* )
{
	return FALSE;
}

void VividTree::OnPaint()
{
	CPaintDC dc( this );

	GetClientRect( m_rect );

	if( !_bmpBackBuffer.m_hObject )
	{
		_bmpBackBuffer.CreateCompatibleBitmap( &dc, m_rect.Width(), m_rect.Height() );

		_backbuffer.CreateCompatibleDC(&dc );
		_backbuffer.SelectObject( &_bmpBackBuffer );

		_gdi = Gdiplus::Graphics::FromHDC( _backbuffer.GetSafeHdc() );
		_gdi->SetSmoothingMode( Gdiplus::SmoothingMode::SmoothingModeNone );
		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );
		_gdi->SetPixelOffsetMode( Gdiplus::PixelOffsetMode::PixelOffsetModeHighSpeed );
		_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceOver );
		_gdi->SetCompositingQuality( Gdiplus::CompositingQuality::CompositingQualityHighSpeed );
		_gdi->SetTextRenderingHint( Gdiplus::TextRenderingHint::TextRenderingHintClearTypeGridFit );
	}

	SCROLLINFO scroll_info;
	if( GetScrollInfo( SB_HORZ, &scroll_info, SIF_POS | SIF_RANGE ) )
	{
		m_h_offset = -scroll_info.nPos;
		m_h_size = max( scroll_info.nMax + 1, m_rect.Width() );
	}
	else
	{
		m_h_offset = m_rect.left;
		m_h_size = m_rect.Width();
	}
	if( GetScrollInfo( SB_VERT, &scroll_info, SIF_POS | SIF_RANGE ) )
	{
		if( scroll_info.nMin == 0 && scroll_info.nMax == 100 )
			scroll_info.nMax = 0;
		m_v_offset = -scroll_info.nPos * GetItemHeight();
		m_v_size = max( (scroll_info.nMax + 2)*((int)GetItemHeight() + 1), m_rect.Height() );
	}
	else
	{
		m_v_offset = m_rect.top;
		m_v_size = m_rect.Height();
	}


	DrawBackGround();
	DrawItems();

	// Now Blt the changes to the real device context - this prevents flicker.
	dc.BitBlt( m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), &_backbuffer, 0, 0, SRCCOPY );
}

void VividTree::DrawBackGround()
{
	if( !GetGlobal().theme )
		return;

	if( !GetGlobal().theme->bgImage )
		return;

	const double width = GetGlobal().theme->bgImage->GetWidth();
	const double height = GetGlobal().theme->bgImage->GetHeight();

	const double w = width / GetGlobal().bgFullSize.Width();
	const double h = height / GetGlobal().bgFullSize.Height();

	CRect clientRect;
	GetWindowRect( clientRect );
	if( GetParent() )
		GetParent()->ScreenToClient( clientRect );

	const CRect & treeRect = GetGlobal().bgTree;
	Gdiplus::Rect destRect( treeRect.left, treeRect.top - clientRect.top, treeRect.Width(), treeRect.Height() );

	const Gdiplus::InterpolationMode lastMode = _gdi->GetInterpolationMode();
	_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );

	_gdi->DrawImage(
		GetGlobal().theme->bgImage,
		destRect,
		(INT)0, (INT)0, (INT)(int)(treeRect.Width() * w), (INT)(treeRect.Height() * h),
		Gdiplus::Unit::UnitPixel );

	_gdi->SetInterpolationMode( lastMode );
}

void VividTree::DrawItems( )
{
	HTREEITEM currentItem = GetFirstVisibleItem();
	if( currentItem == nullptr )
		return;

	const TreeStyle & style = GetGlobal().theme->tree;

	Gdiplus::ImageAttributes iaNormal;
	{
		Gdiplus::ColorMatrix cm;
		ZeroMemory( &cm.m, sizeof( cm ) );
		cm.m[0][0] = GetRValue( style.textColor ) / 255.0f; // r
		cm.m[1][1] = GetGValue( style.textColor ) / 255.0f; // g
		cm.m[2][2] = GetBValue( style.textColor ) / 255.0f; // b
		cm.m[3][3] = 1.0f; // a
		cm.m[4][4] = 1.0f;

		iaNormal.SetColorMatrix( &cm );
	}

	Gdiplus::ImageAttributes iaSelected;
	{
		Gdiplus::ColorMatrix cm;
		ZeroMemory( &cm.m, sizeof( cm ) );
		cm.m[0][0] = GetRValue( style.textColorSelected ) / 255.0f; // r
		cm.m[1][1] = GetGValue( style.textColorSelected ) / 255.0f; // g
		cm.m[2][2] = GetBValue( style.textColorSelected ) / 255.0f; // b
		cm.m[3][3] = 1.0f; // a
		cm.m[4][4] = 1.0f;

		iaSelected.SetColorMatrix( &cm );
	}

	do
	{
		const int state = GetItemState( currentItem, TVIF_STATE );
		HTREEITEM parent = GetParentItem( currentItem );
		const bool has_children = ItemHasChildren( currentItem ) || parent == nullptr;
		const bool selected = (state & TVIS_SELECTED) != 0;

		CRect rc_item;
		if( GetItemRect( currentItem, rc_item, TRUE ) )
		{
			if( selected )
			{
				Gdiplus::Rect destRect;
				destRect.X = 0;
				destRect.Y = rc_item.top;
				destRect.Width = GetGlobal().bgTree.Width();
				destRect.Height = rc_item.Height();

				Gdiplus::SolidBrush brush( MakeColor( style.textColor ) );
				_gdi->FillRectangle( &brush, destRect );
			}

			if( has_children )
			{
				Gdiplus::RectF destArrow, srcArrow;

				srcArrow.Width = srcArrow.Height = 32;
				if( state & TVIS_EXPANDED )
					srcArrow.X = 32;				

				destArrow.X = rc_item.left - style.fontSize;
				destArrow.Y = rc_item.top;
				destArrow.Width = destArrow.Height = style.fontSize;

				_gdi->DrawImage( _icons.get(), destArrow, srcArrow, Gdiplus::Unit::UnitPixel, selected ? &iaSelected : &iaNormal );
			}

			// иконка папки
			{
				Gdiplus::RectF destArrow, srcArrow;

				srcArrow.Width = srcArrow.Height = 32;
				srcArrow.X = 64;

				destArrow.X = rc_item.left;
				destArrow.Y = rc_item.top;
				destArrow.Width = destArrow.Height = style.fontSize;

				_gdi->DrawImage( _icons.get(), destArrow, srcArrow, Gdiplus::Unit::UnitPixel );
			}

			Gdiplus::SolidBrush brush( MakeColor( selected ? style.textColorSelected : style.textColor ) );
			const CString str = GetItemText( currentItem );
			Gdiplus::PointF rect;
			rect.X = rc_item.left + style.fontSize;
			rect.Y = rc_item.top;
			_gdi->DrawString( str.GetString(), str.GetLength(), style.font, rect, nullptr, &brush );
		}

		currentItem = GetNextVisibleItem( currentItem );
	}
	while( currentItem );
}


void VividTree::OnTvnItemexpanding( NMHDR *, LRESULT *pResult )
{
	Invalidate(FALSE);
	*pResult = 0;
}

void VividTree::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	Invalidate(FALSE);
	CTreeCtrl::OnVScroll( nSBCode, nPos, pScrollBar );
}


void VividTree::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	Invalidate( FALSE );
	CTreeCtrl::OnHScroll( nSBCode, nPos, pScrollBar );
}


BOOL VividTree::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	Invalidate(FALSE);
	return CTreeCtrl::OnMouseWheel( nFlags, zDelta, pt );
}

void VividTree::OnSize( UINT nType, int cx, int cy )
{
	delete _gdi;
	_gdi = nullptr;
	_backbuffer.DeleteDC();
	_bmpBackBuffer.DeleteObject();

	CTreeCtrl::OnSize( nType, cx, cy );

	((CMainFrame*)GetParentFrame())->UpdateViewSizes();

	Invalidate(FALSE);
}

void VividTree::Initialize()
{
	_icons.reset( LoadPng( IDB_TREE_ICONS ) );

}
