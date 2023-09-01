#pragma once

#include <vector>

enum class KeyLanguages: int
{
	Russian,
	English,

	Size
};

class KeyItem
{
public:

	CRect rect;
	wchar_t key[(int)KeyLanguages::Size];
};

class OSKCtrl: public CStatic
{
public:
	DECLARE_DYNAMIC( OSKCtrl );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );

public:
	void InitKeys();
	void Scale( int w, int h );

private:
	void AddLine( int & cx, int cy, const wchar_t * ru, const wchar_t * en );
	void DrawKeyboard( CDC * dc );

private:
	CBitmap _memBuffer;
	CDC _dc;

	CRect _textRect;
	CRect _okRect;
	CRect _langRect;

	std::vector<KeyItem> _keysRect;

	static KeyLanguages _currentLanguage;

	const int btnSize = 32; // размер кнопок по умолчанию. Будет масштабироваться до всей ширины экрана
	int _roundRect;

	CFont _buttonFont;
	CFont _fieldFont;

	bool _hasLicense;

public:
	CString textField;

	CRect defaultRect;
};

class OnscreenKeyboardDlg: public CDialog
{
	DECLARE_DYNAMIC( OnscreenKeyboardDlg )

public:
	explicit OnscreenKeyboardDlg( CWnd * pParent = nullptr );
	~OnscreenKeyboardDlg() override;
	BOOL OnInitDialog() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ONSCREEN_KEYBOARD };
#endif

	CString GetText() const;

protected:
	void DoDataExchange( CDataExchange * pDX ) override;

	DECLARE_MESSAGE_MAP()

private:
	OSKCtrl _osk;
};
