#ifndef __BMP_H__
#define __BMP_H__

#pragma pack(1)

typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	unsigned long biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()

#endif