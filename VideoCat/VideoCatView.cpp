// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "VideoCatView.h"

#include <thread>
#include <vector>
#include "AdvancedMenu.h"
#include "CommandManager.h"
#include "ContextMenu.h"
#include "DisplayItem.h"
#include "GlobalSettings.h"
#include "HashItem.h"
#include "IconManager.h"
#include "LoadJpg.h"
#include "LoadPng.h"
#include "PosterDownloader.h"
#include "ResString.h"
#include "SetWin10Theme.h"
#include "TipOfDay.h"
#include "VideoCatDoc.h"
#include "VideoTreeView.h"
#include "Commands/CommandExecute.h"
#include "Kinopoisk/Kinopoisk.h"
#include "Traverse/CountViewedFilms.h"

#include "resource.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	BIG_THUMB_WIDTH_FULL = 1600,
	BIG_THUMB_WIDTH = 160,
	BIG_THUMB_HEIGHT = 90,

	LEFT_OFFSET = 96,
	TOP_OFFSET = 100,
};

bool CheckButton( const ItemStyle & style, int btnIndex, const CPoint & itemPos, const CPoint & pt )
{
	CRect btn( style.button[btnIndex].x + itemPos.x,
		style.button[btnIndex].y + itemPos.y,
		style.button[btnIndex].x + style.btnSize + itemPos.x,
		style.button[btnIndex].y + style.btnSize + itemPos.y );

	return btn.PtInRect( pt );
}

void CVideoCatView::BuildShortInfo( const Entry & entry, std::wstring & result )
{
	result.clear();

	static std::vector<std::wstring> usedNames;
	if( usedNames.empty() )
	{
		usedNames.emplace_back( L"Duration" );
		usedNames.emplace_back( L"Width" );
		usedNames.emplace_back( L"Height" );
		usedNames.emplace_back( L"Frame rate" );

		usedNames.emplace_back( L"Format" );
		usedNames.emplace_back( L"File size" );
		usedNames.emplace_back( L"Overall bit rate" );
	}

	const unsigned int numItems = usedNames.size();

	std::vector<int> usedIdx( numItems );
	std::vector<std::wstring> values( numItems );

	for( const auto & data : entry.techInfo )
	{
		for( unsigned i = 0; i < numItems; ++i )
		{
			if( usedIdx[i] == 0 && usedNames[i] == data.first )
			{
				values[i] = data.second;
				usedIdx[i] = 1;
			}
		}
	}

	for( unsigned i = 0; i < numItems; ++i )
	{
		if( !usedIdx[i] )
			continue;

		if( !result.empty() )
			result += L"\r\n";
		if( i == 4 )
			result += L"\r\n";

		result += usedNames[i];
		result += L" :  ";
		result += values[i];
	}
}


IMPLEMENT_DYNCREATE( CVideoCatView, CView )

BEGIN_MESSAGE_MAP( CVideoCatView, CView )
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

CVideoCatView::CVideoCatView()
	: _pageScale( 200 )
	, _numTotal( 0 )
	, _numHorz( 0 )
	, _numVert( 0 )
	, _currentItem( 0 )
	, _gdi( nullptr )
	, _drawThumbBox( false )
	, _thumbs(nullptr)
	, _smallThumbs(nullptr)
	, _thumbIndex( -1 )
	, _forcedIndex( false )
	, _ait( AIT_NONE )
	, _listView( true )
	, _thumbsWidth(0)
	, _thumbsHeight(0)
{
	_scrollHelper.AttachWnd( this );
}

CVideoCatView::~CVideoCatView()
{}

void CVideoCatView::GetScreenRect( CRect & r ) const
{
	GetClientRect( r );
	ScaleRect( r );
}


int CVideoCatView::ScaleInt( int v ) const
{
	return (v * 100) / _pageScale;
}

float CVideoCatView::ScaleFloat( float v ) const
{
	return (v * 100.0f) / (float)_pageScale;
}

CPoint & CVideoCatView::ScalePoint( CPoint & p ) const
{
	p.x = (p.x * 100) / _pageScale;
	p.y = (p.y * 100) / _pageScale;
	
	return p;
}

CRect & CVideoCatView::ScaleRect( CRect & r ) const
{
	r.left = (r.left * 100) / _pageScale;
	r.top = (r.top * 100) / _pageScale;
	r.right = (r.right * 100) / _pageScale;
	r.bottom = (r.bottom * 100) / _pageScale;

	return r;
}


void CVideoCatView::UpdateView()
{
	Invalidate(FALSE);
	//UpdateWindow();
}

void CVideoCatView::SetDarkTheme( bool dark )
{
	SetWin10Theme( GetSafeHwnd(), dark );
}


// усекаем слишком длинное название, вырезая середину и вставляя троеточие
void FitString( const CString & title, const CRect & nameRect, const CRect & calcRect, CString & drawString )
{
	if( calcRect.Width() == 0 || calcRect.Width() <= nameRect.Width() )
	{
		drawString = title;
		return;
	}

	const float percents = 1.0f - float( calcRect.Width() - nameRect.Width() ) / float( calcRect.Width() );
	const int numSymbols = int( title.GetLength() * percents ) - 2;
	const int half = numSymbols / 2;
	drawString = title.Left( half );
	drawString += L"…";
	drawString += title.Right( half );
}

void CVideoCatView::DrawItemButtons( const DisplayItem & item, const CRect & selectedItemRect, bool listButton )
{
	if( !GetGlobal().theme )
		return;

	const ItemStyle & style = listButton ? GetGlobal().theme->listItemSelected : GetGlobal().theme->tileItemSelected;

	const COLORREF btnColor = style.btnColor;
	const COLORREF btnColorHover = style.btnColorHover;
	const int btnSize = style.btnSize;
	const auto & btnPos = style.button;

	// кнопка воспроизведения, папки или сериала
	if( item._info->IsFolder() )
	{
		if( item._info->IsTV() )
			GetIconManager().Draw( *_gdi, IconManager::TVSeries, selectedItemRect.left + btnPos[0].x, selectedItemRect.top + btnPos[0].y, btnSize, _ait == AIT_PLAY ? btnColorHover : btnColor );
		else
			GetIconManager().Draw( *_gdi, IconManager::FolderOpened, selectedItemRect.left + btnPos[0].x, selectedItemRect.top + btnPos[0].y, btnSize, _ait == AIT_PLAY ? btnColorHover : btnColor );
	}
	else
	{
		GetIconManager().Draw( *_gdi, IconManager::Play, selectedItemRect.left + btnPos[0].x, selectedItemRect.top + btnPos[0].y, btnSize, _ait == AIT_PLAY ? btnColorHover : btnColor );
	}

	// кнопка меню
	GetIconManager().Draw( *_gdi, IconManager::MenuButton, selectedItemRect.left + btnPos[1].x, selectedItemRect.top + btnPos[1].y, btnSize, _ait == AIT_MENU ? btnColorHover : btnColor );

	// кнопка описания, если есть, что показать
	if( !item._info->description.empty() || !item._info->tagline.empty() || item._info->genres )
	{
		GetIconManager().Draw( *_gdi, IconManager::EditEntry, selectedItemRect.left + btnPos[2].x, selectedItemRect.top + btnPos[2].y, btnSize, btnColor );
	}

	// кнопка тэгов, если есть тэги
	if( !item._info->tags.empty() || !item._info->comment.empty() )
	{
		GetIconManager().Draw( *_gdi, IconManager::EditTagInfo, selectedItemRect.left + btnPos[3].x, selectedItemRect.top + btnPos[3].y, btnSize, btnColor );
	}

	if( !item._info->IsFolder() || item._info->IsTV() )
	{
		GetIconManager().Draw( *_gdi, IconManager::YouTubeLink, selectedItemRect.left + btnPos[4].x, selectedItemRect.top + btnPos[4].y, btnSize, _ait == AIT_YOUTUBE ? btnColorHover : btnColor );
	}

	if( !item._info->techInfo.empty() )
	{
		GetIconManager().Draw( *_gdi, IconManager::EditTechInformation, selectedItemRect.left + btnPos[5].x, selectedItemRect.top + btnPos[5].y, btnSize, btnColor );
	}
}

void CVideoCatView::DrawGlass( const Gdiplus::Rect &  itemRect, const ItemStyle & style )
{
	// тени вокруг плитки
	{
		const COLORREF shadowColor = style.shadowColor;

		Gdiplus::ColorMatrix cm;
		ZeroMemory( &cm.m, sizeof( cm ) );
		cm.m[0][0] = GetRValue( shadowColor ) / 255.0f; // r
		cm.m[1][1] = GetGValue( shadowColor ) / 255.0f; // g
		cm.m[2][2] = GetBValue( shadowColor ) / 255.0f; // b
		cm.m[3][3] = GetGlobal().theme->tileItemSelected.shadowOpacity / 255.0f; // a
		cm.m[4][4] = 1.0f;

		Gdiplus::ImageAttributes ia;
		ia.SetColorMatrix( &cm );

		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );

		//уголки
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetLeft() - 20, itemRect.GetTop() - 20, 20, 20 }, 0, 0, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetRight(), itemRect.GetTop() - 20, 20, 20 }, 60, 0, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetLeft() - 20, itemRect.GetBottom(), 20, 20 }, 0, 60, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetRight(), itemRect.GetBottom(), 20, 20 }, 60, 60, 20, 20, Gdiplus::Unit::UnitPixel, &ia );

		// серединки
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetLeft(), itemRect.GetTop() - 20, itemRect.Width, 20 }, 20, 0, 40, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetLeft(), itemRect.GetBottom(), itemRect.Width, 20 }, 20, 60, 40, 20, Gdiplus::Unit::UnitPixel, &ia );

		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetLeft() - 20, itemRect.GetTop(), 20, itemRect.Height }, 0, 20, 20, 40, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { itemRect.GetRight(), itemRect.GetTop(), 20, itemRect.Height }, 60, 20, 20, 40, Gdiplus::Unit::UnitPixel, &ia );
	}

	if( Gdiplus::Bitmap * glassImage = GetGlobal().theme->GetGlassImage() )
	{
		if( glassImage->GetWidth() > 0 )
		{
			const double cX = (double)GetGlobal().bgFullSize.Width() / (double)glassImage->GetWidth();
			const double cY = (double)GetGlobal().bgFullSize.Height() / (double)glassImage->GetHeight();
			if( cX > 0.0 && cY > 0.0 )
			{
				const INT x1 = ((itemRect.GetLeft() * _pageScale / 100) + GetGlobal().bgTree.Width()) / cX;
				const INT y1 = (double)(itemRect.GetTop() * _pageScale / 100) / cY;
				const INT x2 = (itemRect.Width * _pageScale / 100) / cX;
				const INT y2 = (itemRect.Height * _pageScale / 100) / cY;

				_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );
				const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
				_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );
				_gdi->DrawImage( glassImage, itemRect, x1, y1, x2, y2, Gdiplus::UnitPixel );
				_gdi->SetCompositingMode( oldComposing );
			}
		}
	}

	Gdiplus::Pen pen( MakeColor( style.borderColor, style.borderOpacity ), 1.0f );
	_gdi->DrawRectangle( &pen, itemRect );
}

void CVideoCatView::DrawTiles( const CRect & clientRect, const CPoint & pos )
{
	const bool isBigPreview = _drawThumbBox && (_thumbIndex != -1);

	if( isBigPreview )
	{
		if( _currentItem >= 0 )
		{
			DrawThumbPreview( clientRect );
		}

		return;
	}

	CVideoCatDoc* doc = GetDocument();

	const bool rusFirst = sort_predicates::GetPredicateId( doc->GetCurrentPredicate() ) != sort_predicates::SortEn;

	const FilteredFileItems & displayItems = doc->GetFilteredFiles();

	const int hor = pos.x / DisplayItem::ItemWidth;
	const int ver = (pos.y - TOP_OFFSET) / DisplayItem::ItemHeight;

	int selectedH = -1;
	int selectedV = -1;

	const MarkType::Enum markType = GetGlobal().GetMarkType();

	const ViewStyle * theme = GetGlobal().theme;

	const COLORREF markerColor = theme->markerColor;

	Gdiplus::ColorMatrix cm;
	ZeroMemory( &cm.m, sizeof( cm ) );
	cm.m[0][0] = GetRValue( markerColor ) / 255.0f; // r
	cm.m[1][1] = GetGValue( markerColor ) / 255.0f; // g
	cm.m[2][2] = GetBValue( markerColor ) / 255.0f; // b
	cm.m[3][3] = theme->markerOpacity / 255.0f; // a
	cm.m[4][4] = 1.0f;

	Gdiplus::ImageAttributes iaMarked;
	iaMarked.SetColorMatrix( &cm );

	Gdiplus::ColorMatrix cm2;
	ZeroMemory( &cm2.m, sizeof( cm2 ) );
	cm2.m[0][0] = 0.0f; // r
	cm2.m[1][1] = 0.0f; // g
	cm2.m[2][2] = 0.0f; // b
	cm2.m[3][3] = 60.0f / 255.0f; // a
	cm2.m[4][4] = 1.0f;

	Gdiplus::ImageAttributes iaPoster;
	iaPoster.SetColorMatrix( &cm2 );


	const int itemWidth = theme->tileItem.width;
	const int itemHeight = theme->tileItem.height;
	const int offsetX = itemWidth + theme->tileItem.offset;
	const int offsetY = itemHeight + theme->tileItem.offset;

	const Gdiplus::Rect itemRect0( 0, 0, itemWidth, itemHeight );

	for( int v = ver; v <= (ver + _numVert + 1); ++v )
	{
		for( int h = hor; h < (_numHorz + hor); ++h )
		{
			int itemIndex = v * _numHorz + h;
			if( itemIndex >= _numTotal )
				goto ALL_FILES_PROCESSED; // выход из цикла двойной вложенности

			const bool isSelected = _currentItem == itemIndex;
			if( isSelected )
			{
				selectedH = h;
				selectedV = v;
			}

			//if( isBigPreview )
			//{
			//	if( isSelected )
			//		goto ALL_FILES_PROCESSED;
			//	else
			//		continue;
			//}

			const DisplayItem & item = displayItems.at( itemIndex );

			Gdiplus::Rect itemRect( itemRect0 );
			itemRect.Offset( h*offsetX + LEFT_OFFSET, v*offsetY + TOP_OFFSET - pos.y );

			// Фон для выделенного в данный момент фильма
			if( isSelected )
			{
				DrawGlass( itemRect, theme->tileItemSelected );
			}

			{
				Gdiplus::Bitmap * poster = _noPosterTile.get();
				if( item._poster )
					poster = item._poster.get();
				else if( item._info->IsTV() )
					poster = _noPosterTileTV.get();				

				if( !poster )
					continue; // такого не может быть, но, на всякий случай, лучше предусмотреть

				const Gdiplus::Rect posterDestRect = { itemRect.GetLeft() + INT(itemWidth - poster->GetWidth())/2 , itemRect.GetTop() + (INT)4, (INT)poster->GetWidth(), (INT)poster->GetHeight() };

				const Gdiplus::InterpolationMode oldInterpolation = _gdi->GetInterpolationMode();
				_gdi->SetInterpolationMode( _pageScale == 100 ? Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor : Gdiplus::InterpolationMode::InterpolationModeBilinear);
				const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
				_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );
				_gdi->DrawImage( poster, posterDestRect, 0,0, posterDestRect.Width, posterDestRect.Height, Gdiplus::UnitPixel );
				_gdi->SetCompositingMode( oldComposing );
				_gdi->SetInterpolationMode( oldInterpolation );

				if( !Test(item._info->flags, EntryTypes::TVepisode) )
				{
					if( item._info->year > 0 )
					{
						int posX = itemRect.GetLeft() + itemRect.Width / 2 + 35;
						int posY = itemRect.GetTop() + posterDestRect.Height - 10;

						DrawYear( item._info, posX, posY );
					}
				}
				else
				{
					if( item._info->seriesData )
					{
						int posX = itemRect.GetLeft() + itemRect.Width / 2 + 35;
						int posY = posterDestRect.Y + posterDestRect.Height - 12;

						DrawEpisode( item._info, posX, posY );
					}
				}

				Gdiplus::ImageAttributes * ia = &iaPoster;

				if( !item._info->IsFolder() && (itemIndex != _currentItem) )
				{
					if( markType != MarkType::NoMark )
					{
						const bool showEye = markType == MarkType::MarkNotPlayed ? item._info->numViews == 0 : item._info->numViews > 0;
						// глазик около фильмов, которых ещё не смотрели
						if( showEye )
						{
							ia = &iaMarked;
						}
					}
				}

				// рисуем обводку вокруг постера либо цветом маркера, либо незаметную тень
				{
					_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );

					_gdi->DrawImage( _itemShadow.get(), { posterDestRect.GetLeft() - 20, posterDestRect.GetTop(), 20, 20 }, 0, 0, 20, 20, Gdiplus::Unit::UnitPixel, ia );
					_gdi->DrawImage( _itemShadow.get(), { posterDestRect.GetRight(), posterDestRect.GetTop(), 20, 20 }, 60, 0, 20, 20, Gdiplus::Unit::UnitPixel, ia );

					_gdi->DrawImage( _itemShadow.get(), { posterDestRect.GetLeft() - 20, posterDestRect.GetBottom() - 20, 20, 20 }, 0, 60, 20, 20, Gdiplus::Unit::UnitPixel, ia );
					_gdi->DrawImage( _itemShadow.get(), { posterDestRect.GetRight(), posterDestRect.GetBottom() - 20, 20, 20 }, 60, 60, 20, 20, Gdiplus::Unit::UnitPixel, ia );

					_gdi->DrawImage( _itemShadow.get(), { posterDestRect.GetLeft() - 20, posterDestRect.GetTop() + 20, 20, posterDestRect.Height - 40 }, 0, 20, 20, 40, Gdiplus::Unit::UnitPixel, ia );
					_gdi->DrawImage( _itemShadow.get(), { posterDestRect.GetRight(), posterDestRect.GetTop() + 20, 20, posterDestRect.Height - 40 }, 60, 20, 20, 40, Gdiplus::Unit::UnitPixel, ia );
				}

				if( item._info->rating )
				{
					if( isSelected || GetGlobal().GetShowStars() )
					{
						GetIconManager().DrawStars( *_gdi, (item._info->rating + 5) / 10, posterDestRect.GetLeft() + posterDestRect.Width / 2, posterDestRect.GetTop() - 16, 24 );
					}
				}
			}

			CRect nameRect( itemRect.GetLeft() + 16, itemRect.GetBottom() - 54, itemRect.GetRight() - 16, itemRect.GetBottom() );
			DrawName( item._info, nameRect, rusFirst, DT_CENTER, isSelected ? theme->tileItemSelected : theme->tileItem, false );

			if( item._info->IsFolder() )
			{
				DrawFolder( item, itemRect.GetLeft(), itemRect.GetBottom() );
			}
		}
	}

ALL_FILES_PROCESSED:

	CRect selectedItemRect = DisplayItem::GetTileRect( selectedH, selectedV ) - pos;
	selectedItemRect.OffsetRect( LEFT_OFFSET, TOP_OFFSET );

	if( _currentItem >= 0 )
	{
		DrawItemButtons( displayItems.at( _currentItem ), selectedItemRect, false );
	}

	if( _drawThumbBox && _currentItem >= 0 )
	{
		DrawThumbPreview( clientRect );
	}

	if( _currentItem >= 0 )
	{
		const DisplayItem & item = displayItems.at( _currentItem );

		if( _ait == AIT_DESCRIPTION )
		{
			DrawDescription( clientRect, selectedItemRect, item );
		}
		else if( _ait == AIT_TAG )
		{
			DrawTag( clientRect, selectedItemRect, item );
		}
		else if( _ait == AIT_TECHINFO )
		{
			DrawTechInfo( clientRect, selectedItemRect, item );
		}
	}
}

void CVideoCatView::DrawList( const CRect & clientRect, const CPoint & pos )
{
	const bool isBigPreview = _drawThumbBox && _thumbIndex != -1;

	if( isBigPreview )
	{
		DrawThumbPreview( clientRect );
		return;
	}

	CVideoCatDoc* doc = GetDocument();

	const bool rusFirst = sort_predicates::GetPredicateId( doc->GetCurrentPredicate() ) != sort_predicates::SortEn;

	const FilteredFileItems & displayItems = doc->GetFilteredFiles();

	const ViewStyle * theme = GetGlobal().theme;

	const int itemHeight = theme->listItem.height;
	int itemWidth = clientRect.Width() - 128;
	if( itemWidth < 640 )
		itemWidth = clientRect.Width();

	const int offsetX = itemWidth + theme->listItem.offset;
	const int offsetY = (std::max)(itemHeight + theme->listItem.offset, 32);

	const int ver = pos.y / offsetY;

	int selectedH = -1;
	int selectedV = -1;

	const MarkType::Enum markType = GetGlobal().GetMarkType();
	
	Gdiplus::Pen pen( MakeColor( theme->listItem.colorTitle2, 191 ) );

	const COLORREF markerColor = theme->markerColor;

	Gdiplus::ColorMatrix cm;
	ZeroMemory( &cm.m, sizeof( cm ) );
	cm.m[0][0] = GetRValue( markerColor ) / 255.0f; // r
	cm.m[1][1] = GetGValue( markerColor ) / 255.0f; // g
	cm.m[2][2] = GetBValue( markerColor ) / 255.0f; // b
	cm.m[3][3] = theme->markerOpacity / 255.0f; // a
	cm.m[4][4] = 1.0f;

	Gdiplus::ImageAttributes ia;
	ia.SetColorMatrix( &cm );

	const Gdiplus::Rect itemRect0( 0, 0, itemWidth, itemHeight );

	for( int v = ver; v <= (ver + _numVert + 1); ++v )
	{
		const int itemIndex = v;
		if( itemIndex >= _numTotal )
			goto ALL_FILES_PROCESSED;

		const bool isSelected = _currentItem == itemIndex;
		if( isSelected )
		{
			selectedH = 0;
			selectedV = v;
		}

		//if( isBigPreview )
		//{
		//	if( isSelected )
		//		goto ALL_FILES_PROCESSED;
		//	else
		//		continue;
		//}

		const ItemStyle & style = isSelected ? theme->listItemSelected : theme->listItem;

		const DisplayItem & item = displayItems.at( itemIndex );

		Gdiplus::Rect itemRect( itemRect0 );
		itemRect.Offset( LEFT_OFFSET - pos.x, v*offsetY + TOP_OFFSET - pos.y );

		if( isSelected )
		{
			DrawGlass( itemRect, theme->listItemSelected );
		}

		{
			Gdiplus::Bitmap * poster = _noPosterList.get();
			if( item._poster )
				poster = item._poster.get();
			else if( item._info->IsTV() )
				poster = _noPosterListTV.get();

			const double scale = (double)poster->GetHeight() / style.height;

			const Gdiplus::Rect posterDestRect( itemRect.GetLeft(), itemRect.GetTop(), poster->GetWidth() / scale, itemRect.Height );

			_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear ); // уменьшенный размер, поэтмоу всегда с интерполяцией
			const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
			_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );
			_gdi->DrawImage( poster, posterDestRect, 0, 0, poster->GetWidth(), poster->GetHeight(), Gdiplus::UnitPixel );
			_gdi->SetCompositingMode( oldComposing );

			if( !(item._info->seriesData) )
			{
				// для НЕ эпизодов сериала пишется год, если задан
				if( item._info->year > 0 )
				{
					const int posX = itemRect.GetLeft() + 5;
					const int posY = itemRect.GetBottom() - _bmp10->GetHeight();

					DrawYear( item._info, posX, posY );
				}
			}
			else
			{
				// для эпизодов пишется номер сезона и номер эпизода, если заданы
				const int posX = itemRect.GetLeft() + 5;
				const int posY = itemRect.GetBottom() - _bmp10->GetHeight();

				DrawEpisode( item._info, posX, posY );
			}

			if( !item._info->IsFolder() && (itemIndex != _currentItem) )
			{
				if( markType != MarkType::NoMark )
				{
					const bool showEye = markType == MarkType::MarkNotPlayed ? item._info->numViews == 0 : item._info->numViews > 0;
					// глазик около фильмов, которых ещё не смотрели
					if( showEye )
					{
						_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );

						_gdi->DrawImage( _itemShadow.get(), { posterDestRect.X - 20, posterDestRect.Y, 20, 20 }, 0, 0, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
						_gdi->DrawImage( _itemShadow.get(), { posterDestRect.X - 20, posterDestRect.Y + posterDestRect.Height - 20, 20, 20 }, 0, 60, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
						_gdi->DrawImage( _itemShadow.get(), { posterDestRect.X - 20, posterDestRect.Y + 20, 20, posterDestRect.Height - 40 }, 0, 20, 20, 40, Gdiplus::Unit::UnitPixel, &ia );
					}
				}
			}

			if( !isSelected && item._info->rating )
			{
				if( isSelected || GetGlobal().GetShowStars() )
				{
					GetIconManager().DrawStars( *_gdi, (item._info->rating + 5) / 10, posterDestRect.GetLeft() + 140, posterDestRect.GetTop() + 8, 24 );
				}
			}
		}

		CRect nameRect( itemRect.GetLeft() + 216, itemRect.GetTop(), itemRect.GetRight() - 18, itemRect.GetBottom() );
		DrawName( item._info, nameRect, rusFirst, DT_LEFT, style, true );

		if( !isSelected )
			_gdi->DrawLine( &pen, itemRect.GetLeft(), itemRect.GetBottom(), itemRect.GetRight() - 50, itemRect.GetBottom() );

		if( item._info->IsFolder() )
		{
			DrawListFolder( item, itemRect );
		}

		if( !item._info->techInfo.empty() )
		{
			for( const auto & tech : item._info->techInfo )
			{
				if( tech.first == L"Duration" )
				{
					Gdiplus::SolidBrush brush( MakeColor( style.colorTitle2 ) );
					_gdi->DrawString( tech.second.c_str(), tech.second.length(), style.fontTitle2, Gdiplus::PointF( itemRect.GetLeft() + 92, itemRect.GetBottom() - style.sizeTitle2 - 4 ), nullptr, &brush );

					break;
				}
			}
		}
	}

ALL_FILES_PROCESSED:

	if( selectedV < 0 )
		return;


	CRect selectedItemRect = DisplayItem::GetListRect( selectedH, selectedV ) - pos;
	selectedItemRect.OffsetRect( LEFT_OFFSET, TOP_OFFSET );


	DrawItemButtons( displayItems.at( _currentItem ), selectedItemRect, true );

	if( _drawThumbBox && _currentItem >= 0 )
	{
		DrawThumbPreview( clientRect );
	}

	const DisplayItem & item = displayItems.at( _currentItem );

	if( _ait == AIT_DESCRIPTION )
	{
		DrawDescription( clientRect, selectedItemRect, item );
	}
	else if( _ait == AIT_TAG )
	{
		DrawTag( clientRect, selectedItemRect, item );
	}
	else if( _ait == AIT_TECHINFO )
	{
		DrawTechInfo( clientRect, selectedItemRect, item );
	}
}

void CVideoCatView::DrawYear( const Entry * entry, int posX, int posY )
{
	int div = 1000;

	_gdi->SetInterpolationMode( _pageScale == 100 ? Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor : Gdiplus::InterpolationMode::InterpolationModeBilinear );

	const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
	_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );

	Gdiplus::Rect destRect( posX-5, posY, 5, 18 );

	_gdi->DrawImage( _bmp10.get(), destRect, 10 * 10, 0, 5, 18, Gdiplus::UnitPixel );

	destRect.X = posX;
	destRect.Width = 10;

	for( int i = 0; i < 4; ++i )
	{
		const int index = ((int)entry->year / div) % 10;

		_gdi->DrawImage( _bmp10.get(), destRect, index * 10, 0, 10, 18, Gdiplus::UnitPixel );

		destRect.X += 10;
		div /= 10;
	}

	destRect.Width = 5;

	_gdi->DrawImage( _bmp10.get(), destRect, 10 * 10 + 5, 0, 5, 18, Gdiplus::UnitPixel );

	_gdi->SetCompositingMode( oldComposing );
}

void CVideoCatView::DrawEpisode( const Entry * entry, int posX, int posY )
{
	const int season = entry->GetSeason();
	const int episode = entry->GetEpisode();

	std::vector<int> digits;
	digits.reserve( 9 );
	if( season > 100 )
		digits.push_back( (season / 100) % 10 );
	digits.push_back( (season / 10) % 10 );
	digits.push_back( season % 10 );

	digits.push_back( -1 );

	if( episode > 10000 )
		digits.push_back( (episode / 10000) % 10 );
	if( episode > 1000 )
		digits.push_back( (episode / 1000) % 10 );
	if( episode > 100 )
		digits.push_back( (episode / 100) % 10 );
	digits.push_back( (episode / 10) % 10 );
	digits.push_back( episode % 10 );

	_gdi->SetInterpolationMode( _pageScale == 100 ? Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor : Gdiplus::InterpolationMode::InterpolationModeBilinear );

	const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
	_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );

	Gdiplus::Rect destRect( posX-5, posY, 5, 18 );

	_gdi->DrawImage( _bmp10.get(), destRect, 10 * 10, 0, 5, 18, Gdiplus::UnitPixel );

	destRect.X = posX;
	destRect.Width = 10;

	for( const int digit : digits )
	{
		if( digit == -1 )
		{
			_gdi->DrawImage( _bmp10.get(), destRect, 10 * 10+5, 0, 5, 18, Gdiplus::UnitPixel );

			destRect.X += 5;
			_gdi->DrawImage( _bmp10.get(), destRect, 10 * 10, 0, 5, 18, Gdiplus::UnitPixel );

			destRect.X += 5;
		}
		else
		{
			_gdi->DrawImage( _bmp10.get(), destRect, digit * 10, 0, 10, 18, Gdiplus::UnitPixel );

			destRect.X += 10;
		}
	}

	destRect.Width = 5;
	_gdi->DrawImage( _bmp10.get(), destRect, 10 * 10 + 5, 0, 5, 18, Gdiplus::UnitPixel );

	_gdi->SetCompositingMode( oldComposing );
}

void CVideoCatView::DrawName( const Entry * entry, CRect & rect, bool rusFirst, UINT flags, const ItemStyle & style, bool advanced )
{
	const std::wstring * firstLine = &entry->title;
	const std::wstring * secondLine = &entry->titleAlt;
	if( !rusFirst )
		std::swap( firstLine, secondLine );

	Gdiplus::RectF stringRect( rect.left, rect.top, rect.Width(), rect.Height() );

	Gdiplus::StringFormat format( Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap );
	format.SetAlignment( (flags == DT_LEFT) ? Gdiplus::StringAlignment::StringAlignmentNear : Gdiplus::StringAlignment::StringAlignmentCenter );
	format.SetTrimming( Gdiplus::StringTrimming::StringTrimmingEllipsisPath );

	stringRect.Y += style.sizeTitle1 / 2;
	const Gdiplus::SolidBrush brush( MakeColor( style.colorTitle1 ) );
	_gdi->DrawString( firstLine->c_str(), firstLine->length(), style.fontTitle1, stringRect, &format, &brush );

	if( !GetGlobal().hideSecondTitle && !secondLine->empty() )
	{
		stringRect.Y += style.sizeTitle1 + style.sizeTitle2 / 2;

		const Gdiplus::SolidBrush brush2( MakeColor( style.colorTitle2 ) );
		_gdi->DrawString( secondLine->c_str(), secondLine->length(), style.fontTitle2, stringRect, &format, &brush2 );
	}

	if( advanced )
	{
		stringRect.Y = rect.bottom - style.sizeTag - style.sizeTag/2;
		if( !entry->tags.empty() )
		{
			CVideoCatDoc* doc = GetDocument();
			CollectionDB * cdb = doc->GetCurrentCollection();
			TagManager & tm = cdb->GetTagManager();

			std::wstring tagString;
			for( TagId id : entry->tags )
			{
				tagString += tm.GetTag( id );
				tagString += L", ";
			}

			const Gdiplus::SolidBrush brush3( MakeColor( style.colorTag ) );
			_gdi->DrawString( tagString.c_str(), tagString.length(), style.fontTags, stringRect, &format, &brush3 );
		}
	
		if( entry->genres != EmptyGenres )
		{
			stringRect.Y -= style.sizeGenre + style.sizeGenre / 2;

			std::wstring genreString;

			for( int g = (int)Genre::FirstGenre; g < (int)Genre::NumOfGenres; ++g )
			{
				if( Test( entry->genres, (Genre)g ) )
				{
					genreString += Descript( (Genre)g ).GetString();
					genreString += L", ";
				}
			}

			const Gdiplus::SolidBrush brush4( MakeColor( style.colorGenre ) );
			_gdi->DrawString( genreString.c_str(), genreString.length(), style.fontGenres, stringRect, &format, &brush4 );
		}

	}
}

void CVideoCatView::DrawFolder( const DisplayItem & item, int posX, int posY )
{
	Gdiplus::Bitmap * folderImg = _bmpRealFolder.get();
	if( item._info->IsPrivate() )
	{
		folderImg = _bmpLockedFolder.get();
	}
	else if( !item._info->thisEntry.IsReal() )
	{
		folderImg = _bmpVirtualFolder.get();
	}

	posX += 16;
	posY -= 90 + 48;

	const Gdiplus::Rect destRect( posX, posY, 100, 90 );

	_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic );

	_gdi->DrawImage( folderImg, destRect, 0, 0, folderImg->GetWidth(), folderImg->GetHeight(), Gdiplus::UnitPixel );

	const ItemStyle & style = GetGlobal().theme->tileItem;

	CString viewStat;
	viewStat.Format( L"%u / %u", item._viewedFilms, item._totalFilms );

	Gdiplus::StringFormat format;
	format.SetAlignment( Gdiplus::StringAlignmentCenter );

	const Gdiplus::SolidBrush brush( MakeColor( RGB(75,75,75) ) );
	const Gdiplus::RectF textRect( posX + 8, posY + 90 / 2, 100 - 16, style.sizeTitle1 );
	_gdi->DrawString( viewStat, viewStat.GetLength(), style.fontTitle1, textRect, &format, &brush );
}

void CVideoCatView::DrawListFolder( const DisplayItem & item, const Gdiplus::Rect & rect )
{
	const ItemStyle & style = GetGlobal().theme->tileItem;

	Gdiplus::RectF textRect( rect.GetLeft() + 85, rect.GetBottom() - style.sizeTitle1 - 4, rect.Width, style.sizeTitle1 );

	GetIconManager().Draw( *_gdi, IconManager::File, textRect.GetLeft(), textRect.GetTop(), style.sizeTitle1, style.btnColor );

	textRect.X += style.sizeTitle1;
	
	CString viewStat;
	viewStat.Format( L"%u / %u", item._viewedFilms, item._totalFilms );

	Gdiplus::StringFormat format;
	format.SetAlignment( Gdiplus::StringAlignmentNear );

	const Gdiplus::SolidBrush brush( MakeColor( style.colorTitle2 ) );
	_gdi->DrawString( viewStat, viewStat.GetLength(), style.fontTitle1, textRect, &format, &brush );
}

void CVideoCatView::DrawTextBox( const CRect & clientRect, const CRect & itemRect, const std::wstring & headerText, const std::wstring & bodyText, const std::wstring & footerText )
{
	ViewStyle * theme = GetGlobal().theme;

	if( !theme->GetGlassImage() )
		return;

	const int vCenter = itemRect.top + itemRect.Height() / 2;
	const int hCenter = _listView ? 340 : (itemRect.left + itemRect.Width()/2);

	// полоски хэдера и футера
	const int headerSize = theme->tileItemSelected.sizeTitle1 * 2;

	const int boxWidth = 36 * theme->tileItemSelected.sizeTitle1;
	const int boxHeight = 20 * theme->tileItemSelected.sizeTitle1 + headerSize*2;

	Gdiplus::Rect destRect( hCenter - boxWidth / 2, vCenter - boxHeight / 2, boxWidth, boxHeight );
	if( destRect.X < (clientRect.left + LEFT_OFFSET) )
	{
		destRect.X = clientRect.left + LEFT_OFFSET;
	}
	else
	{
		const int offsetX = destRect.GetRight() - clientRect.right;
		if( offsetX > 0 )
			destRect.X -= offsetX + 16;
	}

	if( destRect.GetTop() < clientRect.top )
	{
		destRect.Y = clientRect.top + 16;
	}
	else
	{
		const int offsetY = destRect.GetBottom() - clientRect.bottom;
		if( offsetY > 0 )
			destRect.Y -= offsetY + 16;
	}

	if( theme->GetGlassImage()->GetWidth() <= 0 )
		return;

	const double cX = (double)GetGlobal().bgFullSize.Width() / (double)theme->GetGlassImage()->GetWidth();
	const double cY = (double)GetGlobal().bgFullSize.Height() / (double)theme->GetGlassImage()->GetHeight();
	if( cX <= 0 || cY <= 0 )
		return;

	const INT x1 = (destRect.X + GetGlobal().bgTree.Width()) / cX;
	const INT y1 = (double)(destRect.Y) / cY;
	const INT x2 = boxWidth / cX;
	const INT y2 = boxHeight / cY;
	
	// стекло
	_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );

	const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
	_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );
	_gdi->DrawImage( theme->GetGlassImage(), destRect, x1, y1, x2, y2, Gdiplus::UnitPixel );
	_gdi->SetCompositingMode( oldComposing );

	Gdiplus::RectF bodyRect( destRect.X, destRect.Y + headerSize, destRect.Width, destRect.Height - headerSize * 2 );

	{
		Gdiplus::ColorMatrix cm;
		ZeroMemory( &cm.m, sizeof( cm ) );
		cm.m[0][0] = 1.0f; // r
		cm.m[1][1] = 1.0f; // g
		cm.m[2][2] = 1.0f; // b
		cm.m[3][3] = 10.0f/255.0f; // a
		cm.m[4][4] = 1.0f;

		Gdiplus::ImageAttributes iaDesc;
		iaDesc.SetColorMatrix( &cm );

		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );
		_gdi->DrawImage( _bgDescription.get(), bodyRect, 0, 0, _bgDescription->GetWidth(), _bgDescription->GetHeight(), Gdiplus::UnitPixel, &iaDesc );
	}

	bodyRect.Inflate( -8.0, -8.0 );

	// тени вокруг плитки
	{
		const COLORREF shadowColor = theme->tileItemSelected.shadowColor;

		Gdiplus::ColorMatrix cm;
		ZeroMemory( &cm.m, sizeof( cm ) );
		cm.m[0][0] = GetRValue( shadowColor ) / 255.0f; // r
		cm.m[1][1] = GetGValue( shadowColor ) / 255.0f; // g
		cm.m[2][2] = GetBValue( shadowColor ) / 255.0f; // b
		cm.m[3][3] = theme->tileItemSelected.shadowOpacity / 255.0f; // a
		cm.m[4][4] = 1.0f;

		Gdiplus::ImageAttributes ia;
		ia.SetColorMatrix( &cm );

		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );

		//уголки
		_gdi->DrawImage( _itemShadow.get(), { destRect.X - 20, destRect.Y - 20, 20, 20 }, 0, 0, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { destRect.X + boxWidth, destRect.Y - 20, 20, 20 }, 60, 0, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { destRect.X - 20, destRect.Y + boxHeight, 20, 20 }, 0, 60, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { destRect.X + boxWidth, destRect.Y + boxHeight, 20, 20 }, 60, 60, 20, 20, Gdiplus::Unit::UnitPixel, &ia );

		// серединки
		_gdi->DrawImage( _itemShadow.get(), { destRect.X, destRect.Y - 20, boxWidth, 20 }, 20, 0, 40, 20, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { destRect.X, destRect.Y + boxHeight, boxWidth, 20 }, 20, 60, 40, 20, Gdiplus::Unit::UnitPixel, &ia );

		_gdi->DrawImage( _itemShadow.get(), { destRect.X - 20, destRect.Y, 20, boxHeight }, 0, 20, 20, 40, Gdiplus::Unit::UnitPixel, &ia );
		_gdi->DrawImage( _itemShadow.get(), { destRect.X + boxWidth, destRect.Y, 20, boxHeight }, 60, 20, 20, 40, Gdiplus::Unit::UnitPixel, &ia );
	}

	// контур стекла
	const Gdiplus::Pen pen( Gdiplus::Color( theme->tileItemSelected.borderOpacity, GetRValue( theme->tileItemSelected.borderColor ), GetGValue( theme->tileItemSelected.borderColor ), GetBValue( theme->tileItemSelected.borderColor ) ), 1.0f );
	_gdi->DrawRectangle( &pen, destRect.X, destRect.Y, destRect.Width+1, destRect.Height+1 );


	const Gdiplus::Rect headerRect( destRect.X + 1, destRect.Y+1, destRect.Width-1, headerSize );
	Gdiplus::Rect footerRect( headerRect );
	footerRect.Y += boxHeight - headerSize - 1;

	const Gdiplus::Color presetColors[3] = { MakeColor( RGB( 190, 190, 190 ), 200 ), MakeColor( RGB( 255, 255, 255 ), 210 ), MakeColor( RGB( 190, 190, 190 ), 200 ) };
	Gdiplus::REAL blendPos[3] = { 0.0, 0.5, 1.0 };

	Gdiplus::LinearGradientBrush headerBrush( headerRect, presetColors[0], presetColors[1], Gdiplus::LinearGradientMode::LinearGradientModeHorizontal );

	headerBrush.SetInterpolationColors( presetColors, blendPos, _countof( blendPos ) );

	_gdi->FillRectangle( &headerBrush, headerRect );
	_gdi->FillRectangle( &headerBrush, footerRect );

	// контур хэдера и футера
	Gdiplus::Pen pen2( presetColors[0], 1.0f );
	_gdi->DrawLine( &pen2, headerRect.X, headerRect.Y + headerRect.Height, headerRect.X + headerRect.Width - 1, headerRect.Y + headerRect.Height );
	_gdi->DrawLine( &pen2, footerRect.X, footerRect.Y, footerRect.X + footerRect.Width - 1, footerRect.Y );

	
	Gdiplus::StringFormat format;
	format.SetAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );
	format.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );
	format.SetTrimming( Gdiplus::StringTrimming::StringTrimmingEllipsisPath );

	const Gdiplus::SolidBrush brush( MakeColor( RGB(63,63,63) ) );
	_gdi->DrawString( headerText.c_str(), headerText.length(), theme->tileItemSelected.fontTitle1, Gdiplus::RectF( headerRect.X, headerRect.Y, headerRect.Width, headerRect.Height ), &format, &brush );
	_gdi->DrawString( footerText.c_str(), footerText.length(), theme->tileItemSelected.fontTitle1, Gdiplus::RectF( footerRect.X, footerRect.Y, footerRect.Width, footerRect.Height ), &format, &brush );

	const Gdiplus::SolidBrush brushBody( MakeColor( theme->tileItemSelected.colorTitle1 ) );
	_gdi->DrawString( bodyText.c_str(), bodyText.length(), theme->tileItemSelected.fontTitle1, bodyRect, &format, &brushBody );
	_gdi->DrawString( bodyText.c_str(), bodyText.length(), theme->tileItemSelected.fontTitle1, bodyRect, &format, &brushBody );
}

void CVideoCatView::DrawThumbPreview( const CRect & clientRect )
{
	if( !_forcedIndex && _smallThumbs )
	{
		const Gdiplus::Rect destRect( clientRect.Width() / 2 - _thumbsWidth / 2, 0, _thumbsWidth, _thumbsHeight );

		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );

		const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
		_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );

		_gdi->DrawImage( _smallThumbs.get(), destRect, 0, 0, _smallThumbs->GetWidth(), _smallThumbs->GetHeight(), Gdiplus::UnitPixel );

		_gdi->SetCompositingMode( oldComposing );

		const Gdiplus::Pen pen( Gdiplus::Color( 255, 255, 255 ), 1.0f );
		_gdi->DrawLine( &pen, (INT)destRect.GetLeft()-1, (INT)destRect.GetTop(), (INT)destRect.GetLeft()-1, (INT)destRect.GetBottom()+1 );
		_gdi->DrawLine( &pen, (INT)destRect.GetLeft()-1, (INT)destRect.GetBottom()+1, (INT)destRect.GetRight()+1, (INT)destRect.GetBottom()+1 );
		_gdi->DrawLine( &pen, (INT)destRect.GetRight()+1, (INT)destRect.GetTop(), (INT)destRect.GetRight()+1, (INT)destRect.GetBottom()+1);
	}
}

void CVideoCatView::DrawDescription( const CRect & clientRect, const CRect & destRect, const DisplayItem & item )
{
	std::wstring genreNames;

	if( item._info->genres )
	{
		for( int i = (int)Genre::FirstGenre; i < (int)Genre::NumOfGenres; ++i )
		{
			const Genre genre = (Genre)i;

			if( Test( item._info->genres, genre ) )
			{
				const std::wstring genreName = Descript( genre ).GetString();
				genreNames += genreName;
				genreNames += L", ";
			}
		}
	}

	DrawTextBox( clientRect, destRect, item._info->tagline, item._info->description, genreNames );
}

void CVideoCatView::DrawTag( const CRect & clientRect, const CRect & destRect, const DisplayItem & item )
{
	CVideoCatDoc* doc = GetDocument();
	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	std::wstring tags;
	const TagManager & tm = cdb->GetTagManager();
	for( const TagId id : item._info->tags )
	{
		const std::wstring & tag = tm.GetTag( id );
		if( !tag.empty() )
		{
			tags += tag;
			tags += L", ";
		}
	}

	DrawTextBox( clientRect, destRect, item._info->tagline, item._info->comment, tags );
}

void CVideoCatView::DrawTechInfo( const CRect & clientRect, const CRect & destRect, const DisplayItem & item )
{
	SYSTEMTIME UTC, localTime;
	FileTimeToSystemTime( &item._info->date, &UTC );
	SystemTimeToTzSpecificLocalTime( nullptr, &UTC, &localTime );
	const std::wstring fileData = ResFormat( L"Дата добавления: %02d.%02d.%d", localTime.wDay, localTime.wMonth, localTime.wYear ).GetString();

	std::wstring shortFileInfo;
	BuildShortInfo( *item._info, shortFileInfo );

	DrawTextBox( clientRect, destRect, fileData, shortFileInfo, L"" );
}

void CVideoCatView::DrawBackground( const CRect & clientRect )
{
	if( GetGlobal().theme->bgImage )
	{
		if( GetGlobal().bgFullSize.Width() <= 0 )
			return;

		const CRect & treeRect = GetGlobal().bgTree;
		const double width = GetGlobal().theme->bgImage->GetWidth();
		const double height = GetGlobal().theme->bgImage->GetHeight();

		const CRect & viewRect = GetGlobal().bgView;

		const double w = width / GetGlobal().bgFullSize.Width();
		const double h = height / GetGlobal().bgFullSize.Height();
		const CRect srcRect( (int)(treeRect.Width() * w), 0, (int)width, (int)(viewRect.Height() * h) );

		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );
		const Gdiplus::CompositingMode oldMode = _gdi->GetCompositingMode();
		_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );

		const Gdiplus::Rect destRect( 0, 0, clientRect.Width(), clientRect.Height() );

		_gdi->DrawImage(
			GetGlobal().theme->bgImage,
			destRect,
			(INT)srcRect.left, (INT)srcRect.top, (INT)srcRect.Width(), (INT)srcRect.Height(),
			Gdiplus::Unit::UnitPixel );

		_gdi->SetCompositingMode( oldMode );
	}
	else
	{
		const Gdiplus::SolidBrush bgBrush( Gdiplus::Color(GetRValue( GetGlobal().theme->bgTopColor ), GetGValue( GetGlobal().theme->bgTopColor ), GetBValue( GetGlobal().theme->bgTopColor )) );

		_gdi->FillRectangle( &bgBrush, 0, 0, GetGlobal().bgView.Width(), GetGlobal().bgView.Height() );
	}
}	

void CVideoCatView::DrawToolbar( const CRect & clientRect )
{
	CVideoCatDoc * doc = GetDocument();
	if( !doc )
		return;

	const COLORREF iconColor = _listView ? GetGlobal().theme->listItem.colorTitle1 : GetGlobal().theme->tileItem.colorTitle1;
	const COLORREF selectedBG = iconColor;
	const COLORREF selectedText = (~iconColor) & 0x00FFffFF;

	constexpr int btnSize = 32;
	constexpr int btnOffset = btnSize + 16;
	constexpr int btnPosX = 8;

	std::vector<CommandID> commandButtons( GetGlobal().toolbarCommands );
	commandButtons.push_back( CommandID::EditToolbar );
		
	CPoint pos;
	GetCursorPos( &pos );
	ScreenToClient( &pos );
	ScalePoint( pos );

	const Gdiplus::SolidBrush brush( MakeColor( selectedBG, 220 ) );

	int btnPosY = 8;

	if( pos.y <= btnOffset && pos.x <= (LEFT_OFFSET-8) )
	{
		Gdiplus::Rect rect( 0, 0, clientRect.Width(), 64 );
		_gdi->FillRectangle( &brush, rect );

		GetIconManager().Draw( *_gdi, IconManager::GoBack, btnPosX, btnPosY, 48, selectedText );

		const Gdiplus::RectF textRect( 64, rect.GetTop(), rect.Width - 64, rect.Height );

		const Gdiplus::SolidBrush textBrush( MakeColor( selectedText ) );
		Gdiplus::StringFormat format;
		format.SetAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
		format.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );
		format.SetTrimming( Gdiplus::StringTrimming::StringTrimmingEllipsisPath );
		
		const CString & title = doc->GetCurrentCatFolder();

		_gdi->DrawString( title, title.GetLength(), GetGlobal().theme->fontCommandbar, textRect, &format, &textBrush );

		// тени под тулбаром
		{
			constexpr COLORREF shadowColor = RGB( 0, 0, 0 );

			Gdiplus::ColorMatrix cm;
			ZeroMemory( &cm.m, sizeof( cm ) );
			cm.m[0][0] = GetRValue( shadowColor ) / 255.0f; // r
			cm.m[1][1] = GetGValue( shadowColor ) / 255.0f; // g
			cm.m[2][2] = GetBValue( shadowColor ) / 255.0f; // b
			cm.m[3][3] = 120.0f / 255.0f; // a
			cm.m[4][4] = 1.0f;

			Gdiplus::ImageAttributes ia;
			ia.SetColorMatrix( &cm );

			_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );
			_gdi->DrawImage( _itemShadow.get(), { 0, rect.GetBottom(), rect.Width, 20 }, 30, 60, 20, 20, Gdiplus::Unit::UnitPixel, &ia );
		}
	}
	else
	{
		GetIconManager().Draw( *_gdi, IconManager::GoBack, btnPosX, btnPosY, 48, iconColor );
	}


	btnPosY += 64 + 8;

	CRect btnRect( 8, btnPosY - 8, 8 + btnSize, btnPosY + btnSize + 8 );

	for( const CommandID id : commandButtons )
	{
		const IconID iconId = CommandExecute::Instance().GetIcon( id );

		if( !btnRect.PtInRect( pos ) )
		{
			GetIconManager().Draw( *_gdi, iconId, btnPosX, btnPosY, btnSize, iconColor );
		}
		else
		{
			const CString & title = CommandExecute::Instance().GetTitle( id );

			Gdiplus::RectF boundingBox;
			_gdi->MeasureString( title, title.GetLength(), GetGlobal().theme->fontCommandbar, Gdiplus::PointF( 0.f, 0.f ), &boundingBox );

			Gdiplus::Rect rect( 0, btnRect.top, boundingBox.Width + LEFT_OFFSET + LEFT_OFFSET, btnOffset );

			_gdi->FillRectangle( &brush, rect );

			GetIconManager().Draw( *_gdi, iconId, btnPosX, btnPosY, btnSize, selectedText );

			Gdiplus::RectF textRect( LEFT_OFFSET, rect.GetTop(), rect.Width - LEFT_OFFSET, rect.Height );
			Gdiplus::SolidBrush textBrush( MakeColor( selectedText ) );
			Gdiplus::StringFormat format;
			format.SetAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
			format.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );
			format.SetTrimming( Gdiplus::StringTrimming::StringTrimmingEllipsisPath );

			_gdi->DrawString( title, title.GetLength(), GetGlobal().theme->fontCommandbar, textRect, &format, &textBrush );
		}

		btnPosY += btnOffset;
		btnRect.OffsetRect( 0, btnOffset );
	}
}

void CVideoCatView::OnDraw( CDC* pDC )
{
	VC_TRY;

	CVideoCatDoc* doc = GetDocument();
	//ASSERT_VALID(doc);
	if( !doc )
		return;

	//static std::ofstream out( L"T:\\times.txt" );

	//DWORD st = GetTickCount();

	const CPoint pos( 0, ScaleInt( _scrollHelper.GetScrollPos().cy ) );

	CRect clientRect;
	GetClientRect( clientRect );

	CRect screenRect( clientRect );
	ScaleRect( screenRect );

	if( !_gdi )
	{
		_gdiBuffer = std::make_unique<Gdiplus::Bitmap>(  clientRect.Width(), clientRect.Height(), PixelFormat32bppPARGB  );
		_gdi.reset( Gdiplus::Graphics::FromImage( _gdiBuffer.get() ) );

		_bmpBackBuffer.CreateCompatibleBitmap( pDC, clientRect.Width(), clientRect.Height() );
		_backbuffer.CreateCompatibleDC( pDC );
		_backbuffer.SelectObject( _bmpBackBuffer );

		_gdi->SetPageUnit( Gdiplus::Unit::UnitPixel );
		_gdi->SetPageScale( ((double)_pageScale) / 100.0 );


		_gdi->SetSmoothingMode( Gdiplus::SmoothingMode::SmoothingModeNone );// _pageScale == 100 ? Gdiplus::SmoothingMode::SmoothingModeNone : Gdiplus::SmoothingMode::SmoothingModeAntiAlias );
		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );// _pageScale == 100 ? Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor : Gdiplus::InterpolationMode::InterpolationModeBilinear);
		_gdi->SetPixelOffsetMode( Gdiplus::PixelOffsetMode::PixelOffsetModeHalf );
		_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceOver );
		_gdi->SetCompositingQuality( Gdiplus::CompositingQuality::CompositingQualityHighSpeed );
		_gdi->SetTextRenderingHint( Gdiplus::TextRenderingHint::TextRenderingHintClearTypeGridFit );
	}


	const bool isBigPreview = _drawThumbBox && (_thumbIndex != -1);

	if( isBigPreview )
	{
		Gdiplus::SolidBrush bgBrush( MakeColor(RGB(0,0,0)) );
		_gdi->FillRectangle( &bgBrush, 0, 0, screenRect.Width(), screenRect.Height() );

		const int thumbWidth = _thumbs->GetWidth();
		int thumbHeight = _thumbs->GetHeight() / 10;

		constexpr int srcX = 0;
		int offsetY = 0;
		// смещение для поддержки старых скриншотов
		if( _thumbs->GetHeight() != (thumbHeight * 10) )
		{
			thumbHeight = (_thumbs->GetHeight() - 18) / 10;
			offsetY = 18;
		}

		CRect destRect( screenRect );
		const int scale = destRect.Width() * 100 / thumbWidth;
		const int scaledH = thumbHeight * scale / 100;
		destRect.top = (screenRect.Height() - scaledH) / 2;
		destRect.bottom = screenRect.bottom - destRect.top;

		CRect srcRect( 0, offsetY, thumbWidth, thumbHeight + offsetY );
		srcRect.OffsetRect( 0, _thumbIndex * thumbHeight );

		const Gdiplus::Rect dest( destRect.left, destRect.top, destRect.Width(), destRect.Height() );

		_gdi->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );

		const Gdiplus::CompositingMode oldComposing = _gdi->GetCompositingMode();
		_gdi->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );

		_gdi->DrawImage( _thumbs.get(), dest, srcRect.left, srcRect.top, srcRect.Width(), srcRect.Height(), Gdiplus::UnitPixel );

		_gdi->SetCompositingMode( oldComposing );

		DrawThumbPreview( screenRect );

	}
	else
	{	
		DrawBackground( screenRect );

		if( doc->Ready() )
		{
			if( !_listView )
				DrawTiles( screenRect, pos );
			else
				DrawList( screenRect, pos );

			DrawToolbar( screenRect );
		}

	}

	Gdiplus::Graphics g2( _backbuffer.GetSafeHdc() );
	g2.SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor );
	g2.SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );
	g2.SetCompositingQuality( Gdiplus::CompositingQuality::CompositingQualityHighSpeed );
	g2.SetSmoothingMode( Gdiplus::SmoothingMode::SmoothingModeNone );
	g2.SetPixelOffsetMode( Gdiplus::PixelOffsetMode::PixelOffsetModeHalf );

	g2.DrawImage( _gdiBuffer.get(), 0, 0, 0, 0, clientRect.Width(), clientRect.Height(), Gdiplus::UnitPixel );

	pDC->BitBlt( 0, 0, clientRect.Width(), clientRect.Height(), &_backbuffer, 0, 0, SRCCOPY );


	//DWORD et = GetTickCount();
	//out << ((et - st)) << "\n";

	VC_CATCH_ALL;
}

void CVideoCatView::OnInitialUpdate()
{
	__super::OnInitialUpdate();

	_bmpRealFolder.reset( LoadPng( IDB_FOLDER ) );
	_bmpLockedFolder.reset( LoadPng( IDB_FOLDER_LOCKED ) );
	_bmpVirtualFolder.reset( LoadPng( IDB_FOLDER_VIRTUAL ) );
	_bmp10.reset( LoadPng( IDB_10 ) );
	_bgDescription.reset( LoadPng( IDB_BG_DESCRIPTION ) );

	_noPosterTile.reset( LoadPng( IDB_NOPOSTER ) );
	_noPosterTileTV.reset( LoadPng( IDB_NOPOSTER ) );
	_noPosterList.reset( LoadPng( IDB_NOPOSTER_SMALL ) );
	_noPosterListTV.reset( LoadPng( IDB_EPISODE_SMALL ) );

	_itemShadow.reset( LoadPng(IDB_SHADOW) );

	if( static_cast<int>((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 31) == 1 )
	{
		// иногда показываем страничку с просьбой поддержать
		::ShellExecute( nullptr, L"open", L"https://videocat.maagames.ru/purchase.html", nullptr, nullptr, SW_SHOWNORMAL );
	}
	else
	{
		ShowTipOfDay();
	}
}

void CVideoCatView::OnRButtonUp( UINT /* nFlags */, CPoint point )
{
	VC_TRY;

	OnContextMenu( nullptr, point );

	VC_CATCH_ALL;
}

void CVideoCatView::OnContextMenu( CWnd* /* pWnd */, CPoint point )
{
	VC_TRY;

	CRect clientRect;
	GetClientRect( clientRect );
	ScaleRect( clientRect );

	ScalePoint( point );

	// размещаем посередине окна
	CRect rect( 0, 0, _thumbsWidth, _thumbsHeight );
	rect.OffsetRect( clientRect.Width() / 2 - rect.Width() / 2, 0 );
	if( rect.PtInRect( point ) )
	{
		// игнорируем клик на превьюшке кадров
		return;
	}


	const int index = GetItemIndex( point );
	if( index < 0 )
		return;

	CSize pos = _scrollHelper.GetScrollPos();
	pos.cx = ScaleInt( pos.cx );
	pos.cy = ScaleInt( pos.cy );

	if( index != _currentItem )
	{
		SetCurrentItem( index );
		UpdateView();
	}

	const int hor = _currentItem % _numHorz;
	const int ver = _currentItem / _numHorz;
	CRect itemRect = (_listView ? DisplayItem::GetListRect( hor, ver ) : DisplayItem::GetTileRect( hor, ver )) - CPoint( -LEFT_OFFSET, pos.cy - TOP_OFFSET + 32 );
	
	const ItemStyle & style = _listView ? GetGlobal().theme->listItemSelected : GetGlobal().theme->tileItemSelected;

	itemRect.left += style.button[1].x - 48;
	itemRect.top += style.button[1].y - 16;
	itemRect.left = (itemRect.left * _pageScale) / 100;
	itemRect.top = (itemRect.top * _pageScale) / 100;

	ClientToScreen( &itemRect );

	CVideoCatDoc* doc = GetDocument();
	const FilteredFileItems & displayItems = doc->GetFilteredFiles();
	if( displayItems.empty() )
		return;

	const Entry * entry = displayItems.at( _currentItem )._info;
	if( !entry )
		return;

	adv_mfc::AdvancedMenu menu;
	menu.CreatePopupMenu();

	if( entry->IsFolder() )
		CreateFolderMenu( menu, *entry, false );
	else
		CreateFileMenu( menu, *entry );

	menu.TrackPopupMenuEx( TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, itemRect.left, itemRect.top, this, nullptr );

	VC_CATCH_ALL;
}


#ifdef _DEBUG
void CVideoCatView::AssertValid() const
{
	__super::AssertValid();
}

void CVideoCatView::Dump( CDumpContext& dc ) const
{
	__super::Dump( dc );
}

CVideoCatDoc* CVideoCatView::GetDocument() const
{
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CVideoCatDoc ) ) );
	return (CVideoCatDoc*)m_pDocument;
}
#endif //_DEBUG


void CVideoCatView::OnSize( UINT nType, int cx, int cy )
{
	VC_TRY;

	__super::OnSize( nType, cx, cy );

	_scrollHelper.OnSize( nType, cx, cy );

	RecalcScrollLayout();

	VC_CATCH_ALL;
}

void CVideoCatView::RecalcScrollLayout( BOOL scrollTop /*= FALSE*/ )
{
	_gdi.reset(nullptr);
	_gdiBuffer.reset(nullptr);

	_backbuffer.DeleteDC();
	_bmpBackBuffer.DeleteObject();	

	CVideoCatDoc* doc = GetDocument();
	if( !doc )
		return;

	const ViewStyle * theme = GetGlobal().theme;

	if( !theme )
		return;

	_pageScale = GetGlobal().viewScale;

	CRect clientRect;
	GetScreenRect( clientRect );

	_thumbsWidth = BIG_THUMB_WIDTH_FULL;
	if( _thumbsWidth > (clientRect.Width() - ScaleInt(256)) )
	{
		_thumbsWidth = clientRect.Width() - ScaleInt(256);
		if( _thumbsWidth < 0 )
			_thumbsWidth = 0;
	}

	_thumbsHeight = (BIG_THUMB_HEIGHT * _thumbsWidth) / BIG_THUMB_WIDTH_FULL;


	const int numFiles = (int)doc->GetFilteredFiles().size();

	int itemWidth = !_listView ? theme->tileItem.width + theme->tileItem.offset : clientRect.Width() - ScaleInt(128);
	if( itemWidth <= 0 )
		itemWidth = 128;

	int itemHeight = !_listView ? theme->tileItem.height + theme->tileItem.offset : theme->listItem.height + theme->listItem.offset;
	if( itemHeight <= 0 )
		itemHeight = 128;

	_numHorz = (clientRect.Width() - LEFT_OFFSET) / itemWidth;
	if( _numHorz == 0 )
		_numHorz = 1;

	_numVert = (clientRect.Height() + itemHeight/2) / itemHeight;
	if( _numVert == 0 )
		_numVert = 1;

	const int vertCount = (numFiles + _numHorz) / _numHorz;

	const int horz = _numHorz * itemWidth;
	const int vert = vertCount * itemHeight;

	_scrollHelper.SetDisplaySize( (horz * _pageScale) / 100, (vert + itemHeight) * _pageScale / 100 );
	_scrollHelper.SetPageSize( (horz * _pageScale) / 100, (clientRect.Height()) * _pageScale / 100 );

	if( numFiles != _numTotal )
	{
		_numTotal = numFiles;
		SetCurrentItem( 0 );

		_scrollHelper.ScrollToOrigin( true, true );
	}
	else if( scrollTop )
	{
		_scrollHelper.ScrollToOrigin( true, true );
	}
}

void CVideoCatView::SelectFirst()
{
	SetCurrentItem( 0 );
}

void CVideoCatView::RefreshDisplayList( Entry * selectFileInfo )
{
	CVideoCatDoc * doc = GetDocument();
	if( !doc )
		return;

	CollectionDB * cdb = doc->GetCurrentCollection();
	if( !cdb )
		return;

	_ait = AIT_NONE;

	int selectIndex = -1;
	int num = 0;

	FilteredFileItems & files = doc->GetFilteredFiles();

	for( DisplayItem & item : files )
	{
		if( item._info == selectFileInfo )
		{
			selectIndex = num;
		}
		++num;

		if( !item._info->IsFolder() )
			continue;

		Traverse::CountViewedFilms countFilms;
		cdb->TraverseAll( item._info->thisEntry, &countFilms );
		item._totalFilms = countFilms.totalFilms;
		item._viewedFilms = countFilms.viewed;
	}

	RecalcScrollLayout( TRUE );

	if( selectIndex != -1 )
	{
		SetCurrentItem( selectIndex );
		const int selectedLine = selectIndex / _numHorz;
		if( selectedLine >= _numVert )
		{
			const int posY = selectedLine * (_listView ? DisplayItem::ListItemHeight : DisplayItem::ItemHeight );

			_scrollHelper.ScrollToPositonY( (posY * _pageScale) / 100 );
		}
	}
	else
	{
		SelectFirst();
	}

	UpdateView();
}

BOOL CVideoCatView::OnEraseBkgnd( CDC* /*pDC*/ )
{
	return TRUE;
}

int CVideoCatView::GetItemIndex( const CPoint & point ) const
{
	// чтобы при даблклике на "назад" не выбирался элемент
	if( point.x < 64 )
		return -1;

	CVideoCatDoc * doc = GetDocument();
	if( !doc )
		return -1;

	const ViewStyle * theme = GetGlobal().theme;
	if( !theme )
		return -1;

	//int numFiles = (int)doc->GetFilteredFiles().size();

	int itemWidth = 0;
	int itemHeight = 0;

	if( !_listView )
	{
		itemWidth = theme->tileItem.width + theme->tileItem.offset;
		itemHeight = theme->tileItem.height + theme->tileItem.offset;
	}
	else
	{
		CRect clientRect;
		GetScreenRect( clientRect );

		itemWidth = clientRect.Width() - ScaleInt( 128 );
		itemHeight = theme->listItem.height + theme->listItem.offset;
	}

	if( itemWidth <= 0 )
		itemWidth = 128;
	if( itemHeight <= 0 )
		itemHeight = 128;

	CPoint pos( 0, ScaleInt( _scrollHelper.GetScrollPos().cy ) );

	pos += point;
	pos.y -= TOP_OFFSET;
	const int x = (pos.x - LEFT_OFFSET) / itemWidth;
	if( x >= _numHorz )
		return -1;

	const int y = pos.y / itemHeight;
	const int index = y * _numHorz + x;

	return index < _numTotal ? index : -1;
}


void CVideoCatView::OnLButtonUp( UINT /*nFlags*/, CPoint point )
{
	VC_TRY;
	
	{
		_downPos -= point;
		if( abs( _downPos.x ) > 8 )
			return;

		if( abs( _downPos.y ) > 8 )
			return;
	}

	CVideoCatDoc * doc = GetDocument();
	if( !doc )
		return;

	CPoint screenPoint( point );
	ScalePoint( screenPoint );

	CommandInfo info;
	info.doc = doc;

	if( _currentItem >= 0 )
	{
		FilteredFileItems & displayItems = doc->GetFilteredFiles();
		if( displayItems.empty() )
			return;

		DisplayItem & item = displayItems.at( _currentItem );

		info.entry = item._info;
		info.displayItem = &item;
	}
	else
	{
		// Если никакой элемент не выбран или их нет, то вызываем команду в контекстеродительской папки
		info.entry = doc->GetVideoTreeView()->GetCurrentItem();
	}

	if( _ait == AIT_TOOLBAR )
	{
		if( screenPoint.y < 64 )
		{
			CVideoCatDoc* doc = GetDocument();
			CVideoTreeView * treeView = doc->GetVideoTreeView();
			if( treeView )
				treeView->GoBack();

			return;
		}

		std::vector<CommandID> commandButtons( GetGlobal().toolbarCommands );
		commandButtons.push_back( CommandID::EditToolbar );

		constexpr int offsetY = 32 + 16;

		CRect btnRect( 8, 64 + 8, 8 + offsetY, 56 + 8 + offsetY );

		for( CommandID commandId : commandButtons )
		{
			if( btnRect.PtInRect( screenPoint ) )
			{
				CommandExecute::Instance().Process( commandId, &info );
				return;
			}
			btnRect.OffsetRect( 0, offsetY );
		}
	}

	const int index = GetItemIndex( screenPoint );
	if( index < 0 )
		return;

	if( index != _currentItem )
	{
		SetCurrentItem( index );
		UpdateView();
		return;
	}

	if( _ait == AIT_NONE )
		return;

	if( _ait == AIT_MENU )
	{
		OnContextMenu( nullptr, point );
		return;
	}

	static constexpr CommandID commandID[] = {
		CommandID::None, // AIT_NONE = 0,
		CommandID::OpenEntry, // AIT_PLAY,
		CommandID::None, // AIT_MENU,
		CommandID::EditSingleEntry, // AIT_DESCRIPTION,
		CommandID::EditTagInfo, // AIT_TAG,
		CommandID::EditTechInfo, // AIT_TECHINFO,
		CommandID::OpenYoutube, // AIT_YOUTUBE,
		CommandID::None	// AIT_THUMBS,
	};

	CommandExecute::Instance().Process( commandID[_ait], &info );

	UpdateView();

	VC_CATCH_ALL;
}

void BuildThumbPreviewThread( CVideoCatView * view )
{
	CVideoCatDoc * doc = view->GetDocument();
	if( !doc )
	{
		return;
	}

	const FilteredFileItems & displayItems = doc->GetFilteredFiles();
	if( displayItems.empty() )
		return;

	const DisplayItem & item = displayItems.at( view->_currentItem );

	CString thumbName = doc->GetThumbDir();
	thumbName += item._info->thisEntry.ToBase64() + L".jpg";
	if( PathFileExists( thumbName.GetString() ) )
	{
		std::lock_guard lock( view->_mutex );

		Gdiplus::Bitmap * thumb = LoadBitmapFromFile( thumbName );
		if( thumb )
		{
			view->_thumbs.reset( thumb );

			view->BuildThumbsPreview();

			view->_drawThumbBox = true;

			view->UpdateView();
		}
	}
}


void CVideoCatView::BuildThumbsPreview()
{
	_smallThumbs.reset();

	Gdiplus::Bitmap * full = _thumbs.get();

	if( !full )
		return;

	Gdiplus::Bitmap * thumbs = new Gdiplus::Bitmap(BIG_THUMB_WIDTH_FULL, BIG_THUMB_HEIGHT, PixelFormat32bppPARGB);

	const UINT bmpWidth =  full->GetWidth();
	const UINT bmpHeight = full->GetHeight();

	int srcHeight = bmpHeight / 10;

	constexpr int srcX = 0;
	int srcY = 0;
	// смещение для поддержки старых скриншотов
	if( bmpHeight != (srcHeight * 10) )
	{
		srcHeight = (bmpHeight - 18) / 10;
		srcY = 18;
	}

	std::unique_ptr<Gdiplus::Graphics> dc( Gdiplus::Graphics::FromImage( thumbs ) );
	dc->SetInterpolationMode( Gdiplus::InterpolationMode::InterpolationModeBilinear );
	dc->SetCompositingMode( Gdiplus::CompositingMode::CompositingModeSourceCopy );


	const int destX = 0;
	const int destY = 0;
	const int destWidth = BIG_THUMB_WIDTH;

	Gdiplus::RectF srcRect( srcX, srcY, bmpWidth, srcHeight );
	Gdiplus::RectF destRect( destX, destY, destWidth, BIG_THUMB_HEIGHT );

	const Gdiplus::Pen pen( Gdiplus::Color( 0, 0, 0 ), 2.0f );

	for( int idx = 0; idx < 10; ++idx )
	{
		dc->DrawImage( full, destRect, srcRect, Gdiplus::Unit::UnitPixel, nullptr );

		dc->DrawRectangle( &pen, destRect );

		srcRect.Offset( 0, srcHeight );
		destRect.Offset( destWidth, 0 );
	}

	_smallThumbs.reset( thumbs );
}

void CVideoCatView::SetCurrentItem( int index )
{
	CWaitCursor waiting;

	std::lock_guard lock( _mutex );

	_drawThumbBox = false;

	_currentItem = index;
	if( _currentItem < 0 )
		_currentItem = 0;
	if( _currentItem >= _numTotal )
		_currentItem = _numTotal - 1;

	if( _currentItem < 0 )
		return;

	CVideoCatDoc* doc = GetDocument();
	if( !doc )
	{
		return;
	}

	const FilteredFileItems & displayItems = doc->GetFilteredFiles();
	if( displayItems.empty() )
		return;

	const DisplayItem & item = displayItems.at( _currentItem );

	if( item._info->IsFolder() )
	{
		return;
	}

	_thumbs.reset();
	_smallThumbs.reset();

	_drawThumbBox = false;
	_thumbIndex = -1;

	std::thread thumbThread( BuildThumbPreviewThread, this );
	thumbThread.detach();
}

void CVideoCatView::OnLButtonDblClk( UINT /*nFlags*/, CPoint point )
{
	VC_TRY;

	CVideoCatDoc* doc = GetDocument();
	if( !doc )
		return;

	ScalePoint( point );

	FilteredFileItems & displayItems = doc->GetFilteredFiles();

	if( point.x < 64 )
	{
		_scrollHelper.ScrollToOrigin( true, true );

		CVideoTreeView * treeView = doc->GetVideoTreeView();
		if( !treeView )
			return;

		if( displayItems.empty() )
		{
			treeView->GoBack();
			return;
		}

		const DisplayItem & item = displayItems.at( _currentItem );

		if( doc->IsFilterDirectory() )
		{
			// Переходим в родительскую директорию
			treeView->GoBack();
		}
		else
		{
			CollectionDB * cdb = doc->GetCurrentCollection();
			if( cdb )
			{
				// Если фильтр по имени или жанру, то переходим в папку с этим фильмом
				Entry * parent = cdb->FindEntry( item._info->parentEntry );
				treeView->SelectItem( parent );
			}
		}

		return;
	}

	const int index = GetItemIndex( point );
	if( index < 0 )
		return;

	SetCurrentItem( index );

	if( displayItems.empty() )
		return;

	const DisplayItem & item = displayItems.at( _currentItem );

	CommandInfo info;
	info.doc = doc;
	info.entry = item._info;

	CommandExecute::Instance().Process( CommandID::OpenEntry, &info );

	VC_CATCH_ALL;
}

void CVideoCatView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	VC_TRY;

	__super::OnKeyDown( nChar, nRepCnt, nFlags );

	const int itemWidth = !_listView ? DisplayItem::ItemWidth : DisplayItem::ListItemWidth;
	const int itemHeight = !_listView ? DisplayItem::ItemHeight : DisplayItem::ListItemHeight;

	CRect r;
	GetScreenRect( r );

	int pageSize = (r.Height() / itemHeight) * _numHorz;
	if( pageSize < _numHorz )
		pageSize = _numHorz;

	int selectItem = -1;

	switch( nChar )
	{
		case VK_LEFT:
			selectItem = _currentItem - 1;
			break;
		case VK_UP:
			selectItem = _currentItem - _numHorz;
			break;
		case VK_RIGHT:
			selectItem = _currentItem + 1;
			break;
		case VK_DOWN:
			selectItem = _currentItem + _numHorz;
			break;

		case VK_BACK:
		{
			CVideoCatDoc* doc = GetDocument();
			if( !doc )
				return;

			CVideoTreeView * treeView = doc->GetVideoTreeView();

			if( doc->IsFilterDirectory() )
			{
				// Переходим в родительскую директорию
				treeView->GoBack();
			}
			else
			{
				FilteredFileItems & displayItems = doc->GetFilteredFiles();
				if( displayItems.empty() )
				{
					treeView->GoBack();
					return;
				}

				const DisplayItem & item = displayItems.at( _currentItem );

				// Если фильтр по имени или жанру, то переходим в папку с этим фильмом
				CollectionDB * cdb = doc->GetCurrentCollection();
				if( cdb )
				{
					// Если фильтр по имени или жанру, то переходим в папку с этим фильмом
					Entry * parent = cdb->FindEntry( item._info->parentEntry );
					treeView->SelectItem( parent );
				}
			}

			return;
		}

		case VK_RETURN:
		{
			CVideoCatDoc * doc = GetDocument();

			FilteredFileItems & displayItems = doc->GetFilteredFiles();
			if( displayItems.empty() )
				return;

			const DisplayItem & item = displayItems.at( _currentItem );

			CommandInfo info;
			info.doc = doc;
			info.entry = item._info;

			CommandExecute::Instance().Process( CommandID::OpenEntry, &info );

			return;
		}
		case VK_HOME:
		{
			selectItem = 0;
			break;
		}
		case VK_END:
		{
			selectItem = _numTotal - 1;
			break;
		}
		case VK_PRIOR:
		{
			selectItem = _currentItem - pageSize;
			break;
		}
		case VK_NEXT:
		{
			selectItem = _currentItem + pageSize;
			break;
		}

		case VK_F2:
		{
			AfxGetApp()->GetMainWnd()->SetWindowPos( nullptr, 128, 128, 1280, 720, SWP_SHOWWINDOW );
			return;
		}

		case VK_F3:
		{
			CVideoCatDoc* doc = GetDocument();
			if( !doc )
				return;

			FilteredFileItems & displayItems = doc->GetFilteredFiles();
			if( displayItems.empty() )
				return;

			DisplayItem & item = displayItems.at( _currentItem );
			item._info->numViews--;
			doc->InvalidateProgressInfo();

			return;
		}

		case VK_F4:
		{
			CVideoCatDoc* doc = GetDocument();
			if( !doc )
				return;

			FilteredFileItems & displayItems = doc->GetFilteredFiles();
			if( displayItems.empty() )
				return;

			DisplayItem & item = displayItems.at( _currentItem );
			item._info->numViews++;
			doc->InvalidateProgressInfo();

			return;
		}

		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'0':
		{
			if( _drawThumbBox )
			{
				_ait = AIT_THUMBS;
				_thumbIndex = (nChar != L'0') ? nChar - L'1' : 9;
				_forcedIndex = true;
			}
			return;
		}

		default:
			return;
	}

	_forcedIndex = false;


	if( selectItem < 0 )
		selectItem = 0;
	else if( selectItem >= _numTotal )
		selectItem = _numTotal - 1;
	
	SetCurrentItem( selectItem );

	const int newLine = selectItem / _numHorz;
	const int topY = (selectItem / _numHorz) * itemHeight;
	const int botY = topY + itemHeight;

	r.OffsetRect( 0, ScaleInt( _scrollHelper.GetScrollPos().cy ) );

	const bool isVisible = r.PtInRect( CPoint( 100, topY ) ) && r.PtInRect( CPoint( 100, botY ) );
	if( !isVisible )
	{
		_scrollHelper.ScrollToPositonY( (topY * _pageScale) / 100 );
	}

	UpdateView();
	
	VC_CATCH_ALL;
}

CVideoCatView::ActiveItemType CVideoCatView::TestItemButtons( const Entry & entry, const ItemStyle & style, const CRect & itemRect, const CPoint & point )
{
	const int btnSize = style.btnSize;
	const auto & btnPos = style.button;

	static const ActiveItemType id[] = { AIT_PLAY, AIT_MENU, AIT_DESCRIPTION, AIT_TAG, AIT_YOUTUBE, AIT_TECHINFO };
	bool visibleButton[6] = { true, true, true, true, true, true };
	visibleButton[2] = !entry.description.empty() || !entry.tagline.empty() || entry.genres;
	visibleButton[3] = !entry.tags.empty() || !entry.comment.empty();
	visibleButton[5] = !entry.techInfo.empty();

	for( int idx = 0; idx < _countof( id ); ++idx )
	{
		if( !visibleButton[idx] )
			continue;

		// кнопка воспроизведения, папки или сериала
		const int cx = itemRect.left + btnPos[idx].x;
		const int cy = itemRect.top + btnPos[idx].y;

		if( point.x > cx && point.x < (cx + btnSize) && point.y > cy && point.y < (cy + btnSize) )
			return id[idx];
	}

	return ActiveItemType::AIT_NONE;
}

BOOL CVideoCatView::OnMouseWheel( UINT f, short zDelta, CPoint p )
{
	_scrollHelper.OnMouseWheel( f, zDelta*_pageScale / 100, p );

	UpdateView();

	return TRUE;
}


void CVideoCatView::OnMouseMove( UINT /*nFlags*/, CPoint point )
{
	static int lastX = -100;
	if( _forcedIndex && (abs( lastX - point.x ) < 5) )
		return;

	_forcedIndex = false;
	lastX = point.x;

	ScalePoint( point );

	const int oldThumb = _thumbIndex;
	_thumbIndex = -1;

	const ActiveItemType oldItemType = _ait;
	_ait = AIT_NONE;

	if( point.x < (LEFT_OFFSET - 8) )
	{
		_ait = AIT_TOOLBAR;
		UpdateView();
		return;
	}

	// Сперва проверяю, не находится ли курсор над иконками кадров
	{
		CRect clientRect;
		GetScreenRect( clientRect );

		// размещаем посередине окна
		CRect rect( 0, 0, _thumbsWidth, _thumbsHeight );
		rect.OffsetRect( clientRect.Width() / 2 - rect.Width() / 2, 0 );
		if( _thumbsWidth > 0 && rect.PtInRect( point ) )
		{
			// если курсор внутри иконок кадров, то вычисляем индекс
			const int index = ((point.x - rect.left) * 10) / rect.Width();
			if( index >= 0 && index < 10 )
			{
				_ait = AIT_THUMBS;
				// если индекс не изменился, то незачем тратить ресурсы на перерисовку
				if( oldThumb != index )
				{
					_thumbIndex = index;
					UpdateView();
				}
			}
			return;
		}
		else if( oldItemType == AIT_THUMBS )
		{
			UpdateView();
		}
	}

	if( _currentItem < 0 )
		return;

	CVideoCatDoc* doc = GetDocument();
	const FilteredFileItems & displayItems = doc->GetFilteredFiles();

	if( displayItems.empty() )
		return;

	const int hor = _currentItem % _numHorz;
	const int ver = _currentItem / _numHorz;

	CSize pos = _scrollHelper.GetScrollPos();
	pos.cy = ScaleInt( pos.cy );


	CRect itemRect = (_listView ? DisplayItem::GetListRect( hor, ver ) : DisplayItem::GetTileRect( hor, ver )) - CPoint(0,pos.cy);
	itemRect.OffsetRect( LEFT_OFFSET, TOP_OFFSET );

	const ItemStyle & style = _listView ? GetGlobal().theme->listItemSelected : GetGlobal().theme->tileItemSelected;

	_ait = TestItemButtons( *displayItems[_currentItem]._info, style, itemRect, point );

	if( oldItemType != _ait )
	{
		UpdateView();
	}

	switch( _ait )
	{
		case AIT_PLAY:
		case AIT_MENU:
		case AIT_YOUTUBE:
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_HAND ) );
		break;
		case AIT_DESCRIPTION:
		case AIT_TAG:
		case AIT_TECHINFO:
			SetCursor( AfxGetApp()->LoadStandardCursor( nullptr ) );
			break;
		default:
			break;
	}

}

void CVideoCatView::OnLButtonDown( UINT nFlags, CPoint point )
{
	_downPos = point;

	CView::OnLButtonDown( nFlags, point );
}


BOOL CVideoCatView::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if( HIWORD( wParam ) == 0 && (lParam == 0) )
	{
		if( AllCommandsDisabled() )
			return TRUE;

		const DWORD menuId = LOWORD( wParam );

		CVideoCatDoc* doc = GetDocument();
		if( !doc )
			return TRUE;
		
		if( _currentItem < 0 )
			return TRUE;

		FilteredFileItems & displayItems = doc->GetFilteredFiles();
		if( displayItems.empty() )
			return TRUE;

		if( _currentItem >= displayItems.size() )
			return TRUE;

		DisplayItem & item = displayItems[ _currentItem ];

		CommandInfo info;
		info.doc = doc;
		info.entry = item._info;
		info.displayItem = &item;

		CommandExecute::Instance().Process( (CommandID)menuId, &info );

		return TRUE;
	}

	return CView::OnCommand( wParam, lParam );
}



void CVideoCatView::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar * pScrollBar )
{
	_scrollHelper.OnVScroll( nSBCode, nPos, pScrollBar );

	UpdateView();
}


void CVideoCatView::PostNcDestroy()
{
	_scrollHelper.DetachWnd();

	CView::PostNcDestroy();
}


BOOL CVideoCatView::PreCreateWindow( CREATESTRUCT & cs )
{
	cs.style |= WS_VSCROLL;

	return CView::PreCreateWindow( cs );
}
