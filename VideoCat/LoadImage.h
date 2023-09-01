#pragma once

inline void ULLoadImage(LPCTSTR szResource, LPCTSTR szResourceType, CImage & image)
{
// szResourceType "PNG" "JPG"
	HRSRC hrsrc = ::FindResource(hMod, szResource, szResourceType);
	if( hrsrc )
	{
		HGLOBAL hgrsrc = ::LoadResource(hMod, hrsrc);
		if (hgrsrc)
		{
			LPBYTE pData = (LPBYTE)::LockResource(hgrsrc);
			DWORD dwTotalSize = ::SizeofResource(hMod, hrsrc);
			HGLOBAL hg = ::GlobalAlloc(GHND, dwTotalSize);
			LPVOID pvData = ::GlobalLock(hg);
			CopyMemory(pvData, pData, dwTotalSize);
			::GlobalUnlock(hg);

			LPSTREAM pStream = NULL;
			if( CreateStreamOnHGlobal(hg, TRUE, &pStream) == S_OK && pStream )
			{
				CImage image;
				image.Load(pStream);
				pStream->Release();
			}
		}
	}
}