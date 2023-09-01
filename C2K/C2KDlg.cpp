#include "stdafx.h"
#include "C2KDlg.h"

#include <fstream>
#include <strstream>
#include <algorithm>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "FemStore/femstore.h"
#include "FemStore/std_map.h"
#include "FemStore/std_vector.h"
#include "ForEachPredicates.h"
#include "HashItem.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define ENABLE_C2K_BUILD 1

CC2KDlg::CC2KDlg( CWnd* pParent /*=NULL*/ )
	: CDialog( IDD_C2K_DIALOG, pParent )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void CC2KDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESS, _progress );
}

BEGIN_MESSAGE_MAP( CC2KDlg, CDialog )
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

struct Convert : public ns_foreach::ForEachFunctor
{
	CC2KDlg * dlg;
	const std::map<int, int> * c2k;
	const std::vector<KinopoiskInfo> * kinopoiskBase;

	ns_foreach::Result Run( FileInfo & info ) override
	{
		auto ii = c2k->find( info.filmID );
		if( ii == c2k->end() )
		{
			info.filmID = NO_KINOPOISK_ID;
		}
		else
		{
			const KinopoiskInfo * ki = GetInfo( ii->second );
			if( ki )
			{
				(KinopoiskInfo&)info = *ki;
			}
			else
				info.filmID = ii->second;
		}

		dlg->Step();

		return ns_foreach::CONTINUE;
	}

	const KinopoiskInfo * GetInfo( KinopoiskId id ) const
	{
		KinopoiskInfo ki;
		ki.filmID = id;

		auto ii = std::lower_bound( kinopoiskBase->begin(), kinopoiskBase->end(), ki, KinopoiskID() );
		if( ii != kinopoiskBase->end() )
		{
			if( ii->filmID == id )
				return &(*ii);
		}

		return nullptr;
	}
};


void __cdecl ProcessInThread( void *args )
{
	CC2KDlg * dlg = (CC2KDlg*)args;

	try
	{
		dlg->LoadKinopoisk();

#if ENABLE_C2K_BUILD
		dlg->LoadCinemate();
#else
		dlg->LoadC2K();
#endif

#if ENABLE_C2K_BUILD

		dlg->SetStepCounts( (int)_cinemateBase.size() );

		for( const CinemateInfo & ci : dlg->_cinemateBase )
		{
			dlg->Step();

			KinopoiskInfo ki;
			ki.nameEN = ci.titleOriginal;
			ki.nameRU = ci.titleRussian;
			if( ki.nameEN.empty() )
				ki.nameEN = ki.nameRU;

			auto ii = std::lower_bound( dlg->_kinopoiskBase.begin(), dlg->_kinopoiskBase.end(), ki, KinopoiskEnRu() );

			// фильм отсутствует в базе кинопоиска
			if( ii == dlg->_kinopoiskBase.end() )
				continue;
			if( ii->nameEN != ki.nameEN )
				continue;

			dlg->_c2k[ci.id] = ii->filmID;
		}

		std::ofstream out( "c2k.bin", std::ios_base::binary );
		FemStore::Writer w( out );
		w & dlg->_c2k;

#else

		{
			std::ifstream inFile( dlg->userLibrary, std::ios_base::binary );
			if( !inFile )
			{
				throw std::exception();
			}

			boost::archive::binary_iarchive in( inFile );

			UINT documentVersion = 0;
			in >> documentVersion;

			in >> dlg->_fileTree;

			PosterManager & pm = GetPosterManager();
			in >> pm;
		}

		{
			// переименовываем старый, чтобы не затереть
			::MoveFile( dlg->userLibrary, dlg->userLibrary + L".old" );
		}

		struct CountAll : public ns_foreach::ForEachFunctorC
		{
			int totalFilms = 0;

			ns_foreach::Result Run( const FileInfo & /*info*/ ) override
			{
				++totalFilms;
				return ns_foreach::CONTINUE;
			}
		};

		CountAll countAll;

		for( unsigned i = 0; i < dlg->_fileTree.GetRootsCount(); ++i )
		{
			FileInfo & rootItem = dlg->_fileTree.GetRoot( i );
			ns_foreach::ForEach( rootItem, &countAll );
		}

		dlg->SetStepCounts( countAll.totalFilms );

		Convert convert;
		convert.dlg = dlg;
		convert.c2k = &dlg->_c2k;
		convert.kinopoiskBase = &dlg->_kinopoiskBase;

		for( unsigned i = 0; i < dlg->_fileTree.GetRootsCount(); ++i )
		{
			FileInfo & rootItem = dlg->_fileTree.GetRoot( i );
			ForEach( rootItem, &convert );

			ns_foreach::HashItem hi;
			hi.rootLength = rootItem.fullPath.GetLength();
			ns_foreach::ForEach( rootItem, &hi );
		}

		{
			std::ofstream outFile( dlg->userLibrary, std::ios_base::binary );
			FemStore::Writer w( outFile );

			const unsigned int DOCUMENT_VERSION = 3;

			w & DOCUMENT_VERSION;

			w & dlg->_fileTree;

			PosterManager & pm = GetPosterManager();
			w & pm;
		}

#endif

	}
	catch( ... )
	{
	}

	dlg->PostMessage( WM_CLOSE, 0, 0 );
}

BOOL CC2KDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon( m_hIcon, TRUE );			// Крупный значок
	SetIcon( m_hIcon, FALSE );		// Мелкий значок

	if( !PathFileExists( L"video.dat" ) )
	{
		AfxMessageBox( L"Не найден файл базы данных.", MB_OK | MB_ICONERROR );
		return FALSE;
	}
	if( !PathFileExists( L"c2k.bin" ) )
	{
		AfxMessageBox( L"Не найден файл базы данных.", MB_OK | MB_ICONERROR );
		return FALSE;
	}

	_beginthread( ProcessInThread, 0, this );

	return TRUE;
}

void CC2KDlg::OnPaint()
{
	if( IsIconic() )
	{
		CPaintDC dc( this ); // контекст устройства для рисования

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0 );

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect( &rect );
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon( x, y, m_hIcon );
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CC2KDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CC2KDlg::LoadC2K()
{
	std::ifstream in( "c2k.bin", std::ios_base::binary );
	FemStore::Reader r( in );
	r & _c2k;
}

#if ENABLE_C2K_BUILD
void CC2KDlg::LoadCinemate()
{
	try
	{

		const CString dbPath = L"video.dat";
		std::ifstream rf( dbPath, std::ios_base::in | std::ios_base::binary );
		if( !rf.is_open() )
			return;

		std::string buf;

		unsigned dbVersion = 0;	// Версия базы данных фильмов
		unsigned numFilms = 0;	// Количество фильмов в базе данных

		std::getline( rf, buf );
		dbVersion = atoi( buf.c_str() );

		std::getline( rf, buf );
		numFilms = atoi( buf.c_str() );

		boost::iostreams::filtering_streambuf< boost::iostreams::input > stream;

		stream.push( boost::iostreams::zlib_decompressor() );
		stream.push( rf );

		std::strstream in;

		boost::iostreams::copy( stream, in );

		_cinemateBase.resize( numFilms );

		wchar_t unicodeBuffer[64];

		unsigned filmIndex = 0;

		while( !in.eof() && (filmIndex < numFilms) )
		{
			CinemateInfo & cd = _cinemateBase[filmIndex];

			std::getline( in, buf );
			cd.id = atoi( buf.c_str() );

			std::getline( in, buf );
			cd.year = atoi( buf.c_str() );

			// rus
			std::getline( in, buf );
			MultiByteToWideChar( CP_UTF8, 0, buf.c_str(), buf.size() + 1, unicodeBuffer, 64 );
			unicodeBuffer[63] = 0;
			cd.titleRussian = unicodeBuffer;

			// eng
			std::getline( in, buf );
			MultiByteToWideChar( CP_UTF8, 0, buf.c_str(), buf.size() + 1, unicodeBuffer, 64 );
			unicodeBuffer[63] = 0;
			cd.titleOriginal = unicodeBuffer;

			if( cd.titleOriginal.IsEmpty() )
				cd.titleOriginal = cd.titleRussian;

			// genre
			std::getline( in, buf );
			cd.genres = _atoi64( buf.c_str() );

			// poster
			std::getline( in, buf );
			cd.posterUrl = buf.c_str();

			++filmIndex;
		}

	}
	catch( ... )
	{
	}
}
#endif

void CC2KDlg::LoadKinopoisk()
{
	try
	{
		std::ifstream in( "video.dat", std::ios_base::binary );
		FemStore::Reader read( in, false );
		read & _kinopoiskBase;
	}
	catch( ... )
	{
	}

#if ENABLE_C2K_BUILD
	std::sort( _kinopoiskBase.begin(), _kinopoiskBase.end(), KinopoiskEnRu() );
#else
	std::sort( _kinopoiskBase.begin(), _kinopoiskBase.end(), KinopoiskID() );
#endif
}



void CC2KDlg::OnOK()
{
}
