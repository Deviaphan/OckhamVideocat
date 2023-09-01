// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "pch.h"
#include "IsValidFileName.h"

const wchar_t * pInvalidFileNameErrStr[] =
{
	_T("Error"),
	_T("CLOCK$"),
	_T("AUX"),
	_T("CON"),
	_T("NUL"),
	_T("PRN"),
	_T("COM1"),
	_T("COM2"),
	_T("COM3"),
	_T("COM4"),
	_T("COM5"),
	_T("COM6"),
	_T("COM7"),
	_T("COM8"),
	_T("COM9"),
	_T("LPT1"),
	_T("LPT2"),
	_T("LPT3"),
	_T("LPT4"),
	_T("LPT5"),
	_T("LPT6"),
	_T("LPT7"),
	_T("LPT8"),
	_T("LPT9"),
	nullptr
};

int IsValidFileName( CString & fileName)
{
	int nonDot = -1;	// position of the first non dot in the file name
	int dot = -1;		// position of the first dot in the file name
	int len = 0;		// length of the file name
	for (; len < 256 && fileName[len]; len++)
	{
		if (fileName[len] == '.')
		{
			if (dot < 0)
				dot = len;
			continue;
		}
		else if (nonDot < 0)
			nonDot = len;

		// The upper characters can be passed with a single check and
		// since only the backslash and bar are above the ampersand
		// it saves memory to do the check this way with little performance
		// cost.
		if (fileName[len] >= '@')
		{
			if (fileName[len] == '\\' || fileName[len] == '|')
				return -1;

			continue;
		}

		static bool isCharValid[32] =
		{
		//  ' '   !     "      #     $     %     &     '     (     )     *      +     ,      -     .      / 
			true, true, false, true, true, true, true, true, true, true, false, true, true,  true, true,  false,
		//  0     1     2      3     4     5     6     7     8     9     :      ;     <      =     >      ?
			true, true, true,  true, true, true, true, true, true, true, false, true, false, true, false, false
		//  0     1     2      3     4     5     6     7     8     9     :      ;     <      =     >      ?
		};

		// This is faster, at the expense of memory, than checking each
		// invalid character individually. However, either method is much
		// faster than using strchr().
		if (fileName[len] >= 32)
		{
			if( fileName[len] > 64 ||  isCharValid[fileName[len] - 32])
				continue;
		}
		return -1;
	}

	if (len == 256)
		return -1;

	// if nonDot is still -1, no non-dots were encountered, return a dot (period)
	if (nonDot < 0)
		return '.';

	// if the first character is a dot, the filename is okay
	if (dot == 0)
		return 0;

	// if the file name has a dot, we only need to check up to the first dot
	if (dot > 0)
		len = dot;

	// Since the device names aren't numerous, this method of checking is the
	// fastest. Note that each character is checked with both cases.
	if (len == 3)
	{
		if (fileName[0] == 'a' || fileName[0] == 'A')
		{
			if ((fileName[1] == 'u' || fileName[1] == 'U') &&
				(fileName[2] == 'x' || fileName[2] == 'X'))
				return -1;
		}
		else if (fileName[0] == 'c' || fileName[0] == 'C')
		{
			if ((fileName[1] == 'o' || fileName[1] == 'O') &&
				(fileName[2] == 'n' || fileName[2] == 'N'))
				return -1;
		}
		else if (fileName[0] == 'n' || fileName[0] == 'N')
		{
			if ((fileName[1] == 'u' || fileName[1] == 'U') &&
				(fileName[2] == 'l' || fileName[2] == 'L'))
				return -1;
		}
		else if (fileName[0] == 'p' || fileName[0] == 'P')
		{
			if ((fileName[1] == 'r' || fileName[1] == 'R') &&
				(fileName[2] == 'n' || fileName[2] == 'N'))
				return -1;
		}
	}
	else if (len == 4)
	{
		if (fileName[0] == 'c' || fileName[0] == 'C')
		{
			if ((fileName[1] == 'o' || fileName[1] == 'O') &&
				(fileName[2] == 'm' || fileName[2] == 'M') &&
				(fileName[3] >= '1' && fileName[3] <= '9'))
				return -1;
		}
		else if (fileName[0] == 'l' || fileName[0] == 'L')
		{
			if ((fileName[1] == 'p' || fileName[1] == 'P') &&
				(fileName[2] == 't' || fileName[2] == 'T') &&
				(fileName[3] >= '1' && fileName[3] <= '9'))
				return -1;
		}
	}
	else if (len == 6)
	{
		if ((fileName[0] == 'c' || fileName[0] == 'C') &&
			(fileName[1] == 'l' || fileName[1] == 'L') &&
			(fileName[2] == 'o' || fileName[2] == 'O') &&
			(fileName[3] == 'c' || fileName[3] == 'C') &&
			(fileName[4] == 'k' || fileName[4] == 'K') &&
			(fileName[5] == '$') )
			return -1;
	}

	return 1;
}

