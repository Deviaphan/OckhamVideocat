// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "CardTabCtrl.h"
#include "VideoCatDoc.h"
#include "CollectionDB.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CCardTabCtrl, CMFCTabCtrl)

CCardTabCtrl::CCardTabCtrl()
{

}

CCardTabCtrl::~CCardTabCtrl()
{
}


BEGIN_MESSAGE_MAP(CCardTabCtrl, CMFCTabCtrl)
END_MESSAGE_MAP()

void CCardTabCtrl::Init( CVideoCatDoc * doc, Entry * entry, CardTabName::Enum activeTab )
{
	EnableTabSwap( FALSE );
	m_nTabsHeight = 20;

	CRect rc;
	GetWindowRect( &rc );
	ScreenToClient( &rc );

	int index = 0;

	{
		_pageAutoFill.doc = doc;
		_pageAutoFill.entry = entry;

		_pageAutoFill.Create( IDD_TAB_AUTOFILL, this );
		_pageAutoFill.MoveWindow( &rc );
		_pageAutoFill.ShowWindow( SW_SHOW );
		InsertTab( &_pageAutoFill, L"PlugIn", index++ );
	}

	{
		_pageFilmInfo.doc = doc;
		_pageFilmInfo.entry = entry;

		_pageFilmInfo.Create( IDD_TAB_FILM_INFO, this );
		_pageFilmInfo.MoveWindow( &rc );
		_pageFilmInfo.ShowWindow( SW_HIDE );
		InsertTab( &_pageFilmInfo, L"Фильм", index++ );
	}

	{		
		_pageDescription.doc = doc;
		_pageDescription.entry = entry;

		_pageDescription.Create( IDD_TAB_DESCRIPTION, this );
		_pageDescription.MoveWindow( &rc );
		_pageDescription.ShowWindow( SW_HIDE );
		InsertTab( &_pageDescription, L"Описание", index++ );
	}

	{
		_pagePoster.doc = doc;
		_pagePoster.entry = entry;

		_pagePoster.Create( IDD_TAB_POSTER, this );
		_pagePoster.MoveWindow( &rc );
		_pagePoster.ShowWindow( SW_HIDE );
		InsertTab( &_pagePoster, L"Постер", index++ );
	}

	// тех.инфо только для реальных файлов
	if( entry->IsFile() )
	{
		_pageTechInfo.doc = doc;
		_pageTechInfo.entry = entry;

		_pageTechInfo.Create( IDD_TAB_TECH_INFO, this );
		_pageTechInfo.MoveWindow( &rc );
		_pageTechInfo.ShowWindow( SW_HIDE );
		InsertTab( &_pageTechInfo, L"Тех.Инфо", index++ );
	}

	switch( activeTab )
	{
		case CardTabName::AutoFill:
			SetActiveTab( 0 );	break;
		case CardTabName::FilmInfo:
			SetActiveTab( 1 );	break;
		case CardTabName::FilmDescription:
		case CardTabName::TagInfo:
			SetActiveTab( 2 );	break;
		case CardTabName::FilmPoster:
			SetActiveTab( 3 );	break;			
		case CardTabName::TechInfo:
			SetActiveTab( 4 );	break;

		default:
			SetActiveTab( 1 );
			break;
	}
}

void CCardTabCtrl::SaveData()
{
	_pageAutoFill.SaveData();
	_pageFilmInfo.SaveData();
	_pageDescription.SaveData();
	_pagePoster.SaveData();

	if( _pageTechInfo.entry )
	{
		_pageTechInfo.SaveData();
	}
}

void CCardTabCtrl::FireChangeActiveTab( int nNewTab )
{
	__super::FireChangeActiveTab( nNewTab );
	ICardPageBase * page = dynamic_cast<ICardPageBase*>(GetTabWnd( nNewTab ));
	if( page )
	{
		page->ReinitTab();
	}
}

void CCardTabCtrl::ReloadCard()
{
	_pageFilmInfo.ReinitTab();
	_pageDescription.ReinitTab();
	_pagePoster.LoadPosterById();
}
