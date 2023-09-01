#pragma once

extern const wchar_t tokenTransliteDelim[];

bool PossibleTranslite( const CString & title );

CString FromTranslite( const CString & title, bool ignoreUnexpected = false );
