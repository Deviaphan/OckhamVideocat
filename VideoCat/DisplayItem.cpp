// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "DisplayItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



const CRect DisplayItem::_tileBox( 0, 0, ItemWidth, ItemHeight );	///< Габаритный контейнер объекта
const CRect DisplayItem::_listBox( 0, 0, ListItemWidth, ListItemHeight  );	///< Габаритный контейнер объекта
const CRect DisplayItem::_folderRect( 0, ItemHeight - 128 - 40, 128, ItemHeight - 40 );	///< Размер и положение кнопки, относительно габаритного контейнера


CSize DisplayItem::GetOffset( int x, int y )
{
	return CSize( x * (250+20), y * (250+20) );
}
CSize DisplayItem::GetListOffset( int x, int y )
{
	return CSize( x * ListItemWidth, y * ListItemHeight );
}

CRect DisplayItem::GetTileRect( int x, int y )
{
	CRect r( _tileBox );
	r.OffsetRect( GetOffset( x, y ) );
	return r;
}

CRect DisplayItem::GetListRect( int x, int y )
{
	CRect r( _listBox );
	r.OffsetRect( GetListOffset( x, y ) );
	return r;
}

CRect DisplayItem::GetFolderRect( const CRect & item )
{
	CRect r( _folderRect );
	r.OffsetRect( item.left, item.top );
	return r;
}

CRect DisplayItem::GetPosterRect( const CRect & item, int width, int height )
{
	const int offsetX = (ItemWidth - width) / 2;
	const int offsetY = 4;
	CRect r( offsetX, offsetY, offsetX+width, offsetY+height );
	r.OffsetRect( item.left, item.top );
	return r;
}
