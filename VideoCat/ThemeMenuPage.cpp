// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeMenuPage.h"
#include "ViewStyle.h"

#include "resource.h"


IMPLEMENT_DYNAMIC(ThemeMenuPage, CDialog)

ThemeMenuPage::ThemeMenuPage(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THEME_MENU, pParent)
	, _sizeIcon( 32 )
	, _sizeText( 16 )
{

}

ThemeMenuPage::~ThemeMenuPage()
{
}

void ThemeMenuPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_MENU_GROUP, _menuGroup );

	DDX_Text( pDX, IDC_SIZE_ICON, _sizeIcon );
	DDV_MinMaxInt( pDX, _sizeIcon, 16, 128 );
	DDX_Text( pDX, IDC_SIZE_LINE_1, _sizeText );
	DDX_Control( pDX, IDC_FONT, _font );
	DDX_Control( pDX, IDC_BUTTON_COLOR, _iconColor );
	DDX_Control( pDX, IDC_BUTTON_HOVER_COLOR, _iconSelectedColor );
	DDX_Control( pDX, IDC_COLOR_LINE_1, _textColor );
	DDX_Control( pDX, IDC_SELECTED_COLOR_LINE_1, _textSelectedColor );
}


BEGIN_MESSAGE_MAP(ThemeMenuPage, CDialog)
	ON_BN_CLICKED( IDC_DELETE_FONT, &ThemeMenuPage::OnBnClickedDeleteFont )
END_MESSAGE_MAP()

BOOL ThemeMenuPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void ThemeMenuPage::OnBnClickedDeleteFont()
{
	_font.SetCurSel( -1 );
	UpdateData( FALSE );
}


void ThemeMenuPage::WriteData( const ViewStyle & style )
{
	//_font.SelectFont( style.menu.fontName );

	_iconColor.SetColor( style.menu.iconColor );
	//_iconSelectedColor.SetColor( style.menu.iconSelectedColor );

	//_textColor.SetColor( style.menu.textColor );
	//_textSelectedColor.SetColor( style.menu.textSelectedColor );

	_sizeIcon = style.menu.iconSize;
	//_sizeText = style.menu.fontSize;

	UpdateData( FALSE );
}

void ThemeMenuPage::ReadData( ViewStyle & style )
{
	UpdateData();

	//if( _font.GetSelFont() )
	//{
	//	style.menu.fontName = _font.GetSelFont()->m_strName;
	//}
	//else
	//{
	//	style.menu.fontName.Empty();
	//}

	style.menu.iconColor = _iconColor.GetColor();
	//style.menu.iconSelectedColor = _iconSelectedColor.GetColor();

	//style.menu.textColor = _textColor.GetColor();
	//style.menu.textSelectedColor = _textSelectedColor.GetColor();

	style.menu.iconSize = _sizeIcon;
	//style.menu.fontSize = _sizeText;
}

