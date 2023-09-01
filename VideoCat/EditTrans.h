#pragma once


#define TRANS_BACK -1

class CEditTrans : public CEdit
{
public:
	CEditTrans();

public:
	virtual ~CEditTrans();

public:
	void SetTextColor( COLORREF col )
	{
		m_TextColor = col;
		UpdateCtrl();
	}

	void SetBackColor( COLORREF col );

private:
    void UpdateCtrl();
	
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnUpdate();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillfocus();

private:
	CBrush   m_Brush;
	COLORREF m_TextColor;
	COLORREF m_BackColor;
};


