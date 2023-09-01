#pragma once

void LoadPng( CImage & img, UINT id );

void LoadPng( const CString & fullPath, CImage & img );

void PremulAlpha( CImage & img );

void LoadPng( const CString & fullPath, Gdiplus::Bitmap* & img );

void PremulAlpha( Gdiplus::Bitmap & img );

Gdiplus::Bitmap * LoadPng( UINT id );
