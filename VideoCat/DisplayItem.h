#pragma once

#include <memory>

#include "Entry.h"


class DisplayItem
{
public:
	static CRect GetTileRect( int x, int y );
	static CRect GetListRect( int x, int y );

	static CRect GetFolderRect( const CRect & item );
	static CRect GetPosterRect( const CRect & item, int width, int height );

public:
	static const int ItemWidth = 270;		///< Ширина объекта
	static const int ItemHeight = 270;	///< Высота объекта

	static const int ListItemWidth = 510;		///< Ширина объекта
	static const int ListItemHeight = 110;	///< Высота объекта

private:
	static const CRect _tileBox;	///< Габаритный контейнер объекта
	static const CRect _listBox;	///< Габаритный контейнер для элемента списка
	static const CRect _folderRect;	///< Размер и положение кнопки, относительно габаритного контейнера

	static CSize GetOffset( int x, int y );
	static CSize GetListOffset( int x, int y );

public:
	DisplayItem( Entry * fi = nullptr )
		: _info( fi )
		, _viewedFilms( 0 )
		, _totalFilms( 0 )
	{	}

public:
	std::shared_ptr<Gdiplus::Bitmap> _poster;
	Entry * _info;

	unsigned _viewedFilms;
	unsigned _totalFilms;
};

