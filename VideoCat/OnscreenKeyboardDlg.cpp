// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "OnscreenKeyboardDlg.h"
#include <filesystem>
#include "GlobalSettings.h"
#include "resource.h"

constexpr wchar_t g_backspace = L'←';
constexpr wchar_t g_spacebar = L' ';

KeyLanguages OSKCtrl::_currentLanguage = KeyLanguages::Russian;

IMPLEMENT_DYNAMIC( OSKCtrl, CStatic )
BEGIN_MESSAGE_MAP( OSKCtrl, CStatic )
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC( OnscreenKeyboardDlg, CDialog )

OnscreenKeyboardDlg::OnscreenKeyboardDlg( CWnd * pParent /*=nullptr*/ )
	: CDialog( IDD_ONSCREEN_KEYBOARD, pParent )
{
	_osk.InitKeys();
}

OnscreenKeyboardDlg::~OnscreenKeyboardDlg()
{
}

void OnscreenKeyboardDlg::DoDataExchange( CDataExchange * pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_OSK_STATIC, _osk );
}


BEGIN_MESSAGE_MAP( OnscreenKeyboardDlg, CDialog )
END_MESSAGE_MAP()


BOOL OnscreenKeyboardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect desktopRect;
	CWnd::GetDesktopWindow()->GetWindowRect( desktopRect );

	_osk.Scale( desktopRect.Width(), desktopRect.Height() );

	SetWindowPos( nullptr, _osk.defaultRect.left, _osk.defaultRect.top, _osk.defaultRect.Width(), _osk.defaultRect.Height(), SWP_SHOWWINDOW );

	return TRUE;
}

CString OnscreenKeyboardDlg::GetText() const
{
	return _osk.textField;
}


void OSKCtrl::InitKeys()
{
	_hasLicense = false;
	std::error_code errorCode;
	if( std::filesystem::exists( std::filesystem::path( GetGlobal().GetKeyFileName().GetString() ), errorCode ) )
	{
		_hasLicense = true;
	}

	constexpr int LEFT_OFFSET = 10;
	constexpr int RIGHT_OFFSET = 10;
	constexpr int TOP_OFFSET = 5;
	constexpr int BOTTOM_OFFSET = 10;

	defaultRect.SetRect( 0, 0, btnSize, btnSize );

	int cx = LEFT_OFFSET;
	int cy = TOP_OFFSET + btnSize * 1.5;
	
	AddLine( cx, cy, L"ЙЦУКЕНГШЩЗХЪ", L"QWERTYUIOP  " );

	// бэкспейс расположен в ряду с цифрами, после них.
	KeyItem backspace;
	backspace.rect.SetRect(cx, cy, cx + btnSize * 2, cy + btnSize);
	backspace.key[0] = g_backspace;
	backspace.key[1] = g_backspace;
	_keysRect.push_back( backspace );
	defaultRect.UnionRect( &defaultRect, &backspace.rect);

	cx = LEFT_OFFSET + btnSize * 0.5;
	cy += btnSize;
	AddLine( cx, cy, L"ФЫВАПРОЛДЖЭ", L"ASDFGHJKL  " );
	cx = LEFT_OFFSET + btnSize;
	cy += btnSize;
	AddLine( cx, cy, L"ЯЧСМИТЬБЮ", L"ZXCVBNM<>" );

	cx = LEFT_OFFSET;
	cy += btnSize;
	AddLine( cx, cy, L"1234567890-#<>", L"1234567890-#<>" );

	cx = btnSize * 2 + LEFT_OFFSET;
	cy += btnSize;
	
	KeyItem spacebar;
	spacebar.rect.SetRect( cx, cy, cx + btnSize * 10, cy + btnSize );
	spacebar.key[0] = g_spacebar;
	spacebar.key[1] = g_spacebar;
	_keysRect.push_back( spacebar );
	defaultRect.UnionRect( &defaultRect, &spacebar.rect );

	_langRect.SetRect( LEFT_OFFSET, TOP_OFFSET, LEFT_OFFSET + btnSize * 1.25, TOP_OFFSET + btnSize * 1.25 );
	_textRect.SetRect( LEFT_OFFSET + btnSize * 1.25, TOP_OFFSET, defaultRect.right - btnSize * 2, TOP_OFFSET + btnSize * 1.5 );

	defaultRect.right += RIGHT_OFFSET;
	defaultRect.bottom += BOTTOM_OFFSET;

	_okRect.SetRect( defaultRect.right - btnSize * 1.25 - RIGHT_OFFSET, defaultRect.top + TOP_OFFSET, defaultRect.right - RIGHT_OFFSET, defaultRect.top + btnSize*1.25 + TOP_OFFSET );
}

void ScaleRect( CRect & r, const double & scaleFactor )
{
	r.left = r.left * scaleFactor + 0.5;
	r.right = r.right * scaleFactor + 0.5;
	r.top = r.top * scaleFactor + 0.5;
	r.bottom = r.bottom * scaleFactor + 0.5;
}

void OSKCtrl::Scale( int w, int h )
{
	const double scaleFactor = (double)w / (double)defaultRect.Width();

	defaultRect.right = defaultRect.right * scaleFactor + 0.5;
	defaultRect.bottom = defaultRect.bottom * scaleFactor + 0.5;

	for( auto & item : _keysRect )
	{
		ScaleRect( item.rect, scaleFactor );
	}

	ScaleRect( _okRect, scaleFactor );
	ScaleRect( _textRect, scaleFactor );
	ScaleRect( _langRect, scaleFactor );

	_roundRect = 4 * scaleFactor;

	CFont * font = CFont::FromHandle( (HFONT)::GetStockObject( DEFAULT_GUI_FONT ) );

	LOGFONT logFont;
	font->GetLogFont( &logFont );

	logFont.lfHeight = 14 * scaleFactor;
	logFont.lfWeight = 400;
	_buttonFont.CreateFontIndirect( &logFont );

	logFont.lfHeight = 28 * scaleFactor;
	logFont.lfWeight = 400;
	_fieldFont.CreateFontIndirect( &logFont );
}

void OSKCtrl::AddLine( int & cx, int cy, const wchar_t * ru, const wchar_t * en )
{
	while( *ru && *en )
	{
		KeyItem item;

		item.rect.SetRect( cx, cy, cx + btnSize, cy + btnSize );

		defaultRect.UnionRect( &defaultRect, &item.rect );

		cx += btnSize;

		item.key[0] = *ru;
		item.key[1] = *en;

		_keysRect.push_back( item );

		++ru;
		++en;
	};
}

void OSKCtrl::DrawKeyboard( CDC * dc )
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	COLORREF bgColor = GetGlobal().theme->bgBottomColor;
	COLORREF textColor = GetGlobal().theme->tree.textColor;

	dc->FillSolidRect( defaultRect, bgColor );

	dc->SetBkMode( TRANSPARENT );
	dc->SetTextColor( textColor );

	CBrush * originalBrush = nullptr;
	CPen * originalPen = nullptr;
	CFont * originalFont = nullptr;

	CPen pen( PS_SOLID, 1, textColor );
	originalPen = dc->SelectObject( &pen );

	CBrush brush( bgColor );
	CBrush hoverBtn( RGB( 64, 65, 230 ) );

	if( !textField.IsEmpty() )
	{
		originalFont = dc->SelectObject( &_fieldFont );
		dc->DrawText( textField, textField.GetLength(), _textRect, DT_VCENTER | DT_CENTER | DT_SINGLELINE );
		dc->SelectObject( originalFont );
	}

	if( _langRect.PtInRect( point ) )
	{
		originalBrush = dc->SelectObject( &hoverBtn );
	}
	else
	{
		originalBrush = dc->SelectObject( &brush );
	}
	dc->RoundRect( _langRect.left, _langRect.top, _langRect.right, _langRect.bottom, _roundRect, _roundRect );
	dc->SelectObject( originalBrush );

	originalFont = dc->SelectObject( &_buttonFont );
	dc->DrawText( L"Ru/En", -1, _langRect, DT_VCENTER | DT_CENTER | DT_SINGLELINE );
	dc->SelectObject( originalBrush );

	if( _okRect.PtInRect( point ) )
	{
		originalBrush = dc->SelectObject( &hoverBtn );
	}
	else
	{
		originalBrush = dc->SelectObject( &brush );
	}
	dc->RoundRect( _okRect.left, _okRect.top, _okRect.right, _okRect.bottom, _roundRect, _roundRect );
	dc->SelectObject( originalBrush );

	originalFont = dc->SelectObject( &_buttonFont );
	if( !_hasLicense )
	{
		dc->SetTextColor( RGB( 180, 80, 80 ) );
	}
	dc->DrawText( L"OK", 3, _okRect, DT_VCENTER | DT_CENTER | DT_SINGLELINE );
	dc->SelectObject( originalBrush );

	dc->SetTextColor( textColor );

	for( const auto & item : _keysRect )
	{
		CRect r( item.rect );
		r.DeflateRect( 2, 2 );

		if( r.PtInRect( point ) )
		{
			originalBrush = dc->SelectObject( &hoverBtn );
		}
		else
		{
			originalBrush = dc->SelectObject( &brush );
		}
		dc->RoundRect( r.left, r.top, r.right, r.bottom, _roundRect, _roundRect );
		dc->SelectObject( originalBrush );

		originalFont = dc->SelectObject( &_buttonFont );
		dc->DrawText( &item.key[(int)_currentLanguage], 1, r, DT_VCENTER | DT_CENTER | DT_SINGLELINE );
		dc->SelectObject( originalBrush );
	}

	dc->SelectObject( originalFont );
}

void OSKCtrl::OnPaint()
{
	CPaintDC dc( this );

	CRect rect;
	GetClientRect( rect );

	if( _memBuffer.m_hObject == nullptr )
	{
		_memBuffer.CreateCompatibleBitmap( &dc, rect.Width(), rect.Height() );
		_dc.CreateCompatibleDC( &dc );
		_dc.SelectObject( &_memBuffer );
	}
	   

	DrawKeyboard( &_dc );

	dc.BitBlt( 0, 0, defaultRect.Width(), defaultRect.Height(), &_dc, 0, 0, SRCCOPY );
}


void OSKCtrl::OnSize( UINT nType, int cx, int cy )
{
	CStatic::OnSize( nType, cx, cy );

	if( !IsWindow( GetSafeHwnd() ) )
		return;

	_dc.DeleteDC();
	_memBuffer.DeleteObject();
}


void OSKCtrl::OnMouseMove( UINT nFlags, CPoint point )
{
	Invalidate(FALSE);
}


void OSKCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{
	for( const auto & item : _keysRect )
	{
		CRect r( item.rect );
		r.DeflateRect( 2, 2 );

		if( !r.PtInRect( point ) )
			continue;

		if( item.key[0] == g_backspace )
		{
			if( !textField.IsEmpty() )
				textField.Truncate( textField.GetLength() - 1 );
		}
		else
		{
			textField += item.key[(int)_currentLanguage];
		}
	}

	Invalidate(FALSE);
}


void OSKCtrl::OnLButtonUp( UINT nFlags, CPoint point )
{
	if( _okRect.PtInRect( point ) )
	{
		CDialog * dlg = dynamic_cast<CDialog *>(GetParent());
		if( dlg )
		{
			dlg->EndDialog( IDOK );
		}
	}
	else if( _langRect.PtInRect( point ) )
	{
		switch( _currentLanguage )
		{
			case KeyLanguages::Russian:
				_currentLanguage = KeyLanguages::English;
				break;
			case KeyLanguages::English:
			default:
				_currentLanguage = KeyLanguages::Russian;
				break;
		}

		Invalidate( FALSE );
	}
}


void OSKCtrl::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	OnLButtonDown( nFlags, point );
}
