#pragma once
#include <afxwin.h>

class ComboBoxAdv :	public CComboBox
{
public:
	ComboBoxAdv();
	virtual ~ComboBoxAdv();

public:
	void SetTextColor( COLORREF col )
	{
		m_TextColor = col;
		UpdateCtrl();
	}

	void SetBackColor( COLORREF col );


	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor( CDC * pDC, CWnd * pWnd, UINT nCtlColor );

private:
	void UpdateCtrl();

protected:
	CBrush   m_Brush;
	COLORREF m_TextColor;
	COLORREF m_BackColor;
public:
	afx_msg HBRUSH CtlColor( CDC * /*pDC*/, UINT /*nCtlColor*/ );
};

