#pragma once
#include <Windows.h>
#include <gdiplus.h>

namespace BitmapUtil
{
	//The GetBitmapHeaderSize function returns the size of the DIB header
	ULONG GetBitmapHeaderSize(LPCVOID pDib);

	//The GetBitmapLineWidthInBytes function returns the number of bytes
	//in one scan line of the image
	ULONG GetBitmapLineWidthInBytes(ULONG nWidthInPixels, ULONG nBitCount);

	//The GetBitmapDimensions function returns the width and height of a DIB
	BOOL GetBitmapDimensions(LPCVOID pDib, UINT *pWidth, UINT *pHeight);

	// The GetBitmapSize function returns total size of the DIB. The size is
	// the sum of the bitmap header, the color palette (if present), the color
	// profile data (if present) and the pixel data.
	ULONG GetBitmapSize(LPCVOID pDib);

	//The GetBitmapOffsetBits function returns the offset, in bytes, from the
	//beginning of the DIB data block to the bitmap bits.
	ULONG GetBitmapOffsetBits(LPCVOID pDib);

	// The FixBitmapHeight function calculates the height of the DIB if the
	// height is not specified in the header and fills in the biSizeImage and
	// biHeight fields of the header.
	BOOL FixBitmapHeight(PVOID pDib, ULONG nSize, BOOL bTopDown);

	// The FillBitmapFileHeader function fills in a BITMAPFILEHEADER structure
	// according to the values specified in the DIB.
	BOOL FillBitmapFileHeader(LPCVOID pDib, PBITMAPFILEHEADER pbmfh);
}; 
