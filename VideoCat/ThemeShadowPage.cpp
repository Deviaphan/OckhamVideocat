// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThemeShadowPage.h"
#include "ViewStyle.h"
#include "resource.h"


IMPLEMENT_DYNAMIC(ThemeShadowPage, CDialog)

ThemeShadowPage::ThemeShadowPage(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_THEME_SHADOW, pParent)
	, _shadowOpacity( 0 )
	, _markerOpacity( 0 )
{

}

ThemeShadowPage::~ThemeShadowPage()
{
}

void ThemeShadowPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_SHADOW_1, _shadowGroup1 );
	DDX_Control( pDX, IDC_SHADOW_COLOR, _shadowColor );
	DDX_Control( pDX, IDC_MARKER_COLOR, _markerColor );
	DDX_Text( pDX, IDC_SHADOW_OPACITY, _shadowOpacity );
	DDV_MinMaxInt( pDX, _shadowOpacity, 0, 255 );
	DDX_Text( pDX, IDC_MARKER_OPACITY, _markerOpacity );
	DDV_MinMaxInt( pDX, _markerOpacity, 0, 255 );
	DDX_Control( pDX, IDC_SPIN1, _shadowSpiner );
	DDX_Control( pDX, IDC_SPIN4, _markerSpiner );
}


BEGIN_MESSAGE_MAP(ThemeShadowPage, CDialog)
END_MESSAGE_MAP()

BOOL ThemeShadowPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	_shadowSpiner.SetRange( 0, 255 );
	_markerSpiner.SetRange( 0, 255 );

	return TRUE;
}

void ThemeShadowPage::WriteData( const ViewStyle & style )
{
	_shadowColor.SetColor( style.tileItemSelected.shadowColor );
	_shadowOpacity = style.tileItemSelected.shadowOpacity;

	_markerColor.SetColor( style.markerColor );
	_markerOpacity = style.markerOpacity;

	UpdateData( FALSE );
}

void ThemeShadowPage::ReadData( ViewStyle & style )
{
	UpdateData();

	style.tileItemSelected.shadowColor = _shadowColor.GetColor();
	style.tileItemSelected.shadowOpacity = _shadowOpacity;

	style.listItemSelected.shadowColor = _shadowColor.GetColor();
	style.listItemSelected.shadowOpacity = _shadowOpacity;

	{
		// сейчас эти данные не используются. На будущее заполняю их тоже, для будущей совместимости тем
		style.tileItem.shadowColor = _shadowColor.GetColor();
		style.tileItem.shadowOpacity = 0;// _shadowOpacity;


		style.listItem.shadowColor = _shadowColor.GetColor();
		style.listItem.shadowOpacity = 0;// _shadowOpacity;
	}

	style.markerColor = _markerColor.GetColor();
	style.markerOpacity = _markerOpacity;
}

