#ifndef __STEGBLENDER_H__
#define __STEGBLENDER_H__

#include "BMP.h"

int MODNUMBER = 256;
int GROUPSIZE = 12;
int NUMSIZEDIGITS = 5;
static char *formats[] = { "", "%d", "%02d", "%03d", "%04d", "%05d", "%06d", "%07d", "%08d", "%09d" };

unsigned char *loadBMPRGBData(const char *filePath, int *rgbSize, BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih);
void saveBMP(BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih, unsigned char *rgbData, int rgbSize, const char *filePath);
int countTotal(unsigned char *data, int groupSize);
int calcDelta(unsigned char *data, int groupSize);
int getModOperationValue(unsigned char *data, int groupSize, int modNumber);
void usage(void);
void doEmbed(int argc, char *argv[]);
void embedMessage(unsigned char *rgbData, unsigned char *message, int messageLength, int groupSize, int modNumber );
void doExtract(int argc, char *argv[]);
unsigned char *extractMessage(unsigned char *rgbData, int numSizeDigits, int groupSize, int modNumber, int *messageSize);
int inRange(int data, int delta);

#ifdef __GNUC__
#define _stricmp strcasecmp
#endif

#endif
