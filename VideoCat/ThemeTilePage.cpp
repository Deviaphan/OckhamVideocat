// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeTilePage.h"
#include "ViewStyle.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(ThemeTilePage, CDialog)

ThemeTilePage::ThemeTilePage(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THEME_TILE, pParent)
	, _sizeLine1( 0 )
	, _sizeLine2( 0 )
{

}

ThemeTilePage::~ThemeTilePage()
{
}

void ThemeTilePage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TILE_GROUP, _tileGroup );
	DDX_Control( pDX, IDC_BUTTON_COLOR, _btnSelectedColor );
	DDX_Control( pDX, IDC_BUTTON_HOVER_COLOR, _btnSelectedHoverColor );
	DDX_Control( pDX, IDC_FONT, _fontName );
	DDX_Control( pDX, IDC_COLOR_LINE_1, _colorLine1 );
	DDX_Control( pDX, IDC_SELECTED_COLOR_LINE_1, _colorSelectedLine1 );
	DDX_Text( pDX, IDC_SIZE_LINE_1, _sizeLine1 );
	DDX_Text( pDX, IDC_SIZE_LINE_2, _sizeLine2 );
	DDX_Control( pDX, IDC_COLOR_LINE_2, _colorLine2 );
	DDX_Control( pDX, IDC_SELECTED_COLOR_LINE_2, _colorSelectedLine2 );
}


BEGIN_MESSAGE_MAP(ThemeTilePage, CDialog)
	ON_BN_CLICKED( IDC_DELETE_FONT, &ThemeTilePage::OnBnClickedDeleteFont )
END_MESSAGE_MAP()

void ThemeTilePage::WriteData( const ViewStyle & style )
{
	_fontName.SelectFont( style.tileItem.fontName);

	_btnSelectedColor.SetColor( style.tileItemSelected.btnColor );
	_btnSelectedHoverColor.SetColor( style.tileItemSelected.btnColorHover );

	_colorLine1.SetColor( style.tileItem.colorTitle1 );
	_colorLine2.SetColor( style.tileItem.colorTitle2 );
	_colorSelectedLine1.SetColor( style.tileItemSelected.colorTitle1 );
	_colorSelectedLine2.SetColor( style.tileItemSelected.colorTitle2 );
	_sizeLine1 = style.tileItem.sizeTitle1;
	_sizeLine2 = style.tileItem.sizeTitle2;

	UpdateData( FALSE );
}

void ThemeTilePage::ReadData( ViewStyle & style )
{
	UpdateData();

	if( _fontName.GetSelFont() )
	{
		style.tileItem.fontName = _fontName.GetSelFont()->m_strName;
		style.tileItemSelected.fontName = _fontName.GetSelFont()->m_strName;
	}
	else
	{
		style.tileItem.fontName.Empty();
		style.tileItemSelected.fontName.Empty();
	}

	style.tileItem.btnColor = _btnSelectedColor.GetColor();
	style.tileItem.btnColorHover = _btnSelectedHoverColor.GetColor();
	style.tileItemSelected.btnColor = _btnSelectedColor.GetColor();
	style.tileItemSelected.btnColorHover = _btnSelectedHoverColor.GetColor();

	style.tileItem.colorTitle1 = _colorLine1.GetColor();
	style.tileItem.colorTitle2 = _colorLine2.GetColor();
	style.tileItem.sizeTitle1 = _sizeLine1;
	style.tileItem.sizeTitle2 = _sizeLine2;

	style.tileItemSelected.colorTitle1 = _colorSelectedLine1.GetColor();
	style.tileItemSelected.colorTitle2 = _colorSelectedLine2.GetColor();
	style.tileItemSelected.sizeTitle1 = _sizeLine1;
	style.tileItemSelected.sizeTitle2 = _sizeLine2;

}


void ThemeTilePage::OnBnClickedDeleteFont()
{
	_fontName.SetCurSel( -1 );
	UpdateData( FALSE );
}
