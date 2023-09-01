#pragma once

#include "CardAutoFill.h"
#include "CardFilmInfo.h"
#include "CardDescription.h"
#include "CardPoster.h"
#include "CardTechInfo.h"

class CVideoCatDoc;
class Entry;

// номера табов
namespace CardTabName
{
	enum Enum
	{
		AutoFill = 0,	// Автозаполнение карточки фильма
		FilmInfo, // Карточка фильма
		FilmPoster,	// Выбор постера
		FilmDescription,	// Описание и жанры фильма
		TagInfo,	// Метки и Комментарий к фильму
		TechInfo,	/// Техническая информация о файле

	};
}

class CCardTabCtrl : public CMFCTabCtrl
{
	DECLARE_DYNAMIC(CCardTabCtrl)

public:
	CCardTabCtrl();
	virtual ~CCardTabCtrl();

	void Init( CVideoCatDoc * doc, Entry * entry, CardTabName::Enum activeTab );
	void SaveData();

	virtual void FireChangeActiveTab( int nNewTab ) override;

	void ReloadCard();

protected:
	DECLARE_MESSAGE_MAP()

private:
	CCardAutoFill _pageAutoFill;
	CCardFilmInfo _pageFilmInfo;
	CCardDescription _pageDescription;
	CCardPoster _pagePoster;
	CCardTechInfo _pageTechInfo;
};


