// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "ThumbGenerator.h"
#include <fstream>
#include "CollectionDB.h"
#include "GlobalSettings.h"
#include "PosterDownloader.h"
#include "ResString.h"
#include "Traverse/TraverseCollection.h"
#include "ThumbnailSettingsDlg.h"
#include "WaitDlg.h"
#include "resource.h"


ThumbsMode::Enum GetThumbsGenerationMode( const CString & title, int & units, BOOL & autoselect )
{
	ThumbnailSettingsDlg dlg;

	dlg.createMode = autoselect ? 1 : 0;

	if( IDOK != dlg.DoModal() )
		return ThumbsMode::Disallow;

	autoselect = dlg.selectMode ? 0 : 1;
	units = dlg.sizeMode + 1; 

	return dlg.createMode ? ThumbsMode::AllFiles : ThumbsMode::OnlyNew;
}

struct BuildThumbList : public Traverse::Base
{
	CString * outString;
	CString thumbsDir;
	CollectionDB * cdb = nullptr;
	ThumbsMode::Enum mode = ThumbsMode::Disallow;

	unsigned count = 0;

	virtual Traverse::Result Run( Entry & entry ) override
	{
		if( entry.IsFolder() )
			return Traverse::CONTINUE;

		if( mode == ThumbsMode::OnlyNew )
		{
			// проверяем существование кадров для фильма, существующие пропускаем
			const CString hashString = entry.thisEntry.ToBase64();
			const CString thumbPath = thumbsDir + hashString + L".jpg";
			if( PathFileExists( thumbPath ) )
				return Traverse::CONTINUE;
		}

		std::wstring rootDir;
		Entry * root = cdb->FindEntry( entry.GetRootHandle() );
		if( root )
			rootDir = root->filePath;

		(*outString) += entry.GetMoviePath( rootDir, *cdb ).c_str();
		(*outString) += L"\n";
		(*outString) += entry.thisEntry.ToBase64();
		(*outString) += L"\n";

		++count;

		return Traverse::CONTINUE;
	}

};

void GenerateThumbnails( CollectionDB * cdb, const EntryHandle & root, ThumbsMode::Enum mode, int multiplier, BOOL autoselect )
{
	VC_TRY;

	if( mode == ThumbsMode::Disallow )
		return;

	const DWORD BUFFER_SIZE = 2048;

	wchar_t tempBuffer[BUFFER_SIZE];
	if( !::GetTempPath( BUFFER_SIZE, tempBuffer ) )
		return;

	CString tempFile = tempBuffer;
	tempFile += L"vc_thumb";
	CreateDirectory( tempFile, nullptr );

	tempFile += L"\\thumbList.txt";

	CString outString;
	
	outString = cdb->thumbsDirectory;
	if( outString.GetAt( 0 ) == L'.' )
	{
		outString.Replace( L".\\", GetGlobal().GetProgramDirectory() );
	}

	outString += L"\n";

	// автовыбор кадров
	if( autoselect )
		outString += L"1\n";
	else
		outString += L"0\n";

	// множитель разрешения кадров
	_itow_s( multiplier, tempBuffer, 10 );
	outString += tempBuffer;
	outString += L"\n";
	
	BuildThumbList buildList;
	buildList.cdb = cdb;
	buildList.mode = mode;
	buildList.outString = &outString;
	buildList.thumbsDir = cdb->thumbsDirectory;

	cdb->TraverseAll( root, &buildList );

	if( buildList.count == 0)
		return;

	std::vector<char> utf8( outString.GetLength() * 4 );
	int utf8Size = WideCharToMultiByte( CP_UTF8, 0, outString.GetString(), outString.GetLength(), std::data(utf8), utf8.size(), nullptr, nullptr );
	if( utf8Size >= (int)utf8.size() )
		utf8Size = utf8.size() - 1;
	utf8[utf8Size] = 0;

	if( utf8.empty() )
		return;

	std::ofstream out( tempFile.GetString(), std::ios_base::out );
	out << std::data(utf8);

	const CString thumbExe = GetGlobal().GetThumbsGenerator();

	if( !PathFileExists( thumbExe ) )
	{
		AfxMessageBox( ResFormat( IDS_LOST_FILE, thumbExe.GetString() ), MB_OK | MB_ICONERROR | MB_TOPMOST );
		return;
	}

	PROCESS_INFORMATION piProcInfo;
	ZeroMemory( &piProcInfo, sizeof( PROCESS_INFORMATION ) );
	STARTUPINFO siStartInfo;
	ZeroMemory( &siStartInfo, sizeof( STARTUPINFO ) );
	siStartInfo.cb = sizeof( STARTUPINFO );

	CString cmdLine;
	cmdLine.Format( L"\"%s\" %s", thumbExe.GetString(), tempFile.GetString() );

	wcscpy_s( tempBuffer, cmdLine.GetString() );

	CreateProcess(
		thumbExe,
		tempBuffer,
		nullptr,          // process security attributes 
		nullptr,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		NORMAL_PRIORITY_CLASS,             // creation flags 
		nullptr,          // use parent's environment 
		GetGlobal().GetProgramDirectory() + L"videocat\\" ,
		&siStartInfo,
		&piProcInfo
	);

	// ждать не нужно, процесс работает независимо от каталогизатора

	VC_CATCH( ... )
	{}
}

void GeneratePostersFromThumbs( CollectionDB * cdb, const EntryHandle & root )
{
	VC_TRY;

	struct NoPoster : public Traverse::Base
	{
		std::vector<EntryHandle> allFilms;

		virtual Traverse::Result Run( Entry & entry ) override
		{
			if( entry.IsFile() && entry.posterId == DefaultPosterID )
				allFilms.push_back( entry.thisEntry );

			return Traverse::CONTINUE;
		}
	};

	NoMadalWait waitDlg;
	CWaitCursor waitCursor;

	NoPoster noPoster;
	cdb->TraverseAll( root, &noPoster );

	PosterManager & pm = cdb->GetPosterManager();

	for( const EntryHandle & handle : noPoster.allFilms )
	{
		Entry * entry = cdb->FindEntry( handle );
		if( !entry )
			continue;

		const CString thumbName = cdb->thumbsDirectory + entry->thisEntry.ToBase64() + L".jpg";
		if( !PathFileExists( thumbName.GetString() ) )
			continue;

		std::unique_ptr<Gdiplus::Bitmap> src( Gdiplus::Bitmap::FromFile( thumbName ) );
		if( !src )
			continue;
		const int w = src->GetWidth();
		const int h = (src->GetHeight() - 18) / 10;

		std::unique_ptr<Gdiplus::Bitmap> firstFrame( src->Clone( 0,18, w, h, PixelFormat32bppPARGB ) );

		Gdiplus::Bitmap * poster = nullptr;
		if( !ResizePoster( firstFrame.get(), poster ) )
			continue;

		entry->posterId = pm.SaveImage( poster );
		delete poster;
	}

	VC_CATCH( ... )
	{}
}
