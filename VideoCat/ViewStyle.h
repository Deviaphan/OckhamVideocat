#pragma once

#include "FileStore/femstore.h"
#include "FileStore/array_wrap.h"
#include "CStringStore.h"


template< class Store >
void Storage( Store & store, CPoint & pt, unsigned /*version*/ )
{
	store & pt.x;
	store & pt.y;
}


class ItemStyle
{
public:
	ItemStyle();

	int width; // ширина итема. -1 если на всю ширину
	int height; // высота итема
	int offset; // расстояние между итемами

	COLORREF borderColor; // цвет рамки
	int borderOpacity; // прозрачность рамки

	COLORREF shadowColor; // цвет тени
	int shadowOpacity; // прозрачность тени

	int posterHeight; // высота постера

	CString fontName; // шрифт, используемый для подписи фильма

	int sizeTitle1; // размер текста первой строки
	COLORREF colorTitle1;

	int sizeTitle2; // размер текста второй строки
	COLORREF colorTitle2;

	int sizeGenre; // Размер текста Жанров
	COLORREF colorGenre;

	int sizeTag;	// размер текста Тэгов
	COLORREF colorTag;

	int btnSize; // размер квадратных кнопок
	COLORREF btnColor; // цвет кнопки
	COLORREF btnColorHover; // цвет кнопки при наведении мыши

	enum
	{
		NumButtons = 6
	};
	CPoint button[NumButtons];

public:
	Gdiplus::Font * fontTitle1;
	Gdiplus::Font * fontTitle2;
	Gdiplus::Font * fontTags;
	Gdiplus::Font * fontGenres;

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & width; // ширина итема. -1 если на всю ширину
		ar & height; // высота итема
		ar & offset; // расстояние между итемами

		ar & borderColor; // цвет рамки
		ar & borderOpacity; // прозрачность рамки

		ar & shadowColor; // цвет тени
		ar & shadowOpacity; // прозрачность тени

		ar & posterHeight; // высота постера

		ar & fontName; // шрифт, используемый для подписи фильма

		ar & sizeTitle1; // размер текста первой строки
		ar & colorTitle1;

		ar & sizeTitle2; // размер текста второй строки
		ar & colorTitle2;

		ar & sizeGenre; // Размер текста Жанров
		ar & colorGenre;

		ar & sizeTag;	// размер текста Тэгов
		ar & colorTag;

		ar & btnSize; // размер квадратных кнопок
		ar & btnColor; // цвет кнопки
		ar & btnColorHover; // цвет кнопки при наведении мыши

		// чтобы корректно считывать старые файлы, где могло быть больше или меньше кнопок. При записи просто временная переменная - не страшно.
		int numButtons = NumButtons;
		ar & numButtons;
		FileStore::ArrayWrap arrayWrap( button, numButtons );
		ar & arrayWrap;
	}
};


class MenuStyle
{
public:
	MenuStyle();

	//CString fontName; // шрифт, используемый в меню
	int iconSize; // высота и ширина иконок в контекстных меню
	//int fontSize; // размер шрифта в меню
	COLORREF iconColor; // цвет иконок
	//COLORREF iconSelectedColor; // цвет иконок
	//COLORREF textColor; // цвет шрифта в меню
	//COLORREF textSelectedColor; // цвет шрифта в меню

public:
	//Gdiplus::Font * font;

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		//ar & fontName; // шрифт, исопльзуемый в меню
		ar & iconSize; // высота и ширина иконок в контекстных меню
		//ar & fontSize; // размер шрифта в меню
		ar & iconColor;
		//ar & iconSelectedColor;
		//ar & textColor;
		//ar & textSelectedColor;

	}
};

class TreeStyle
{
public:
	TreeStyle();

	CString fontName; // шрифт, используемый в меню
	int fontSize; // размер шрифта в меню
	COLORREF textColor; // цвет текста
	COLORREF textColorSelected; // цвет выбранного текста

public:
	Gdiplus::Font * font;

private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		ar & fontName; // шрифт, исопльзуемый в меню
		ar & fontSize; // высота и ширина иконок в контекстных меню
		ar & textColor; // размер шрифта в меню
		ar & textColorSelected;
	}
};

class ViewStyle
{

public:
	ViewStyle();
	~ViewStyle();

public:
	void SaveStyle( const CString & fileName );
	void LoadStyle( const CString & fileName );

	void Initialize( bool waitResult );
	void UpdateGlass( bool waitResult );
	void UpdateFonts();

	Gdiplus::Bitmap * GetGlassImage();

public:

	// стиль фона окна
	CString bgImageFile; // фоновая картинка
	COLORREF bgTopColor; // однородный фон, если нет картинки
	COLORREF bgBottomColor; // однородный фон, если нет картинки
	
	COLORREF glassColor; // цвет выбранного элемента
	int glassOpacity; // прозрачность фона выбранного элемента
	int glassBlur; // радиус размытия фоновой картинки

	   
	// отображение плиткой
	ItemStyle tileItem;
	ItemStyle tileItemSelected;

	// отображение списком
	ItemStyle listItem;
	ItemStyle listItemSelected;

	// маркер на объекте
	COLORREF markerColor; // цвет выделения просмотренного/непросмотренного фильма
	int markerOpacity;

	MenuStyle menu;

	TreeStyle tree;

public:

	Gdiplus::Bitmap * bgImage;
	Gdiplus::Bitmap * bgBlured;
	Gdiplus::Font * fontCommandbar;


private:
	friend class FileStore::Access;
	template<class Store>
	void Storage( Store & ar, unsigned /*version*/ )
	{
		//ar & bgImageFile; // фоновая картинка. Сохранять не нужно, т.к. имя всегда одно и то же
		ar & bgTopColor; // однородный фон, если нет картинки
		ar & bgBottomColor; // однородный фон, если нет картинки

		ar & glassColor; // цвет выбранного элемента
		ar & glassOpacity; // прозрачность фона выбранного элемента
		ar & glassBlur; // радиус размытия фоновой картинки

		ar & tileItem;
		ar & tileItemSelected;

		ar & listItem;
		ar & listItemSelected;

		ar & markerColor;
		ar & markerOpacity;

		ar & menu;

		ar & tree;
	}
};

void DefaultTheme( ViewStyle & theme );
void UpdateFonts( ItemStyle & item, bool createAdditional );
Gdiplus::Color MakeColor( COLORREF color, BYTE a = 255 );
