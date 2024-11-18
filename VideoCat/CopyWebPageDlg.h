#pragma once

#include <dcomp.h>
#include "EdgeBrowser/ViewComponent.h"
#include <vector>
#include <memory>

class CopyWebPageDlg: public CDialog
{
	DECLARE_DYNAMIC( CopyWebPageDlg )

public:
	CopyWebPageDlg( CWnd* pParent = nullptr );
	virtual ~CopyWebPageDlg();

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_COPY_PAGE
	};
#endif

public:
	ICoreWebView2Controller* GetWebViewController()
	{
		return m_controller.get();
	}
	ICoreWebView2* GetWebView()
	{
		return m_webView.get();
	}
	ICoreWebView2Environment* GetWebViewEnvironment()
	{
		return m_webViewEnvironment.get();
	}
	HWND GetMainWindow()
	{
		return this->GetSafeHwnd();
	}

protected:
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );
	afx_msg void OnBnClickedCopy();
	afx_msg void OnSize( UINT nType, int cx, int cy );

	HRESULT OnCreateEnvironmentCompleted( HRESULT result, ICoreWebView2Environment* environment );
	HRESULT OnCreateCoreWebView2ControllerCompleted( HRESULT result, ICoreWebView2Controller* controller );
	HRESULT WebMessageReceived( ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args );
	//HRESULT NavigationCompletedEventHandler( ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args );
	HRESULT ExecuteScriptComplete( HRESULT error, ICoreWebView2ExecuteScriptResult* result );
	HRESULT ExScriptComplete( HRESULT errorCode, LPCWSTR result );

	void InitializeWebView();
	void CloseWebView();
	void ResizeEverything();

protected:
	int _minX;
	int _minY;

public:
	CString startPageUrl;
	CString pageData; // utf-8
	CStringA pageCharset;


	HWND m_mainWindow = nullptr;
	HINSTANCE g_hInstance = nullptr;
	DWORD m_creationModeId = 0;

	wil::com_ptr<ICoreWebView2Environment> m_webViewEnvironment;
	wil::com_ptr<ICoreWebView2Controller> m_controller;
	wil::com_ptr<ICoreWebView2> m_webView;
	wil::com_ptr<IDCompositionDevice> m_dcompDevice;
	std::vector<std::unique_ptr<ComponentBase>> m_components;

	static constexpr size_t s_maxLoadString = 100;

	template <class ComponentType, class... Args>
	void NewComponent( Args&&... args );

	template <class ComponentType>
	ComponentType* GetComponent();
};

template <class ComponentType, class... Args>
void CopyWebPageDlg::NewComponent( Args&&... args )
{
	m_components.emplace_back( new ComponentType( std::forward<Args>( args )... ) );
}

template <class ComponentType>
ComponentType* CopyWebPageDlg::GetComponent()
{
	for( auto& component : m_components )
	{
		if( auto wanted = dynamic_cast<ComponentType*>(component.get()) )
		{
			return wanted;
		}
	}
	return nullptr;
}

