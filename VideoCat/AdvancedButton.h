#pragma once
#include <afxbutton.h>
#include <afxmenubutton.h>

class AdvancedButton: public CMFCButton
{
	DECLARE_DYNAMIC( AdvancedButton )
public:
	AdvancedButton();
	~AdvancedButton() override;

public:
	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct ) override;

protected:
	DECLARE_MESSAGE_MAP()

public:
	int _iconID;
};

class AdvancedMenuButton: public CMFCMenuButton
{
	DECLARE_DYNAMIC( AdvancedMenuButton )
public:
	AdvancedMenuButton();
	~AdvancedMenuButton() override;

public:
	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct ) override;

protected:
	DECLARE_MESSAGE_MAP()

public:
	int _iconID;
};