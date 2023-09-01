#pragma once

#include "resource.h"
#include <memory>

// Helper Routine
extern void GradientFillRect( CDC *pDC, CRect &rect, COLORREF col_from, COLORREF col_to, bool vert_grad );


class VividTree : public CTreeCtrl
{
	DECLARE_DYNAMIC( VividTree )

public:
	VividTree();
	virtual ~VividTree();
	void Initialize();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnPaint();
	afx_msg void OnTvnItemexpanding( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg void OnSize( UINT nType, int cx, int cy );

private:
	void DrawBackGround();	// Background Paint code
	void DrawItems();  // Item Paint Code
	
private:
	CRect m_rect;						// The client rect when drawing
	int   m_h_offset;					// 0... -x (scroll offset)
	int   m_h_size;						// width of unclipped window
	int   m_v_offset;					// 0... -y (scroll offset)
	int   m_v_size;						// height of unclipped window

	CBitmap _bmpBackBuffer;
	CDC _backbuffer;
	Gdiplus::Graphics * _gdi;

	std::unique_ptr<Gdiplus::Bitmap> _icons;

};


