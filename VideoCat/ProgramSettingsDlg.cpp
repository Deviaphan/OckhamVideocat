// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ProgramSettingsDlg.h"
#include "GlobalSettings.h"
#include "ResString.h"
#include "VideoCatView.h"
#include "VideoTreeView.h"
#include "AviableFiles.h"
#include "resource.h"

int CProgramSettings::refCount = 0;


IMPLEMENT_DYNAMIC(CProgramSettings, CDialog)

CProgramSettings::CProgramSettings(CVideoCatDoc * doc)
	: CDialog(IDD_SETTINGS, nullptr)
	, _doc(doc)
	, _currentPage(nullptr)
{
	++refCount;
}

CProgramSettings::~CProgramSettings()
{
	--refCount;
}

void CProgramSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SETTINGS_TAB, _tabCtrl);
}


BEGIN_MESSAGE_MAP(CProgramSettings, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SETTINGS_TAB, &CProgramSettings::OnTcnSelchangeSettingsTab)
END_MESSAGE_MAP()


BOOL CProgramSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	_tabCtrl.InsertItem( 0, ResString( IDS_SETTINGS_SHORTCUT ) );
	_tabCtrl.InsertItem( 1, ResString( IDS_SETTINGS_BEHAVIOR ) );
	_tabCtrl.InsertItem( 2, ResString( IDS_SETTINGS_STYLE ) );
	_tabCtrl.InsertItem( 3, ResString( IDS_SETTINGS_FORMATS ) );
	_tabCtrl.InsertItem( 4, ResString( IDS_SETTINGS_PROXY ) );

	CRect rc;
	_tabCtrl.GetWindowRect( &rc ); // получаем "рабочую область"
	_tabCtrl.ScreenToClient( &rc ); // преобразуем в относительные координаты
	_tabCtrl.AdjustRect( FALSE, &rc ); // исключаем область, где отображаются названия закладок:

	{
		shortcutDlg.Create( IDD_SHORTCUT, &_tabCtrl );
		shortcutDlg.MoveWindow( &rc, FALSE );
	}

	{
		behaviorDlg.alwaysShowRating = GetGlobal().alwaysShowRating;
		behaviorDlg.closeBeforePlay = GetGlobal().GetCloseBeforePlay();
		behaviorDlg.hideSecondTitle = GetGlobal().hideSecondTitle;
		behaviorDlg.marktype = GetGlobal().GetMarkType();
		behaviorDlg.tipOfDay = GetGlobal().ShowTipOfDay();
		behaviorDlg.numRandomFilms = GetGlobal().numRandomFilms;
		behaviorDlg.autoplayRandomFilm = GetGlobal().autoplayRandomFilm;
		behaviorDlg.Create( IDD_BEHAVIOR, &_tabCtrl );
		behaviorDlg.MoveWindow( &rc, FALSE );
	}

	{
		//viewStyleDlg.backgroundPath = GetGlobal().backgroundPath;
		viewStyleDlg._doc = _doc;
		viewStyleDlg.viewType = GetGlobal().GetViewType();
		viewStyleDlg.Create( IDD_VIEW_STYLE, &_tabCtrl );
		viewStyleDlg.MoveWindow( &rc, FALSE );
	}

	{
		fileFormatSettings.extensions = GetGlobal().aviabledFiles.extensions;

		fileFormatSettings.Create( IDD_FILE_FORMATS, &_tabCtrl );
		fileFormatSettings.MoveWindow( &rc, FALSE );
	}

	{
		proxySettings.proxyType = (int)GetGlobal().proxyType;
		proxySettings.proxyIP = GetGlobal().proxyIP;
		proxySettings.proxyPort = GetGlobal().proxyPort;
		proxySettings.timeout = GetGlobal().proxyTimeout;
		proxySettings.Create( IDD_PROXY_SETTINGS, &_tabCtrl );
		proxySettings.MoveWindow( &rc, FALSE );
	}

	_tabCtrl.SetCurSel( 1 );
	behaviorDlg.ShowWindow(SW_SHOW);
	_currentPage = &behaviorDlg;

	return TRUE;
}


void CProgramSettings::OnTcnSelchangeSettingsTab(NMHDR *, LRESULT *pResult)
{
	if( _currentPage )
		_currentPage->ShowWindow(SW_HIDE);

	int iTab = _tabCtrl.GetCurSel();

	switch (iTab)
	{
	case 0:
		_currentPage = &shortcutDlg;
		break;
	case 1:
		_currentPage = &behaviorDlg;
		break;

	case 2:
		_currentPage = &viewStyleDlg;
		break;

	case 3:
		_currentPage = &fileFormatSettings;
		break;
	case 4:
		_currentPage = &proxySettings;
		break;
	}

	_currentPage->ShowWindow( SW_SHOW );

	*pResult = 0;
}

void CProgramSettings::OnOK()
{
	{
		fileFormatSettings.SaveLastState();

		if( GetGlobal().aviabledFiles.extensions != fileFormatSettings.extensions )
		{
			GetGlobal().aviabledFiles.extensions.swap( fileFormatSettings.extensions );
			GetGlobal().SaveFileExtensions();
		}
	}

	GlobalSettings & global = GetGlobal();

	{
		behaviorDlg.UpdateData();

		global.SetHideSecondTitle( behaviorDlg.hideSecondTitle );
		global.SetCloseBeforePlay( behaviorDlg.closeBeforePlay != 0 );
		global.SetMarkType( behaviorDlg.marktype );
		global.SetShowStars( behaviorDlg.alwaysShowRating );
		global.SetShowTipOfDay( behaviorDlg.tipOfDay );
		global.SetNumRandomFilms( behaviorDlg.numRandomFilms, behaviorDlg.autoplayRandomFilm );
	}

	{
		viewStyleDlg.UpdateData();

		global.SetViewType( viewStyleDlg.viewType );
		global.SetViewScale( viewStyleDlg.viewScale );
	}

	{
		proxySettings.UpdateData();

		if( GetGlobal().proxyType != (ProxyType)proxySettings.proxyType )
		{
			if( (ProxyType)proxySettings.proxyType == ProxyType::UseProxy )
				GetGlobal().ReadProxyList();
		}

		GetGlobal().SetProxyType( proxySettings.proxyType );
		GetGlobal().SetProxyIP( proxySettings.proxyIP );
		GetGlobal().SetProxyPort( proxySettings.proxyPort );
		GetGlobal().SetTimeout( proxySettings.timeout );
	}

	if( _doc )
	{
		_doc->GetVideoCatView()->Invalidate();
		_doc->GetVideoTreeView()->Invalidate();
	}

	CDialog::OnOK();

	DestroyWindow();
}


BOOL CProgramSettings::DestroyWindow()
{
	BOOL result =  CDialog::DestroyWindow();

	delete this;

	return result;
}


void CProgramSettings::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}
