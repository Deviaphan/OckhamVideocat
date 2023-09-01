#pragma once

class CCommentBox : public CEdit
{
	DECLARE_DYNAMIC(CCommentBox)

public:
	CCommentBox();
	~CCommentBox() override;

	/// Задать текст
	void SetText( const CString & text );
	/// Задать цвет шрифта
	void SetFontColor( COLORREF rgb );
	/// Задать цвет фона
	void SetBackColor( COLORREF rgb );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

private:
	CBrush _brush;
	COLORREF _front;
	COLORREF _back;
};


