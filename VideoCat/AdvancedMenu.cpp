// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "AdvancedMenu.h"
#include "Commands/CommandExecute.h"
#include <iostream>
#include <memory>
#include "GlobalSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace adv_mfc
{
	const UINT MF_MASK = MF_SEPARATOR | MF_ENABLED | MF_GRAYED | MF_DISABLED | MF_CHECKED | MF_OWNERDRAW | MF_POPUP | MF_MENUBARBREAK | MF_MENUBREAK;


namespace defColors
{
	const COLORREF menuBG = RGB(240,240,243);//RGB(237,237,245);
	const COLORREF iconLeft = RGB(246,246,253);
	const COLORREF iconRight = RGB( 92, 92, 102 );//RGB(66,65,77);//RGB(159,157,185);
	const COLORREF highliteUp = RGB(255,244,204);
	const COLORREF highliteDown = RGB(255,214,154);
	const COLORREF highliteFrame = RGB(75,75,110);

	TRIVERTEX triVertex[2];

	void CorrectTriVec( const CRect & rect )
	{
		triVertex[0].x = rect.left;
		triVertex[0].y = rect.top;

		triVertex[1].x = rect.right;
		triVertex[1].y = rect.bottom;
	}

	void CorrectTriVec( COLORREF lt, COLORREF rb )
	{
		triVertex[0].Red = GetRValue(lt) << 8;
		triVertex[0].Green = GetGValue(lt) << 8;
		triVertex[0].Blue = GetBValue(lt) << 8;

		triVertex[1].Red = GetRValue(rb) << 8;
		triVertex[1].Green = GetGValue(rb) << 8;
		triVertex[1].Blue = GetBValue(rb) << 8;
	}

	GRADIENT_RECT grTriangle = { 0, 1 };
}

//-------------------------------------------------------------------------------

IAdvancedMenuItem::IAdvancedMenuItem()
: _popup(nullptr)
, _state(0)
, _size(32)
{	;	}


IAdvancedMenuItem::~IAdvancedMenuItem()
{
	delete _popup;
}

void IAdvancedMenuItem::ItemDrawDeselected( DRAWITEMSTRUCT * dis )
{
	ItemDrawEntire(dis);
}

void IAdvancedMenuItem::DrawItem( DRAWITEMSTRUCT *  dis )
{
	if( dis->itemAction & ODA_DRAWENTIRE )
		ItemDrawEntire( dis );
	else if( (dis->itemState & ODS_SELECTED) && (dis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)) )
		ItemDrawSelected( dis );
	else if( !(dis->itemState & ODS_SELECTED) && (dis->itemAction & ODA_SELECT) )
		ItemDrawDeselected( dis );
}

void IAdvancedMenuItem::MeasureItem( MEASUREITEMSTRUCT * mis )
{
	ItemMeasure( mis );
}


//--------------------------------------------------------------------------------


//AdvancedTextItem::AdvancedTextItem( const CString & text, UINT_PTR IDBitmap, UINT_PTR IDIcon )
//: _text(text)
//, _bitmap(0)
//, _icon(0)
//{
//	if( IDBitmap )
//	{
//		_bitmap = LoadBitmap( 0, MAKEINTRESOURCE(IDBitmap) );
//	}
//	else if( IDIcon )
//	{
//		_icon = LoadIcon( 0, MAKEINTRESOURCE(IDIcon) );
//	}
//}

AdvancedTextItem::AdvancedTextItem( const CString & text, IconID iconID )
: _text(text)
, _iconID(iconID)
{	;	}

AdvancedTextItem::~AdvancedTextItem()
{	;	}

void AdvancedTextItem::ItemDrawEntire( DRAWITEMSTRUCT * dis )
{
	
	CDC* pDC = CDC::FromHandle( dis->hDC );

	CRect rectFull( &dis->rcItem );

	// get icon region and text region
	CRect rectIcon(rectFull.left, rectFull.top, rectFull.left+rectFull.Height(), rectFull.top+rectFull.Height());
	if( _state & MF_SEPARATOR )
	{
		rectIcon.right = rectIcon.left + _size + 8;
	}

	CRect rectText(rectIcon.right, rectFull.top, rectFull.right, rectFull.bottom);

	// paint the brush and icon item in requested
	CRect rect;
	// paint item background
	CBrush bgBrush( defColors::menuBG );
	pDC->FillRect(rectFull, &bgBrush );
	//pDC->FillRect(rectIcon, &CBrush( GetSysColor(COLOR_3DFACE) ) );
	defColors::CorrectTriVec(rectIcon);
	defColors::CorrectTriVec( defColors::iconLeft, defColors::iconRight );
	pDC->GradientFill(defColors::triVertex, 2, &defColors::grTriangle, 1, GRADIENT_FILL_RECT_H);

	if( _state & MF_SEPARATOR )	// item is separator
	{	// draw a etched edge
		rect.CopyRect(rectText);
		rect.OffsetRect(4,4);
		pDC->DrawEdge(rect, EDGE_ETCHED, BF_TOP);
	}

	if( _state & MFS_CHECKED )
	{
		rect.CopyRect(rectIcon);
		rect.DeflateRect(2,2,2,2);
		CBrush hlBrush( defColors::highliteFrame );
		pDC->FrameRect(rect, &hlBrush );
		rect.DeflateRect(1,1,1,1);
		defColors::CorrectTriVec(rect);
		defColors::CorrectTriVec( defColors::highliteUp, defColors::highliteDown);
		pDC->GradientFill(defColors::triVertex, 2, &defColors::grTriangle, 1, GRADIENT_FILL_RECT_V);
	}

	//if( _icon )
	{
		Gdiplus::Graphics * gdi = Gdiplus::Graphics::FromHDC( dis->hDC );

		const int posX = rectIcon.left + (rectIcon.Width() - _size)/2;
		const int posY = rectIcon.top + (rectIcon.Height() - _size)/2;

		//::DrawIconEx( pDC->GetSafeHdc(), posX, posY, _icon, _size, _size, 0, nullptr, DI_NORMAL );
		GetIconManager().Draw( *gdi, _iconID, posX, posY, _size, GetGlobal().theme->menu.iconColor );
		delete gdi;
	}

	// draw display text
	pDC->SetBkMode(TRANSPARENT);
	rect.CopyRect(rectText);
	rect.DeflateRect(2,2,2,2);
	rect.OffsetRect(6,2);
	if( _state & MFS_DISABLED )	// item has been disabled
		pDC->SetTextColor( GetSysColor(COLOR_GRAYTEXT) );

	pDC->DrawText( _text, rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE );	
}

void AdvancedTextItem::ItemDrawSelected( DRAWITEMSTRUCT * dis )
{
	if( _state & MFS_DISABLED )	
		return;

	CDC* pDC = CDC::FromHandle(dis->hDC);

	CRect rectFull(&dis->rcItem); 

	// get icon region and text region
	CRect rectIcon(rectFull.left, rectFull.top, rectFull.left+rectFull.Height(), rectFull.top+rectFull.Height());
	CRect rectText(rectIcon.right, rectFull.top, rectFull.right, rectFull.bottom);

	CRect  rect;

	CBrush hlBrush( defColors::highliteFrame );
	pDC->FrameRect(rectFull, &hlBrush );

	// draw hilite brush
	rectFull.DeflateRect(1,1,1,1);
	//pDC->FillRect(rectFull, &CBrush( defColors::highlite) );
	defColors::CorrectTriVec(rectFull);
	defColors::CorrectTriVec( defColors::highliteUp, defColors::highliteDown);
	pDC->GradientFill(defColors::triVertex, 2, &defColors::grTriangle, 1, GRADIENT_FILL_RECT_V);

	if( _state & MFS_CHECKED ) // item has been checked
	{
		rect.CopyRect(rectIcon);
		rect.DeflateRect(2,2,2,2);
		pDC->FrameRect(rect, &hlBrush );
		rect.DeflateRect(1,1,1,1);
		defColors::CorrectTriVec(rect);
		defColors::CorrectTriVec( defColors::highliteUp, defColors::highliteDown);
		pDC->GradientFill(defColors::triVertex, 2, &defColors::grTriangle, 1, GRADIENT_FILL_RECT_V);
	}

	{
		Gdiplus::Graphics * gdi = Gdiplus::Graphics::FromHDC( dis->hDC );
		const int posX = rectIcon.left + (rectIcon.Width() - _size)/2;
		const int posY = rectIcon.top + (rectIcon.Height() - _size)/2;

		GetIconManager().Draw( *gdi, _iconID, posX, posY, _size, GetGlobal().theme->menu.iconColor );
		delete gdi;
	}

	// draw display text
	rect.CopyRect(rectText);
	rect.DeflateRect(2,2,2,2);
	rect.OffsetRect(6,2);

	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(_text, rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE);	
}

void AdvancedTextItem::ItemMeasure( MEASUREITEMSTRUCT * mis )
{
	if( _state & MF_SEPARATOR )
	{
		mis->itemWidth =  _size+8;
		mis->itemHeight = 8;
	}
	else
	{
		CWindowDC dc(nullptr);
		CRect rcText(0,0,0,0);

		dc.DrawText( _text, rcText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_CALCRECT);

		mis->itemWidth = _size+rcText.Width()+8;
		mis->itemHeight = _size+8;
	}
}


//---------------------------------------------------------------------------------------

AdvancedMenu::AdvancedMenu( int iconSize )
{
	if( iconSize >= 16 )
		_iconSize = iconSize;
	else
		_iconSize = GetGlobal().theme ? GetGlobal().theme->menu.iconSize : 32;
}

AdvancedMenu::~AdvancedMenu()
{
	for(auto & item : _items)
	{
		delete item;
		item = nullptr;
	}
}

void AdvancedMenu::AddItem( UINT_PTR id, IAdvancedMenuItem * item, UINT flags )
{
	_items.push_back( item );

	flags &= MF_MASK;

	item->_state = flags;
	item->_size = _iconSize;

	CMenu::AppendMenu( MF_OWNERDRAW | flags, id, (LPCTSTR)item);
}

void AdvancedMenu::AddItem( AdvancedMenu * popup, IAdvancedMenuItem * item, UINT flags )
{
	item->_popup = popup;

	AddItem( (UINT_PTR)popup->m_hMenu, item, flags | MF_POPUP );
}

void AdvancedMenu::AddItem( CommandID id )
{
	this->AddItem( (UINT_PTR)(id), new adv_mfc::AdvancedTextItem( CommandExecute::Instance().GetCommand( id ).commandTitle, CommandExecute::Instance().GetCommand( id ).iconID ) );
}


void AdvancedMenu::AddSeparator( IAdvancedMenuItem * item )
{
	AddItem( (UINT_PTR)0, item, MF_SEPARATOR );
}


void AdvancedMenu::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	IAdvancedMenuItem * item = reinterpret_cast<IAdvancedMenuItem*>( lpDrawItemStruct->itemData );
	if( item )
		item->DrawItem( lpDrawItemStruct );
}

void AdvancedMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	IAdvancedMenuItem * item = reinterpret_cast<IAdvancedMenuItem*>( lpMeasureItemStruct->itemData );
	if( item )
		item->MeasureItem( lpMeasureItemStruct );
}

BOOL AdvancedMenu::LoadMenu(LPCTSTR lpszResourceName, BOOL importOwnerDraw )
{
	CMenu menu;
	if( FALSE == menu.LoadMenu( lpszResourceName ) )
		return FALSE;
	ImportMenu( menu, importOwnerDraw );
	menu.DestroyMenu();
	return TRUE;
}

BOOL AdvancedMenu::LoadMenu(UINT nIDResource, BOOL importOwnerDraw )
{
	CMenu menu;
	if( FALSE == menu.LoadMenu( nIDResource ) )
		return FALSE;
	ImportMenu( menu, importOwnerDraw );
	menu.DestroyMenu();
	return TRUE;
}

BOOL AdvancedMenu::ImportMenu( CMenu & menu, BOOL importOwnerDraw )
{
	const int itemCount = menu.GetMenuItemCount();
	for( int i = 0; i < itemCount; ++i )
	{
		const UINT state = menu.GetMenuState( i, MF_BYPOSITION );

		if( state & MF_OWNERDRAW && !importOwnerDraw )
			continue;

		CString str;
		menu.GetMenuString( i, str, MF_BYPOSITION );

		MENUITEMINFO info;
		memset(&info, 0, sizeof( MENUITEMINFO ));
		info.cbSize = sizeof( MENUITEMINFO );
		info.fMask = MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		menu.GetMenuItemInfo( i, &info, TRUE );

		UINT_PTR id = 0;
		if( info.hSubMenu )
		{
			CMenu * m = CMenu::FromHandle(info.hSubMenu);
			if( m )
			{
				std::unique_ptr<AdvancedMenu> subMenu = std::make_unique<AdvancedMenu>();
				subMenu->CreatePopupMenu();
				subMenu->ImportMenu( *m, importOwnerDraw );

				AddItem( subMenu.release(), new AdvancedTextItem(str), state );
			}
		}
		else
		{
			id =  menu.GetMenuItemID(i);
			AddItem( id, new AdvancedTextItem(str), state );
		}
	}
	return TRUE;
}

}//	adv_mfc