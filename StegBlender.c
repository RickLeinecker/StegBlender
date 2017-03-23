// StegBlender.c : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>
#ifndef __clang__
#include <malloc.h>
#endif
#include <stdio.h>
#include "StegBlender.h"

int main(int argc, char *argv[])
{

	/* Check for minimal arguments. */
	if (argc < 2)
	{
		usage();
	}

	/* User wants to embed a message. */
	if (_stricmp(argv[1], "embed") == 0)
	{
		doEmbed(argc, argv);
	}
	/* User wants to extract a message. */
	else if (_stricmp(argv[1], "extract") == 0)
	{
		doExtract(argc, argv);
	}
	/* Incorrect value so we show usage. */
	else
	{
		usage();
	}

	return(0);
}

/**********************************************************************************
** This section is for support code.
** Functions: usage(), calcDelta(), getModOperationValue(),
**            countTotal(), adjustGroup(), and getFileLength().
***********************************************************************************/

/*
** This function shows the program usage.
*/
void usage(void)
{
	printf("Blender usage:\n");
	printf("Blender embed BMPLoadFileName BMPSaveFileName MessageLoadFileName");
	printf("Blender extract BMPLoadFileName [MessageSaveFileName] [display]");
	exit(1);
}

/*
** This function calculates the delta
** for a group. It will be -1 for a total
** that is greater than halfway or 1 otherwise.
*/
int calcDelta(unsigned char *data, int groupSize)
{
	int total, halfway;

	/* Get the group total. */
	total = countTotal(data, groupSize);
	/* Calculate halfway. */
	halfway = groupSize * MODNUMBER / 2;

	/* Return -1 or 1. */
	return(total > halfway ? -1 : 1);
}

/*
** This function calculates the total
** mod the predetermined modulus number.
*/
int getModOperationValue(unsigned char *data, int groupSize, int modNumber)
{
	int total;

	/* Get the group total. */
	total = countTotal(data, groupSize);

	/* Return the modded value. */
	return(total % modNumber);
}

/*
** This function sums a series of
** consecutive image data bytes.
*/
int countTotal(unsigned char *data, int groupSize)
{
	/* Start with zero. */
	int total = 0;

	/* Go through the image data in the group. */
	for (int i = 0; i < groupSize; i++)
	{
		/* Update the total. */
		total += (int)data[i];
	}

	/* Return the total. */
	return(total);
}

/*
** Check to make sure a new value is in range.
*/
int inRange(int data, int delta)
{
	return(data + delta >= 0 && data + delta <= 255);
}

/*
** This function adjusts a group of carrier bytes
** so that its mod value will be that of the messageValue
*/
void adjustGroup(unsigned char *data, int groupSize, int modNumber, int messageValue)
{
	/* We will go from 0 to groupSize - 1 */
	int circular = 0;

	/* Need the delta. It will be -1 or 1. */
	int delta = calcDelta(data, groupSize);

	/* We do this until we arrive at the message value. */
	while (getModOperationValue(data, groupSize, modNumber) != messageValue)
	{
		/* Add the delta to the currently indexed byte. */
		if (inRange((int)data[circular], delta))
		{
			data[circular] = data[circular] + delta;
		}

		/* Increment circular, and wrap it back to zero if necessary. */
		circular = circular + 1;
		if (circular >= groupSize)
		{
			circular = 0;
		}
	}
}

/*
** This function returns the file length for an open file.
*/
int getFileLength(FILE *fp)
{
	int fileLength, curr;
	/* Seek to the end. */
	curr = fseek(fp, 0, SEEK_END);
	/* Record where we are. */
	fileLength = ftell(fp);
	/* Back to where we started. */
	fseek(fp, curr, SEEK_SET);

	/* Return the value. */
	return(fileLength);
}

/*********************************************************************************
** This section is for loading and saving data files.
** Functions: loadBMPRGBData(), saveBMP(), loadMessageData(), saveMessageData().
**********************************************************************************/

/*
** This function loads a BMP image file
** and returns a pointer to the RGB data.
** Note that caller must free the memory.
*/
unsigned char *loadBMPRGBData(const char *filePath, int *rgbSize, BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih)
{
	int fileLength;
	FILE *fp;
	unsigned char *ret;

	/* Attemp to open the file. */
	fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		/* Return NULL if the file could not be opened. */
		return(NULL);
	}

	/* Get the file length. */
	fileLength = getFileLength(fp);

	/* Load the bitmap file header and information header. */
	fread(bfh, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(bih, sizeof(BITMAPINFOHEADER), 1, fp);

	/* Calculate the size of the RGB data. */
	*rgbSize = fileLength - (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

	/* Attempt to allocate the RGB buffer. */
	ret = (unsigned char *)malloc(*rgbSize);
	if (ret == NULL)
	{
		/* Close the file. */
		fclose(fp);
		/* Return NULL if the memory could not be allocated. */
		return(NULL);
	}

	/* Read the RGB data into the buffer. */
	fread(ret, sizeof(unsigned char), *rgbSize, fp);
	/* Close the file. */
	fclose(fp);

	/* Return the RGB buffer. */
	return(ret);
}

/*
** This function saves data to a BMP file.
*/
void saveBMP(BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih, unsigned char *rgbData, int rgbSize, const char *filePath)
{
	FILE *fp;

	/* Create the file.*/
	fp = fopen(filePath, "wb");
	/* Write the headers. */
	fwrite(bfh, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(bih, sizeof(BITMAPINFOHEADER), 1, fp);
	/* Write the data. */
	fwrite(rgbData, sizeof(unsigned char), rgbSize, fp);
	/* Close the file. */
	fclose(fp);
}

/*
** This function loads the message data into an allocated buffer.
** It also sets the first characters to indicate the size of the message data.
*/
char *loadMessageData(char *filePath, int *messageSize, int numSizeDigits)
{
	FILE *fp;
	int fileLength;
	char *ret;
	char temp[20];

	/* Attempt to open the message data file. */
	fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		/* Bail out since the file did not open. */
		return(NULL);
	}

	/* Get the file size. */
	fileLength = getFileLength(fp);

	/* Attempt to allocate the message data buffer.
	We need to also allocate additional space for the message size. */
	ret = (char *)malloc(fileLength + numSizeDigits);
	if (ret == NULL)
	{
		/* Buffer did not allocate so we close
		the file and return NULL. */
		fclose(fp);
		return(NULL);
	}

	/* Read the message data into the buffer. We skip the first
	several bytes of the return buffer where the size data
	will reside. */
	fread(&ret[numSizeDigits], sizeof(char), fileLength, fp);
	fclose(fp);

	/* Here we paste the size (with length of 5, and zero padded) into
	the first part of the message. The size becomes part of the message. */
	sprintf(temp, formats[numSizeDigits], fileLength);
	strncpy(ret, temp, numSizeDigits);

	/* Set the messageSize variable to indicate the full buffer size. */
	*messageSize = fileLength + numSizeDigits;

	/* Return the message buffer. */
	return(ret);
}

/*
** This function saves the message data.
*/
void saveMessageData(unsigned char *message, int messageSize, char *filePath)
{
	FILE *fp;

	/* Attempt to create the output file. */
	fp = fopen(filePath, "wb");

	/* Check for a NULL indicating failure. */
	if (fp == NULL)
	{
		/* Alert the user and bail out. */
		printf("Could not create %s\n", filePath);
		return;
	}

	/* Write the message data. */
	fwrite(message, sizeof(unsigned char), messageSize, fp);

	/* Close the file handle. */
	fclose(fp);
}

/*********************************************************************************
** This section is for embedding and extracting images.
** Functions: doEmbed(), doExtract(), extractMessage() and embedMessage().
**********************************************************************************/

/*
** This is a helper function that manages the embed process.
*/

void doEmbed(int argc, char *argv[])
{
	char *messageToStore;
	unsigned char *rgbData;
	int rgbSize, messageSize, bytesThatCanBeHidden;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	/* Make sure we have enough arguments. The test in mail just makes sure
	that the initial stricmp() does not fail. */
	if (argc < 5)
	{
		usage();
	}

	/* Call the function to load the message data. */
	messageToStore = loadMessageData(argv[4], &messageSize, NUMSIZEDIGITS);
	if (messageToStore == NULL)
	{
		/* Bail out if we could not load the data. */
		printf("Could not load the message data file.");
		exit(2);
	}

	/* Call the function to load the RGB data from the BMP file. */
	rgbData = loadBMPRGBData(argv[2], &rgbSize, &bfh, &bih);
	if (rgbData == NULL)
	{
		/* Bail out if we could not load the data. */
		printf("Could not load the BMP data.\n");
		exit(3);
	}

	/* Calculate the number of message bytes that can be hidden in the carrier. */
	bytesThatCanBeHidden = rgbSize / GROUPSIZE;
	/* See if we have enough space. */
	if (messageSize > bytesThatCanBeHidden)
	{
		/* Alert user to the problem. */
		printf("There is not enough room in this image to hide the message data that you have specified.");

		/* Free up the allocated buffers. */
		free(messageToStore);
		free(rgbData);

		/* Bail out. */
		exit(4);
	}

	/* Call the worker function that embeds the message into the RGB data. */
	embedMessage(rgbData, (unsigned char *)messageToStore, messageSize, GROUPSIZE, MODNUMBER);

	/* Call the function to save the altered image data. */
	saveBMP(&bfh, &bih, rgbData, rgbSize, argv[3]);

	/* Delete the memory buffers. */
	free(messageToStore);
	free(rgbData);
}

/*
** This is a helper function that manages the extract process.
*/

void doExtract(int argc, char *argv[])
{
	unsigned char *messageData, *rgbData;
	int rgbSize, messageSize;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	/* Call the function to load the RGB data from the BMP file. */
	rgbData = loadBMPRGBData(argv[2], &rgbSize, &bfh, &bih);
	if (rgbData == NULL)
	{
		/* Bail out if we could not load the data. */
		printf("Could not load the BMP data.\n");
		exit(5);
	}

	/* Call the worker function that extracts the message. */
	messageData = extractMessage(rgbData, NUMSIZEDIGITS, GROUPSIZE, MODNUMBER, &messageSize);

	/* Here we may have more than the minimum command line arguments. */
	for (int i = 0; i < 2; i++)
	{
		if (argc > 3 + i)
		{
			/* See of this indicates display to the console of the message. */
			if (_stricmp(argv[3 + i], "display") == 0)
			{
				printf("%s\n", messageData);
			}
			/* Otherwise save the data to disk. */
			else
			{
				saveMessageData(messageData, messageSize, argv[3 + i]);
			}
		}
	}

	/* Delete the memory buffers. */
	free(messageData);
	free(rgbData);
}

/*
** This function retrieves a hidden message.
*/
unsigned char *extractMessage(unsigned char *rgbData, int numSizeDigits, int groupSize, int modNumber, int *messageSize)
{
	int total;
	unsigned char *ret;

	/* Start by setting messageSize to zero. */
	*messageSize = 0;

	/* Loop through the prefix digits. */
	for (int i = 0; i < numSizeDigits; i++)
	{
		/* Get the total for this group of image data bytes. */
		total = countTotal(rgbData, groupSize);

		/* Get the modulus operation value.
		Don't forget to subtract '0' */
		int modOperationValue = getModOperationValue(rgbData, groupSize, modNumber) - '0';

		/* Update the message size value. */
		*messageSize = (*messageSize) * 10 + modOperationValue;

		/* Next set of image data bytes. */
		rgbData += groupSize;
	}

	ret = (unsigned char *)malloc((*messageSize) + 1);

	for (int i = 0; i < (*messageSize); i++)
	{
		/* Get the total for this group of image data bytes. */
		total = countTotal(rgbData, groupSize);

		/* Get the modulus operation value. */
		int modOperationValue = getModOperationValue(rgbData, groupSize, modNumber);

		/* Store the value. */
		ret[i] = (unsigned char)modOperationValue;

		/* Next set of image data bytes. */
		rgbData += groupSize;
	}

	/* NULL terminate in case this is a string. */
	ret[*messageSize] = 0;

	/* Return the data. */
	return(ret);
}

/*
** This function embeds a message
** into an RGB image buffer.
*/
void embedMessage(unsigned char *rgbData, unsigned char *message, int messageLength, int groupSize, int modNumber)
{

	/* Loop through the bytes in the message. */
	for (int i = 0; i < messageLength; i++)
	{
		/* Call the function that adjusts the carrier bytes in this group. */
		adjustGroup(rgbData, groupSize, modNumber, (int)message[i]);

		/* Move the pointer to the next group of image bytes. */
		rgbData += groupSize;
	}

}
