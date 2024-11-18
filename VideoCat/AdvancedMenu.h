#pragma once

#include <vector>
#include "Commands/CommandID.h"
#include "IconManager.h"

namespace adv_mfc
{

class AdvancedMenu;

/** Базовый класс для всех элементов меню
*/
class IAdvancedMenuItem
{
public:
	/// Нарисовать пункт меню
	void DrawItem( DRAWITEMSTRUCT * dis );
	/// Определить размеры пункта меню
	void MeasureItem( MEASUREITEMSTRUCT * mis );

protected:
	/// Конструктор
	explicit IAdvancedMenuItem();
private:
	IAdvancedMenuItem( const IAdvancedMenuItem & ) = delete;
	const IAdvancedMenuItem & operator = ( const IAdvancedMenuItem & ) = delete;
protected:
	/// Деструктор
	virtual ~IAdvancedMenuItem();

protected:
	/// Нарисовать объект
	virtual void ItemDrawEntire( DRAWITEMSTRUCT * dis ) = 0;
	/// Нарисовать объект при выделении (подсветке)
	virtual void ItemDrawSelected( DRAWITEMSTRUCT * dis ) = 0;
	/// Нарисовать объект после снятия выделения. По умолчанию, вызывает ItemDrawEntire
	virtual void ItemDrawDeselected( DRAWITEMSTRUCT * dis );
	/// Определить размер пункта меню
	virtual void ItemMeasure(  MEASUREITEMSTRUCT * mis ) = 0;

private:
	friend class AdvancedMenu;

protected:
	UINT _state;
	UINT _size;
private:
	AdvancedMenu * _popup;	/// Для многоуровневых меню. Описывает вложенное меню
};


/** Текстовый пункт меню

Текстовый пункт меню содержит текст с картинкой или разделитель
*/
class AdvancedTextItem : public IAdvancedMenuItem
{
public:
	explicit AdvancedTextItem( const CString & text = CString(), IconID iconID = 0 );

private:
	AdvancedTextItem( const AdvancedTextItem & ) = delete;
	const AdvancedTextItem & operator = ( const AdvancedTextItem & ) = delete;
public:
	~AdvancedTextItem() override;

public:
	void ItemDrawEntire( DRAWITEMSTRUCT * dis ) override;
	void ItemDrawSelected( DRAWITEMSTRUCT * dis ) override;

	void ItemMeasure(  MEASUREITEMSTRUCT * mis ) override;

private:
	CString _text;
	IconID _iconID;
};

/** Расширенный класс контекстного меню

Сделан на основе класа CMenu, однако, в целях безопасности использования, интерфейс класса CMenu закрыт.<BR>
Есть возможность загрузки меню из ресурсов или импорта из объекта CMenu.<BR>
Поддерживаются многоуровневые меню.
*/
class AdvancedMenu : public CMenu
{
public:
	explicit AdvancedMenu(int iconSize = 0);
	~AdvancedMenu() override;

	void AddItem( UINT_PTR id, IAdvancedMenuItem * item, UINT flags = MF_ENABLED );
	void AddItem( AdvancedMenu * popup, IAdvancedMenuItem * item, UINT flags = MF_POPUP );
	void AddItem( CommandID id );

	void AddSeparator( IAdvancedMenuItem * item = new AdvancedTextItem );

	BOOL ImportMenu( CMenu & menu, BOOL importOwnerDraw = TRUE );

	BOOL LoadMenu(LPCTSTR lpszResourceName, BOOL importOwnerDraw = TRUE);
	BOOL LoadMenu(UINT nIDResource, BOOL importOwnerDraw = TRUE);
	
	int GetIconSize() const
	{
		return _iconSize;
	}

private:
	using CMenu::Attach;
	using CMenu::Detach;
	using CMenu::AppendMenu;
	using CMenu::GetMenuString;
	using CMenu::SetMenuItemBitmaps;

protected:
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) override;

private:
	using ItemList = std::vector<IAdvancedMenuItem *>;
	ItemList _items;
	int _iconSize;
};

}//	adv_mfc
