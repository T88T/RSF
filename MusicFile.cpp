/*
 * MusicFile.cpp
 *
 *  Created on: 5 mars 2021
 *      Author: TooT
 */

#include "MusicFile.h"
#include "vs1053_SdFat.h"

extern vs1053 MP3player;
extern SdFat SD;

int freeMemory();

char Buffer[60];

MusicFile::MusicFile(String path) : mPath(path)
{
	mTitle = "";
	mDuration = 0;
	mFileSize = 0;

	mIsValid = readInfo();
}


MusicFile::MusicFile(File32 *file)
{
	mTitle = "";
	mDuration = 0;
	mFileSize = 0;


	mPath = "";

	mIsValid = readInfo(file);
}

bool MusicFile::isValid()
{
	return mIsValid;
}

String MusicFile::getTitle()
{
	return mTitle;
}

String MusicFile::getDurationS()
{
	return mDurationString;
}

uint32_t MusicFile::getDuration()
{
	return mDuration;
}


bool MusicFile::readInfo(File32 *file)
{
	File32 *track = nullptr;

	if(file == nullptr)
		return false;
	else
		track = file;

	track->getName(Buffer, 60);
	mPath = Buffer;

	if(!SD.exists(Buffer))
	{
		Serial.println("File does not exist !");
		return false;
	}

	mFileSize = track->fileSize();

	if(readTAG(track, TAG_TITLE, Buffer))
		mTitle = Buffer;

	uint16_t mBitrate = 320;
	mDuration = ((mFileSize-128)<<3)/mBitrate;
	mDurationString = String(mDuration/60/1000) + String(":") + String((float(mDuration)/60.0/1000.0 - (mDuration/60/1000))*60.0, 0);

	Serial.print("\tSuccessfully opened "); Serial.println(mPath);
	Serial.print("\t Title : "); Serial.println(mTitle);

	/*
	if(readTAG(track, TAG_ARTIST, Buffer))
	{
		Serial.print("\t Artist : "); Serial.println(Buffer);
	}

	if(readTAG(track, TAG_ALBUM, Buffer))
	{
		Serial.print("\t Album : "); Serial.println(Buffer);
	}
	*/

	Serial.print("\t Bitrate : "); Serial.println(mBitrate);
	Serial.print("\t Duration (ms) : "); Serial.println(mDuration);
	Serial.print("\t Duration : "); Serial.println(mDurationString);


	Serial.println(freeMemory());

	track->close();

	return true;
}

String MusicFile::getPath()
{
	return mPath;
}


uint16_t MusicFile::getBitrate()
{
	return 320;
}

bool MusicFile::readTAG(File32 *track, uint8_t offset, char* infobuffer)
{
	track->seekEnd(-128);
	char tag[4];
	track->read(tag, 4);

	if(tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'G')
	{
		track->seekEnd((-128 + offset));
		track->read(infobuffer, 30);

		//Delete non alpha numeric characters
		//infobuffer = strip_nonalpha_inplace(infobuffer);

		return true;
	}

	return false;
}
