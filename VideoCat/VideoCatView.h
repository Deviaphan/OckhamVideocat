#pragma once
#include "VideoCatDoc.h"
#include <memory>
#include <vector>
#include "ScrollHelper.h"
#include <mutex>

class ItemStyle;

class CVideoCatView : public CView
{
protected:
	enum ActiveItemType
	{
		AIT_NONE = 0,
		AIT_PLAY,
		AIT_MENU,
		AIT_DESCRIPTION,
		AIT_TAG,
		AIT_TECHINFO,
		AIT_YOUTUBE,
		AIT_THUMBS,

		AIT_TOOLBAR = 300, //временно тут

		AIT_force32 = 0xFFFFFF
	};

protected:
	CVideoCatView();
	DECLARE_DYNCREATE(CVideoCatView)
public:
	~CVideoCatView() override;

public:
	CVideoCatDoc* GetDocument() const;
	void RecalcScrollLayout(BOOL scrollTop = FALSE);
	void SelectFirst();
	void RefreshDisplayList( Entry * selectFileInfo = nullptr );
	void SetListView( bool onoff )
	{
		_listView = onoff;
	}
	bool IsListView() const
	{
		return _listView;
	}

	void UpdateView();
	void SetDarkTheme( bool dark );

public:
	void OnDraw(CDC* pDC) override;

protected:
	friend void BuildThumbPreviewThread( CVideoCatView * view );

	void BuildShortInfo( const Entry & entry, std::wstring & result );
	ActiveItemType TestItemButtons( const Entry & entry, const ItemStyle & style, const CRect & itemRect, const CPoint & point );

	void DrawTiles( const CRect & clientRect, const CPoint & pos );
	void DrawList( const CRect & clientRect, const CPoint & pos );

	void DrawToolbar( const CRect & clientRect );

	void DrawBackground( const CRect & clientRect );
	void DrawYear( const Entry * entry, int posX, int posY );
	void DrawEpisode( const Entry * entry, int posX, int posY );
	void DrawName( const Entry * entry, CRect & rect, bool rusFirst, UINT flags, const ItemStyle & style, bool advanced );
	void DrawFolder( const DisplayItem & item, int posX, int posY );
	void DrawListFolder( const DisplayItem & item, const Gdiplus::Rect & rect );
	void DrawThumbPreview( const CRect & thumbRect );
	void DrawDescription( const CRect & clientRect, const CRect & destRect, const DisplayItem & item );
	void DrawTag( const CRect & clientRect, const CRect & destRect, const DisplayItem & item );
	void DrawTechInfo( const CRect & clientRect, const CRect & destRect, const DisplayItem & item );
	void DrawTextBox( const CRect & clientRect, const CRect & itemRect, const std::wstring & headerText, const std::wstring & bodyText, const std::wstring & footerText );
	void DrawItemButtons( const DisplayItem & item, const CRect & selectedItemRect, bool listButton );
	void DrawGlass( const Gdiplus::Rect & itemRect, const ItemStyle & style );
	void BuildThumbsPreview();

protected:
	void OnInitialUpdate() override;
	BOOL OnCommand( WPARAM wParam, LPARAM lParam ) override;
	void PostNcDestroy() override;
	BOOL PreCreateWindow( CREATESTRUCT & cs ) override;

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	void SetCurrentItem( int index );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar * pScrollBar );

private:
	int GetItemIndex( const CPoint & point ) const;

	void GetScreenRect( CRect & r ) const;

	int ScaleInt( int v ) const;
	float ScaleFloat( float v ) const;
	CPoint & ScalePoint( CPoint & p ) const;
	CRect & ScaleRect( CRect & r ) const;

private:
	CScrollHelper _scrollHelper;

	int _pageScale;

	int _numTotal;	///< Всего объектов
	int _numHorz;	///< Количество объектов, видимых по горизонтали
	int _numVert;	///< Количество объектов, видимых по вертикали

	int _currentItem;

	std::unique_ptr<Gdiplus::Bitmap> _bmpRealFolder;
	std::unique_ptr<Gdiplus::Bitmap> _bmpLockedFolder;
	std::unique_ptr<Gdiplus::Bitmap> _bmpVirtualFolder;
	std::unique_ptr<Gdiplus::Bitmap> _bmp10;
	std::unique_ptr<Gdiplus::Bitmap> _bgDescription;

	std::shared_ptr<Gdiplus::Bitmap> _noPosterTile;
	std::shared_ptr<Gdiplus::Bitmap> _noPosterTileTV;
	std::shared_ptr<Gdiplus::Bitmap> _noPosterList;
	std::shared_ptr<Gdiplus::Bitmap> _noPosterListTV;
	
	// Для двойной буферизации, чтобы не мерцало
	// благодаря вот таким извращениям через буфер для gdi+ и буфер для просто gdi получается получить максимальную производительность
	// Без CDC получаются либо тормоза, либо мерцания. Возможно, есть какая-то хитрость, но лень искать.
	CBitmap _bmpBackBuffer;
	CDC _backbuffer;
	std::unique_ptr<Gdiplus::Bitmap> _gdiBuffer;
	std::unique_ptr<Gdiplus::Graphics> _gdi;

	CPoint _downPos;

	bool _drawThumbBox;
	std::unique_ptr<Gdiplus::Bitmap> _thumbs;
	std::unique_ptr<Gdiplus::Bitmap> _smallThumbs;

	int _thumbIndex;
	bool _forcedIndex;

	std::unique_ptr<Gdiplus::Bitmap> _itemShadow;

	ActiveItemType _ait;

	bool _listView;

	int _thumbsWidth;
	int _thumbsHeight;

	std::mutex _mutex;
};


#ifndef _DEBUG
inline CVideoCatDoc* CVideoCatView::GetDocument() const
   { return reinterpret_cast<CVideoCatDoc*>(m_pDocument); }
#endif

