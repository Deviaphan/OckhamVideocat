#pragma once


class CListBoxCH: public CListBox
{
	//DECLARE_DYNAMIC( CListBoxCH );

public:
	CListBoxCH();

	void SetIcon( HICON icon )
	{
		_icon = icon;
	}

public:
	void DrawItem( LPDRAWITEMSTRUCT lpDIS ) override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu( CWnd * /*pWnd*/, CPoint /*point*/ );

protected:
	HICON _icon;
};
