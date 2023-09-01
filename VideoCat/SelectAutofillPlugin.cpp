// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "SelectAutofillPlugin.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CSelectAutofillPlugin, CDialog)

CSelectAutofillPlugin::CSelectAutofillPlugin(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SELECT_AUTOFILL_PLUGIN, pParent)
	, _choise( 1 )
{

}

CSelectAutofillPlugin::~CSelectAutofillPlugin()
{
}

void CSelectAutofillPlugin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio( pDX, IDC_RADIO_MANUAL, _choise );
	DDX_Control( pDX, IDC_PLUGIN, _pluginList );
}


BEGIN_MESSAGE_MAP(CSelectAutofillPlugin, CDialog)
END_MESSAGE_MAP()

PluginID CSelectAutofillPlugin::GetPluginID()
{
	switch( _choise )
	{
		case 0:
			return (PluginID)PluginManual;

		default:
			return (PluginID)PluginKinopoisk;
	}
}


void CSelectAutofillPlugin::OnOK()
{
	UpdateData();

	CDialog::OnOK();
}


BOOL CSelectAutofillPlugin::OnInitDialog()
{
	CDialog::OnInitDialog();

	_pluginList.AddString( L"Кинопоиск" );
	_pluginList.SetCurSel( 0 );

	return TRUE;
}
