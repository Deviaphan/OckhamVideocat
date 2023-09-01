#pragma once

#include "CardTabCtrl.h"
#include "Entry.h"


class CVideoCatDoc;

class CEditCard : public CDialogEx
{
	DECLARE_DYNAMIC( CEditCard )

public:
	CEditCard( CWnd* pParent = nullptr );
	virtual ~CEditCard();

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_EDIT_CARD
	};
#endif

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

public:
	void SetDocument( CVideoCatDoc * doc )
	{
		_doc = doc;
	}

	void SetEntry( Entry * entry )
	{
		_entry = entry;
		_editEntry = *_entry;
	}

	Entry * GetEntry()
	{
		return _entry;
	}

	void SetDefaultTab( CardTabName::Enum index )
	{
		_defaultTab = index;
	}

protected:
	virtual void DoDataExchange( CDataExchange* pDX );

	DECLARE_MESSAGE_MAP()

public:
	static BOOL _autoSave;

protected:
	CVideoCatDoc * _doc;
	Entry * _entry;
	Entry _editEntry;

	CCardTabCtrl _tab;
	CardTabName::Enum _defaultTab;

	bool _objectChanged;
};
